#ifndef HtmlAnchorElement_h_seen
#define HtmlAnchorElement_h_seen
#include "HtmlElement.h"

class HtmlAnchorElement : public HtmlElement
{
  public:
    HtmlAnchorElement(const std::string & tagName) 
      : HtmlElement(tagName) {}

    HtmlElement * clone() const;
  protected:
    std::string m_href;
    const std::string * attributePtr(const std::string & name) const;
    void copyAttributes(HtmlElement * copyTo) const;
};
#endif
