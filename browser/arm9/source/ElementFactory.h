#ifndef ElementFactory_h_seen
#define ElementFactory_h_seen

#include <string>
#include "HtmlParser.h"
class HtmlElement;
class ElementFactory
{
  public:
    static HtmlElement * create(const std::string & elementType);
    static HtmlElement * create(const std::string & elementType,
        const std::vector<HtmlParser::Attribute*> & attrs);

    static void remove(HtmlElement * element);

  private:
    ElementFactory();
    ~ElementFactory();
};
#endif
