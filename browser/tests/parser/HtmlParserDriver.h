#ifndef HtmlParserDriver_h_seen
#define HtmlParserDriver_h_seen

#include "UnicodeString.h"
#include "HtmlParser.h"
#include <vector>

class HeaderParser;
class HtmlParserDriver : public HtmlParser
{

  public :

    UnicodeString m_data;
    std::vector<std::string> m_tags;
    std::vector< std::vector<Attribute*> > m_attributes;

    HeaderParser * m_headerParser;

  protected:
    virtual void handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs);
    virtual void handleEndTag(const std::string & tag);
    virtual void handleData(unsigned int ucodeChar);

};

#endif
