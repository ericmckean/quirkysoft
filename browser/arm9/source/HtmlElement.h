#ifndef HtmlElement_h_seen
#define HtmlElement_h_seen

#include "ElementList.h"
#include "HtmlParser.h"
#include "UnicodeString.h"


class HtmlElement /* : public Element */
{

  public:
    HtmlElement(const std::string & tagName) 
      : m_tagName(tagName),
        m_parent(0) 
    {}

    virtual ~HtmlElement();

    virtual void setAttribute(const std::string & name, const std::string & value);
    virtual std::string attribute(const std::string & name) const;
    void append(HtmlElement * child);
    void remove(HtmlElement * child);
    void appendText(unsigned int value);
    inline HtmlElement * firstChild() const;
    inline HtmlElement * lastChild() const;
    inline const ElementList & children() const;

    inline HtmlElement* parent() const;
    inline void setParent(HtmlElement * newParent);
    inline bool hasChildren() const;
    inline bool isa(const std::string & name) const;
    inline bool isa_ptr(const std::string * name) const;
    inline const std::string & tagName() const;
    inline const UnicodeString & text() const;

    virtual HtmlElement * clone() const;

  protected:
    std::string m_tagName;
    std::string m_id;
    std::string m_title;
    std::string m_lang;
    std::string m_dir;
    UnicodeString m_text;
    // not implemented
    //std::string m_className;
    // std::vector<std::string> m_classList;
    HtmlElement * m_parent;
    ElementList m_children;

    virtual const std::string * attributePtr(const std::string & name) const;
    virtual void copyAttributes(HtmlElement * copyTo) const;
};

bool HtmlElement::hasChildren() const
{
  return m_children.size() > 0;
}
bool HtmlElement::isa(const std::string & name) const
{
  return m_tagName == name;
}
bool HtmlElement::isa_ptr(const std::string * name) const
{
  return name and isa(*name);
}
HtmlElement * HtmlElement::firstChild() const
{
  if (hasChildren())
  {
    return m_children.front();
  }
  return 0;
}
HtmlElement * HtmlElement::lastChild() const
{
  if (hasChildren())
  {
    return m_children.back();
  }
  return 0;
}
const ElementList & HtmlElement::children() const
{
  return m_children;
}

const std::string & HtmlElement::tagName() const
{
  return m_tagName;
}
const UnicodeString & HtmlElement::text() const
{
  return m_text;
}

HtmlElement* HtmlElement::parent() const
{
  return m_parent;
}
void HtmlElement::setParent(HtmlElement * newParent)
{
  m_parent = newParent;
}
#endif
