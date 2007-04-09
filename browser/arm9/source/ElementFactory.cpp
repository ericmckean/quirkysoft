#include <vector>
#include "ElementFactory.h"
#include "HtmlElement.h"


HtmlElement * ElementFactory::create(const std::string & elementType)
{
  return new HtmlElement(elementType);
}

HtmlElement * ElementFactory::create(const std::string & elementType,
    const std::vector<HtmlParser::Attribute*> & attrs)
{
  HtmlElement * element(create(elementType));
  std::vector<HtmlParser::Attribute*>::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it) {
    HtmlParser::Attribute * attr(*it);
    element->setAttribute(attr->name, attr->value);
  }
  return element;
}

void ElementFactory::remove(HtmlElement * element)
{
  delete element;
}
