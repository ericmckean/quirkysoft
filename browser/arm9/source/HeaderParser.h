#ifndef HeaderParser_h_seen
#define HeaderParser_h_seen
#include <string>
class HtmlParser;
class HeaderParser
{
  // parse the headers / chunks
  public:
    HeaderParser(HtmlParser * html);
    void feed(const char * data, unsigned int length);

    void handleHeader(const std::string & field, const std::string & value);
    void handleEndHeaders();
    void handleData(const char * data, unsigned int length);
    void parseError();

    const std::string redirect() const;

    void reset();

  private:
    enum HeaderState
    {
      HTTP_RESPONSE,
      BEFORE_FIELD,
      FIELD,
      AFTER_FIELD,
      BEFORE_VALUE,
      VALUE,
      ENDING_HEADERS,
      CHUNK_LINE,
      DATA,
      PARSE_ERROR
    };

    // RFC 2616
    enum Response
    {
      //CONTINUE=100,
      //SWITCHING_PROTOCOLS=101,

      OK=200,
      //CREATED=201,
      //ACCEPTED=202,

      // redirection
      MULTIPLE_CHOICES=300,
      MOVED_PERMANENTLY=301,
      FOUND=302,
      SEE_OTHER=303,
      NOT_MODIFIED=304,
      USE_OTHER=305,
      TEMPORARY_REDIRECT=307,
    };

    HeaderState m_state;
    unsigned int m_value;
    const char * m_position;
    const char * m_end;
    const char * m_lastPosition;

    std::string m_field;
    std::string m_headerValue;
    std::string m_redirect;
    bool m_chunked;
    int m_chunkLength;
    std::string m_chunkLengthString;
    unsigned int m_httpStatusCode;

    HtmlParser * m_htmlParser;

    void rewind();
    void next();
    void httpResponse();
    void beforeField();
    void field();
    void afterField();
    void beforeValue();
    void value();
    void endingHeaders();
    void chunkLine();
    void fireData();

};

#endif
