#ifndef HtmlMetaElement_h_seen
#define HtmlMetaElement_h_seen
#include "HtmlElement.h"

class HtmlMetaElement : public HtmlElement
{
  public:
    HtmlMetaElement(const std::string & tagName);
    HtmlElement * clone() const;
  protected:
    std::string m_httpEquiv;
    std::string m_content;
    const std::string * attributePtr(const std::string & name) const;
    void copyAttributes(HtmlElement * copyTo) const;
};
#endif
