#include "Document.h"
#include "HeaderParser.h"

using namespace std;

Document::Document():HtmlParser(),
  m_amount(0),
  m_headerParser(new HeaderParser(this))
{
  m_data.clear();
}

void Document::setUri(const std::string & uriString)
{
  m_uri = uriString;
}

const std::string & Document::uri() const
{
  return m_uri;
}

// const char * Document::asText() const
const std::basic_string<unsigned int> & Document::asText() const
{
  return m_data; // .c_str();
}

void Document::reset() 
{
  m_data.clear();
  m_headerParser->reset();
}

void Document::registerView(ViewI * view)
{
   vector<ViewI*>::iterator it = find(m_views.begin(), m_views.end(), view);
   if (it == m_views.end())
     m_views.push_back(view);
}

void Document::unregisterView(ViewI * view)
{
   vector<ViewI*>::iterator it = find(m_views.begin(), m_views.end(), view);
   if (it != m_views.end())
     m_views.erase(it);
}

void Document::appendData(const char * data, int size)
{
  // cout << "Append data: "  << size << endl;
  m_status = INPROGRESS;
  if (size) {
    m_headerParser->feed(data,size);
    if (not m_headerParser->redirect().empty()) 
    {
      m_uri = m_headerParser->redirect();
    }
  } 
  notifyAll();
}

void Document::setLoading(int amount)
{
  m_status = INPROGRESS;
  if (amount != m_amount) {
    notifyAll();
  }
  m_amount = amount;
}

void Document::notifyAll() const
{
  for_each(m_views.begin(), m_views.end(), mem_fun(&ViewI::notify));
}

void Document::setStatus(Document::Status status)
{
  m_status = status;
  notifyAll();
}
Document::Status Document::status() const
{
  return m_status;
}

void Document::handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  /*
  cout << "+- tag token:" << tag << endl;
  vector<Attribute*>::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it)
  {
    cout << "Attribute:" << (*it)->name << " = " << (*it)->value << endl;
  }
  */
}

void Document::handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  /*
  if ( tag == "br") {
    cout << endl;
  }
  if ( tag == "p") {
    cout << endl;
  }
  */
  if ( tag == "meta") {
    vector<Attribute*>::const_iterator it(attrs.begin());
    for (; it != attrs.end(); ++it)
    {
      if ( (*it)->name == "content" and (*it)->value == "text/html; charset=iso-8859-1") {
        // parse charset...
        this->setEncoding(ISO_ENCODING);
        break;
      }
    }
  }
}
void Document::handleEndTag(const std::string & tag)
{
}

//void Document::handleData(const std::string & data)
void Document::handleData(unsigned int ucodeChar)
{
  // m_data.append(data.c_str(), data.length());
  m_data += ucodeChar;
}
