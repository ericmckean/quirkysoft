#include <vector>
#include "ElementFactory.h"
#include "HtmlAnchorElement.h"
#include "HtmlMetaElement.h"
#include "HtmlElement.h"


HtmlElement * ElementFactory::create(const std::string & elementType)
{
  if (elementType == "a")
  {
    return new HtmlAnchorElement(elementType);
  }
  else if (elementType == "meta")
  {
    return new HtmlMetaElement(elementType);
  }

  return new HtmlElement(elementType);
}

HtmlElement * ElementFactory::create(const std::string & elementType,
    const AttributeVector & attrs)
{
  HtmlElement * element(create(elementType));
  AttributeVector::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it) {
    Attribute * attr(*it);
    element->setAttribute(attr->name, attr->value);
  }
  return element;
}

void ElementFactory::remove(HtmlElement * element)
{
  delete element;
}