#ifndef HtmlElement_h_seen
#define HtmlElement_h_seen

#include <list>
#include "HtmlParser.h"
#include "UnicodeString.h"


class HtmlElement /* : public Element */
{

  public:
    HtmlElement(const std::string & tagName) 
      : m_tagName(tagName) 
    {}

    virtual ~HtmlElement();

    virtual void setAttribute(const std::string & name, const std::string & value);
    virtual std::string attribute(const std::string & name) const;
    void append(HtmlElement * child);
    inline HtmlElement * firstChild() const;
    inline const std::list<HtmlElement*> & children() const;

    inline bool hasChildren() const;
    inline bool isa(const std::string & name) const;
    inline const std::string & tagName() const;
    inline void append(unsigned int value);
    inline const UnicodeString & text() const;

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
    std::list<HtmlElement*> m_children;

    const std::string * attributePtr(const std::string & name) const;
};

bool HtmlElement::hasChildren() const
{
  return m_children.size() > 0;
}
bool HtmlElement::isa(const std::string & name) const
{
  return m_tagName == name;
}
HtmlElement * HtmlElement::firstChild() const
{
  if (hasChildren())
  {
    return m_children.front();
  }
  return 0;
}
const std::list<HtmlElement*> & HtmlElement::children() const
{
  return m_children;
}

const std::string & HtmlElement::tagName() const
{
  return m_tagName;
}

void HtmlElement::append(unsigned int value)
{
  m_text += value;
}
const UnicodeString & HtmlElement::text() const
{
  return m_text;
}
#endif
