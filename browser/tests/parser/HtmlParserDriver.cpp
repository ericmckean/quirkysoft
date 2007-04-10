#include "HtmlParserDriver.h"
#include "HeaderParser.h"
#include "ElementFactory.h"
#include "HtmlElement.h"
using namespace std;

void HtmlParserDriver::handleStartEndTag(const string & tag, const vector<Attribute*> & attrs)
{
  handleStartTag(tag, attrs);
}

void HtmlParserDriver::handleStartTag(const string & tag, const vector<Attribute*> & attrs)
{
  m_tags.push_back(tag);
  vector<Attribute*> newVector;
  vector<Attribute*>::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it)
  {
    Attribute * at(*it);
    Attribute * newAttr = new Attribute(*at);
    newVector.push_back(newAttr);
  }
  // gah - copy paste from Document, no other way
  HtmlElement * element = ElementFactory::create(tag, attrs);
  if (tag == "meta") { 
    m_headerParser->checkMetaTagHttpEquiv(element); 
  }
  delete element;
  m_attributes.push_back(newVector);
}

void HtmlParserDriver::handleEndTag(const string & tag)
{
  m_tags.push_back(tag);
}

void HtmlParserDriver::handleData(unsigned int ucodeChar)
{
  if (ucodeChar == '\n')
    return;
  m_data += ucodeChar;
}
