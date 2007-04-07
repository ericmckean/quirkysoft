#include <assert.h>
#include "HeaderParser.h"
#include "HtmlParser.h"

using namespace std;
HeaderParser::HeaderParser(HtmlParser * htmlParser):
  m_htmlParser(htmlParser)
{
  reset();
}

void HeaderParser::reset()
{
  m_state = HTTP_RESPONSE;
  m_redirect = "";
  m_chunked = false;
  m_chunkLength = 0;
  m_chunkLengthString = "";
  m_htmlParser->setToStart();
}

void HeaderParser::rewind()
{
  m_position = m_lastPosition;
}

void HeaderParser::next()
{
  m_value = *m_position;
  m_lastPosition = m_position;
  m_position++;
}

void HeaderParser::feed(const char * data, unsigned int length)
{
  m_position = data;
  m_end = data+length;
  while (m_position < m_end)
  {
    next();
    switch (m_state)
    {
      case HTTP_RESPONSE:
        httpResponse();
        break;
      case BEFORE_FIELD:
        beforeField();
        break;
      case FIELD:
        field();
        break;
      case AFTER_FIELD:
        afterField();
        break;
      case BEFORE_VALUE:
        beforeValue();
        break;
      case VALUE:
        value();
        break;
      case ENDING_HEADERS:
        endingHeaders();
        break;
      case CHUNK_LINE:
        chunkLine();
        break;
      case PARSE_ERROR:
        parseError();
        break;
      case DATA:
        fireData();
        break;
    }
  }
}

const std::string HeaderParser::redirect() const
{
  return m_redirect;
}


unsigned int HeaderParser::expected() const
{
  return m_expected;
}

static void stripWhitespace(string & modify)
{
  if (modify.empty())
    return;
  static const string delimter(" \r\n	");
  int firstNonBlank = modify.find_first_not_of(delimter);
  int lastNonBlank = modify.find_last_not_of(delimter);
  modify = modify.substr(firstNonBlank, (lastNonBlank-firstNonBlank+1));
}

static void extractCharset(const string & value, string & mimeType, string & charset)
{
  unsigned int position(value.find(";"));
  if (position != string::npos)
  {
    mimeType = value.substr(0,position);
    position++;
    unsigned int nextPosition(value.find(";",position));
    nextPosition = nextPosition==string::npos?value.length():nextPosition;
    charset = value.substr(position, (nextPosition-position+1));
  } 
  else {
    mimeType = value.substr(0,value.length());
    charset.clear();
  }
  stripWhitespace(mimeType);
  stripWhitespace(charset);
}

void HeaderParser::handleHeader(const string & field, const string & value)
{
  // cout << "Header: " << field << " = \"" << value << "\"" << endl;
  if (field == "transfer-encoding" and value == "chunked") {
    m_chunked = true;
  }
  if (field == "location" and m_httpStatusCode >= 300 and m_httpStatusCode < 400)
  {
    m_redirect = value;
  }
  if (field == "content-length") {
    m_expected = strtol(value.c_str(), 0 , 0);
  }
  if (field == "content-type") {
    string lowerValue(value);
    transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
    string charset, mimeType;
    extractCharset(lowerValue, charset, mimeType);
    if (mimeType == "charset=iso-8859-1")
    {
      // cout << "ISO encoding" << endl;
      m_htmlParser->setEncoding(HtmlParser::ISO_ENCODING);
    }
    bool isPlain = lowerValue.find("text/plain") != string::npos;
    if (isPlain)
    {
      m_htmlParser->setPlainText();
    }

  }
}

void HeaderParser::checkMetaTagHttpEquiv(const std::vector<HtmlParser::Attribute*> & attrs)
{
  vector<HtmlParser::Attribute*>::const_iterator it(attrs.begin());
  string httpEquiv; httpEquiv.clear();
  for (; it != attrs.end(); ++it)
  {
    if (not httpEquiv.empty() and (*it)->name == "content")
    {
      transform(httpEquiv.begin(), httpEquiv.end(), httpEquiv.begin(), ::tolower);
      handleHeader(httpEquiv, (*it)->value);
      break;
    }
    if ( (*it)->name == "http-equiv")
    {
      httpEquiv = (*it)->value;
    }
  }
}

void HeaderParser::parseError()
{
  if (m_chunked and m_chunkLength == 0) {
    return;
  }
  // woops "this should never happen" so catch it when it does.
  assert(m_state != PARSE_ERROR);
  m_position = m_end;
}

void HeaderParser::beforeField()
{
  if (::isalpha(m_value)) {
    m_value = ::tolower(m_value);
    m_field = m_value;
    m_state = FIELD;
  } else {
    if (m_value == '\r') {
      next();
    }
    if (m_value == '\n') {
      m_state = ENDING_HEADERS;
      rewind();
    }
  }
  // else parse error, but keep going...
}

void HeaderParser::endingHeaders()
{
  if (m_chunked) {
    m_state = CHUNK_LINE;
    m_chunkLengthString = "0x";
  }
  else
  {
    m_state = DATA;
  }
}

void HeaderParser::field()
{
  if (m_value == '-' or ::isalpha(m_value)) {
    m_value = ::tolower(m_value);
    m_field += m_value;
  } else {
    switch (m_value) {
      case ' ':
        m_state = AFTER_FIELD;
        break;
      case ':':
        m_state = BEFORE_VALUE;
        break;
      default:
        // parse error?
        m_state = PARSE_ERROR;
        break;
    }
  }
}
void HeaderParser::afterField()
{
  if (m_value == ':') {
    m_state = BEFORE_VALUE;
  } else {
    switch (m_value) {
      case ' ':
        // keep looking for :
        m_state = AFTER_FIELD;
        break;
      default:
        // parse error.
        m_state = PARSE_ERROR;
        break;
    }
  }
}
void HeaderParser::beforeValue()
{
  switch (m_value) {
    case ' ':
      break;
    case '\n':
      m_state = PARSE_ERROR;
      break;
    default:
      m_headerValue = m_value;
      m_state = VALUE;
      break;
  }
}
void HeaderParser::value()
{
  switch (m_value) {
    case '\r':
      m_state = BEFORE_FIELD;
      next();
      // allow \n or \r\n 
      if (m_value != '\n') {
        rewind();
      } 
      break;
    default:
      m_headerValue += m_value;
      break;
  }
  if (m_state != VALUE)
  {
    handleHeader(m_field,m_headerValue);
  }
}

void HeaderParser::chunkLine()
{
  if (::isxdigit(m_value))
  {
    m_chunkLengthString += m_value;
  }
  else
  {
    m_state = PARSE_ERROR;
    if (m_value == '\r') {
      next();
    }
    if (m_value == '\n') {
      m_chunkLength = strtol(m_chunkLengthString.c_str(),0,0);
      m_expected = m_chunkLength;
      m_state = DATA;
    }
  }
}

void HeaderParser::setDataState()
{
  m_state = DATA;
}

unsigned int HeaderParser::httpStatusCode() const
{
  return m_httpStatusCode;
}

void HeaderParser::httpResponse()
{
  string response;
  while (m_value != '\n') {
    response += ::toupper(m_value);
    next();
  }
  if (response.substr(0,9) == "HTTP/1.1 ") {
    m_httpStatusCode = strtol(response.substr(9,3).c_str(), 0, 0);
    m_state = BEFORE_FIELD;
  } else {
    m_state = PARSE_ERROR;
  }
}
  
void HeaderParser::fireData()
{
  rewind();
  int length = (m_end - m_position);
  if (m_chunked) {
    if (length >= m_chunkLength) {
      length = m_chunkLength;
      m_state = BEFORE_FIELD;
      m_chunkLengthString = "";
      m_chunkLength = 0;
    }
    else 
    {
      m_chunkLength -= length;
    }
  }
  // once done, feed the data to the html parser.
  m_htmlParser->feed(m_position, length);
  m_position += length;
}
