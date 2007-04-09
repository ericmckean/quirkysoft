//#include <iostream>
#include "ElementFactory.h"
#include "HtmlElement.h"
#include "HtmlDocument.h"
#include "HeaderParser.h"

using namespace std;

const static char HEAD_TAG[] = "head";
const static char HTML_TAG[] = "html";
const static char BODY_TAG[] = "body";
const static char FRAMESET_TAG[] = "frameset";
const static char A_TAG[] = "a";
const static char P_TAG[] = "p";
const static char TABLE_TAG[] = "table";
const static char FORM_TAG[] = "form";


HtmlDocument::HtmlDocument(): 
  m_dataGot(0), 
  m_headerParser(0),
  m_state(INITIAL),
  m_head(0),
  m_form(0)
{
  m_data.clear();
}
void HtmlDocument::reset() 
{
  m_data.clear();
  
  while (not m_openElements.empty())
  {
    ElementFactory::remove(m_openElements.front());
    m_openElements.pop_front();
  }

  for_each(m_activeFormatters.begin(), m_activeFormatters.end(), ElementFactory::remove);
  m_activeFormatters.clear();

  m_dataGot = 0; 
  m_state = INITIAL;
}

void HtmlDocument::handleStartEndTag(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  // equivalent to :
  handleStartTag(tag, attrs);
  handleEndTag(tag);
}

void HtmlDocument::beforeHead(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  if (tag == HEAD_TAG)
  {
    m_head = ElementFactory::create(tag, attrs);
    m_openElements.front()->append(m_head);
    m_openElements.push_front(m_head);
    m_insertionMode = IN_HEAD;
  } 
  else
  {
    // push false head tag and reparse this tag (in IN_HEAD mode now)
    handleStartTag(HEAD_TAG, vector<Attribute*>());
    handleStartTag(tag, attrs);
  }
}

void HtmlDocument::beforeHead(const std::string & tag)
{
  if (tag == HTML_TAG)
  {
    // Act as if a start tag token with the tag name HEAD_TAG and no attributes had been seen, then reprocess the current token.
    handleStartTag(HEAD_TAG, vector<Attribute*>());
    handleEndTag(tag);
  }
  // else parse error, ignore end tag.
}

void HtmlDocument::inHead(const std::string & tag, const std::vector<Attribute*> & attrs)
{

  if (   tag == "script"
      or tag == "style"
      or tag == "title"
     )
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_head->append(element);
    // m_openElements.push_front(element); // no, this is an error, do not push
    if (tag == "title") {
      setContentModel(RCDATA);
    }
    if (tag == "style" or tag == "script") {
      setContentModel(CDATA);
    }
    // collect all character tokens until a non character one is returned.
    m_state = MAIN_WAITING_TOKEN;
  }
  else if (
         tag == "base" 
      or tag == "link"
      or tag == "meta"
      )
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_head->append(element);
  }
  else
  {
    if (tag != HEAD_TAG)
    {
      if (m_openElements.front()->isa(HEAD_TAG))
      {
        handleEndTag(HEAD_TAG);
      }
      else
      {
        m_insertionMode = AFTER_HEAD;
      }
      handleStartTag(tag, attrs);
    }
    // else another head tag - parse error, ignore.
  }
}

void HtmlDocument::inHead(const std::string & tag)
{
  if (tag == HEAD_TAG)
  {
    if (m_openElements.front()->isa(HEAD_TAG))
    {
      m_openElements.pop_front();
    }
    m_insertionMode = AFTER_HEAD;
  }
  else /*if (tag == HTML_TAG)*/
  {
    if (m_openElements.front()->isa(HEAD_TAG))
    {
      handleEndTag(HEAD_TAG);
    }
    else 
    {
      m_insertionMode = AFTER_HEAD;
    }
    handleEndTag(tag);
  }
  // Possibly broken here... title etc?
}

void HtmlDocument::afterHead(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  if (tag == BODY_TAG)
  {
    HtmlElement * body = ElementFactory::create(tag, attrs);
    m_openElements.front()->append(body);
    m_openElements.push_front(body);
    m_insertionMode = IN_BODY;
  }
  else if (tag == FRAMESET_TAG)
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_openElements.front()->append(element);
    m_openElements.push_front(element);
    m_insertionMode = IN_FRAMESET;
  }
  else if (   tag == "base" 
      or tag == "link"
      or tag == "meta"
      or tag == "script"
      or tag == "style"
      or tag == "title"
     )
  {
    // Woops, parse error - reprocess
    m_insertionMode = IN_HEAD;
    handleStartTag(tag, attrs);
  }
  else
  {
    // any other start tag - pretend body
    afterHead(BODY_TAG, vector<Attribute*>());
    handleStartTag(tag, attrs);
  }
}

void HtmlDocument::afterHead(const std::string & tag)
{
  // any other tag - pretend body
  afterHead(BODY_TAG, vector<Attribute*>());
  handleEndTag(tag);
}

void HtmlDocument::inBody(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  if (   tag == "base" 
      or tag == "link"
      or tag == "meta"
      or tag == "script"
      or tag == "style"
      or tag == "title"
     )
  {
    // Parse error.. or not. Process as if in IN_HEAD mode
    m_insertionMode = IN_HEAD;
    handleStartTag(tag, attrs);
    m_insertionMode = IN_BODY;
  }
  else if (tag == BODY_TAG)
  {
    // parse error - update any attributes though
    if (m_openElements.front()->isa(BODY_TAG))
    {
      HtmlElement * body(m_openElements.front());
      setNewAttributes(body, attrs);
    }
  }
  else if ( tag == "address"
      or tag == "blockquote"
      or tag == "center"
      or tag == "dir"
      or tag == "div"
      or tag == "dl"
      or tag == "fieldset"
      or tag == "listing"
      or tag == "menu"
      or tag == "ol"
      or tag == P_TAG
      or tag == "ul"
      or tag == "pre"
      )
  {
    if (inScope(P_TAG))
    {
      handleEndTag(P_TAG);
    }
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_openElements.front()->append(element);
    m_openElements.push_front(element);
    // FIXME: if tag == "pre" then eat following LF (if any)
  }
  else if (tag == FORM_TAG)
  {
    if (not m_form)
    {
      if (inScope(P_TAG))
      {
        handleEndTag(P_TAG);
      }
      m_form = ElementFactory::create(tag, attrs);
      m_openElements.front()->append(m_form);
      m_openElements.push_front(m_form);

    }
  }
  else if (tag == A_TAG)
  {
    HtmlElement * activeFormatA = activeFormatContains(A_TAG);
    if (activeFormatA != 0)
    {
      // parse error.
      handleEndTag(A_TAG);
      removeFromActiveFormat(activeFormatA);
      removeFromOpenElements(activeFormatA);
    }

    /* Reconstruct the active formatting elements, if any. */

    /* Insert an HTML element for the token. Add that element to the list of
     * active formatting elements.
     */
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_openElements.front()->append(element);
    m_openElements.push_front(element);
  }
  else
  {

  }
}

void HtmlDocument::inBody(const std::string & tag)
{
  if (tag == BODY_TAG)
  {
    if (m_openElements.front()->isa(BODY_TAG))
    {
      m_insertionMode = AFTER_BODY;
      m_openElements.pop_front();
    }
    // else ignore the token, parse error
  }
  else if (tag == HTML_TAG)
  {
    handleEndTag(BODY_TAG);
    if (m_insertionMode == AFTER_BODY) {
      handleEndTag(tag);
    }
  }
  else if (tag == P_TAG)
  {
    if (inScope(P_TAG))
    {
      generateImpliedEndTags(P_TAG);
    }

    while (inScope(P_TAG))
    {
      // pop until P not in scope.
      m_openElements.pop_front();
    }
  }
  else
  {
    bool popToNode(false);
    HtmlElement * node(0);
    while (m_openElements.size())
    {
      node = m_openElements.front();
      if (node->isa(tag)) {
        popToNode = true;
        generateImpliedEndTags();
        node = m_openElements.front();
        break;
      }
      else if (not isFormatting(node) and not isPhrasing(node))
      {
        // ignore end tag
        break;
      }
    }
    if (popToNode)
    {
      // pop m_openElements up to and including node
      while (m_openElements.size())
      {
        if (m_openElements.front() == node) {
          m_openElements.pop_front();
          break;
        }
        m_openElements.pop_front();
      }
    }
  }

}

void HtmlDocument::afterBody(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  m_insertionMode = IN_BODY;
}

void HtmlDocument::afterBody(const std::string & tag)
{
  if (tag == HTML_TAG)
  {
    m_state = TRAILING_END;
  }
  else
  {
    m_insertionMode = IN_BODY;
  }
}

// main phase start tag
void HtmlDocument::mainPhase(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  // cout << "Main phase " << m_insertionMode << " open :" << tag << endl;
  if (tag == HTML_TAG)
  {
    if (m_isFirst)
    {
      // add the attributes to the html node..
      HtmlElement * html = m_openElements.front();
      setNewAttributes(html, attrs);
    }
    // else parse error.
    m_isFirst = false;
  }
  else {
    switch (m_insertionMode)
    {
      case BEFORE_HEAD:
        beforeHead(tag, attrs);
        break;
      case IN_HEAD:
        inHead(tag, attrs);
        break;
      case AFTER_HEAD:
        afterHead(tag, attrs);
        break;
      case IN_BODY:
        inBody(tag, attrs);
        break;
      case AFTER_BODY:
        afterBody(tag, attrs);
        break;
      default:
        break;
    };
    if ( tag == "meta") {
      m_headerParser->checkMetaTagHttpEquiv(attrs);
    }
  }
}


// main phase end tag
void HtmlDocument::mainPhase(const std::string & tag)
{
  // cout << "main phase " << m_insertionMode << " close:" << tag << endl;
  switch (m_insertionMode)
  {
    case BEFORE_HEAD:
      beforeHead(tag);
      break;
    case IN_HEAD:
      inHead(tag);
      break;
    case AFTER_HEAD:
      afterHead(tag);
      break;
    case IN_BODY:
      inBody(tag);
      break;
    case AFTER_BODY:
      afterBody(tag);
      break;
    default:
      break;
  }

}

void HtmlDocument::handleStartTag(const std::string & tag, const std::vector<Attribute*> & attrs)
{
  switch (m_state)
  {
    case INITIAL:
      m_state = ROOT_ELEMENT;
      /* FALL THROUGH */
    case ROOT_ELEMENT:
      m_state = MAIN;
      m_insertionMode = BEFORE_HEAD;
      m_openElements.push_front(ElementFactory::create(HTML_TAG));
      m_isFirst = true;
      /* FALL THROUGH */
    case TRAILING_END:
      m_state = MAIN;
      /* FALL THROUGH */
    case MAIN:
      mainPhase(tag, attrs);
      break;
    case MAIN_WAITING_TOKEN:
      m_state = MAIN;
      // either ignore, or a parse error.. either way do nowt.
      break;
  }
}
void HtmlDocument::handleEndTag(const std::string & tag)
{
  switch (m_state)
  {
    case INITIAL:
      m_state = ROOT_ELEMENT;
      /* FALL THROUGH */
    case ROOT_ELEMENT:
      // if an end tag appears first, then append a html tag and redo this tag,
      m_state = MAIN;
      m_insertionMode = BEFORE_HEAD;
      m_openElements.push_front(ElementFactory::create(HTML_TAG));
      m_isFirst = true;
      /* FALL THROUGH */
    case TRAILING_END:
      m_state = MAIN;
      /* FALL THROUGH */
    case MAIN:
      mainPhase(tag);
      break;
    case MAIN_WAITING_TOKEN:
      // ignore or a parse error.
      m_state = MAIN;
      break;
  }
}

void HtmlDocument::handleData(unsigned int ucodeChar)
{
  m_dataGot += 1;
  if (m_openElements.size())
    m_openElements.front()->append(ucodeChar);
  // m_data += ucodeChar;
}

const HtmlElement * HtmlDocument::rootNode() const
{
  if (not m_openElements.empty()) {
    return m_openElements.front();
  }
  return 0;
}

void HtmlDocument::setNewAttributes(HtmlElement * element, const std::vector<Attribute*> & attrs)
{
  vector<Attribute*>::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it) {
    Attribute * attr(*it);
    element->setAttribute(attr->name, attr->value);
  }
}

bool HtmlDocument::inScope(const std::string & element)
{
   list<HtmlElement*>::const_reverse_iterator it(m_openElements.rbegin());
   for (; it != m_openElements.rend(); ++it) {
     HtmlElement * node = *it;
     if (node->isa(element))
     {
       return true;
     }
   }
   return false;
}

HtmlElement* HtmlDocument::activeFormatContains(const std::string & tagName)
{
  
  list<HtmlElement*>::iterator it = m_activeFormatters.begin();
  for (; it != m_activeFormatters.end(); ++it)
  {
    if ( (*it)->isa(tagName))
      break;
  }
  if (it != m_activeFormatters.end())
  {
    return *it;
  }
  return 0;
}

void HtmlDocument::removeFromActiveFormat(HtmlElement* element)
{
  list<HtmlElement*>::iterator it = find(m_activeFormatters.begin(), m_activeFormatters.end(), element);
  if (it != m_activeFormatters.end())
  {
    m_activeFormatters.erase(it);
  }
}

void HtmlDocument::removeFromOpenElements(HtmlElement* element)
{
  list<HtmlElement*>::iterator it = find(m_openElements.begin(), m_openElements.end(), element);
  if (it != m_activeFormatters.end())
  {
    m_openElements.erase(it);
  }
}


void HtmlDocument::generateImpliedEndTags(const string & except)
{
  HtmlElement * node(m_openElements.front());
  if (   node->isa(P_TAG)
      or node->isa("dd")
      or node->isa("dt")
      or node->isa("li")
      or node->isa("td")
      or node->isa("th")
      or node->isa("tr")
     )
  {
    if (not node->isa(except))
    {
      handleEndTag(node->tagName());
      generateImpliedEndTags(except);
    }
  }
}

bool HtmlDocument::isFormatting(HtmlElement * node)
{
  return node->isa(A_TAG) 
    or node->isa("b")
    or node->isa("big")
    or node->isa("em")
    or node->isa("font")
    or node->isa("i")
    or node->isa("nobr")
    or node->isa("s")
    or node->isa("small")
    or node->isa("strike")
    or node->isa("strong")
    or node->isa("tt")
    or node->isa("u");
}
bool HtmlDocument::isPhrasing(HtmlElement * node)
{
  return false;
}
