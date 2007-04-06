#ifndef HtmlParser_h_seen
#define HtmlParser_h_seen

#include <vector>
#include <string>
class HtmlParser
{
  public:
    enum Encoding {
      UTF8_ENCODING,
      ISO_ENCODING
    };
    struct Attribute
    {
      std::string name;
      std::string value;
    };
    HtmlParser();
    virtual ~HtmlParser();
    void feed(const char * data, unsigned int length);

    void setToStart();

    void setEncoding(Encoding enc);
    Encoding encoding() const;

    virtual void handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleEndTag(const std::string & tag);
    // virtual void handleData(const std::string & data);
    virtual void handleData(unsigned int ucodeChar);

  private:
    class HtmlParserImpl & m_details;
};
#endif
