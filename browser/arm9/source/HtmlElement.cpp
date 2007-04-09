#include <vector>
#include "HtmlElement.h"
#include "ElementFactory.h"

using namespace std;

const string * HtmlElement::attributePtr(const string & name) const
{
  if (name == "id")
  {
    return &m_id;
  }
  if (name == "title")
  {
    return &m_title;
  }
  if (name == "lang")
  {
    return &m_lang;
  }
  if (name == "dir")
  {
    return &m_dir;
  }
  return 0;
}

string HtmlElement::attribute(const string & name) const
{
   const string * ptr = attributePtr(name);
   if (ptr)
   {
     return *ptr;
   }
   return "";
}

void HtmlElement::setAttribute(const string & name, const string & value)
{
   string * ptr = const_cast<string*>(attributePtr(name));
   if (ptr)
   {
     *ptr = value;
   }
}

void HtmlElement::append(HtmlElement * child)
{
  m_children.push_back(child);
}

HtmlElement::~HtmlElement()
{
  for_each(m_children.begin(), m_children.end(), ElementFactory::remove);
}

