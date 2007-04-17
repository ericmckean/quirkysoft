#include <assert.h>
#include "ElementFactory.h"
#include "HtmlElement.h"
#include "HtmlDocument.h"

using namespace std;

const static char HEAD_TAG[] = "head";
const static char HTML_TAG[] = "html";
const static char BODY_TAG[] = "body";
const static char FRAMESET_TAG[] = "frameset";
const static char A_TAG[] = "a";
const static char UL_TAG[] = "ul";
const static char LI_TAG[] = "li";
const static char DD_TAG[] = "dd";
const static char DT_TAG[] = "dt";
const static char P_TAG[] = "p";
const static char DIV_TAG[] = "div";
const static char PLAINTEXT_TAG[] = "plaintext";
const static char TABLE_TAG[] = "table";
const static char BR_TAG[] = "br";
const static char FORM_TAG[] = "form";
const static char META_TAG[] = "meta";

#if 1
#include <iostream>
void HtmlDocument::walkNode(const HtmlElement * node)
{
  for (int i =0; i < m_depth; i++)
  {
    cout << "    ";
  }
  if (node->hasChildren())
  {
    cout << "+ " <<node->tagName() << endl;
    m_depth++;
    const ElementList & theChildren = node->children();
    ElementList::const_iterator it(theChildren.begin());
    for (; it != theChildren.end(); ++it)
    {
      walkNode(*it);
    }
    m_depth--;
  } 
  else
  {
    cout << node->tagName() << endl;;
  }
}

void HtmlDocument::dumpAF()
{
  ElementList::const_iterator it(m_activeFormatters.begin());
  cout << "Active formatters " << endl;
  cout << m_activeFormatters.size() << endl;
  for (; it != m_activeFormatters.end(); ++it)
  {
    cout << (*it)->tagName() << endl;
  }
}
void HtmlDocument::dumpDOM()
{
  cout << " Begin DOM: " << endl;
  m_depth = 0;
  cout << endl;
  const HtmlElement * root = rootNode();
  walkNode(root);
  if (m_openElements.size() > 1)
  {
    cout << " ------------- " << endl;
    ElementVector::const_iterator it(m_openElements.begin());
    for (; it != m_openElements.end(); ++it)
    {
      if ( (*it) != root)
      {
        cout << "Open node:" << endl;
        walkNode(*it);
      }
    }
  }
}
#endif

HtmlDocument::HtmlDocument(): 
  m_dataGot(0), 
  m_state(INITIAL),
  m_head(0),
  m_form(0)
{
  m_data.clear();
}

HtmlDocument::~HtmlDocument()
{
  reset();
}

void HtmlDocument::reset() 
{
  m_data.clear();
  
  delete rootNode();
  m_openElements.clear();

  // no need to delete here - each element removes its sons
  /*for_each(m_activeFormatters.begin(), m_activeFormatters.end(), ElementFactory::remove);*/
  m_activeFormatters.clear();

  m_dataGot = 0; 
  m_state = INITIAL;
}

void HtmlDocument::handleStartEndTag(const std::string & tag, const AttributeVector & attrs)
{
  // equivalent to :
  handleStartTag(tag, attrs);
  handleEndTag(tag);
}

void HtmlDocument::beforeHead(const std::string & tag, const AttributeVector & attrs)
{
  if (tag == HEAD_TAG)
  {
    m_head = ElementFactory::create(tag, attrs);
    insertElement(m_head);
    m_insertionMode = IN_HEAD;
  } 
  else
  {
    // push false head tag and reparse this tag (in IN_HEAD mode now)
    handleStartTag(HEAD_TAG, AttributeVector());
    handleStartTag(tag, attrs);
  }
}

void HtmlDocument::beforeHead(const std::string & tag)
{
  if (tag == HTML_TAG)
  {
    // Act as if a start tag token with the tag name HEAD_TAG and no attributes had been seen, then reprocess the current token.
    handleStartTag(HEAD_TAG, AttributeVector());
    handleEndTag(tag);
  }
  // else parse error, ignore end tag.
}

void HtmlDocument::inHead(const std::string & tag, const AttributeVector & attrs)
{

  if (   tag == "script"
      or tag == "style"
      or tag == "title"
     )
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    // insertElement(element);
    m_head->append(element);
    // m_openElements.push_back(element); // no, this is an error, do not push
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
      or tag == META_TAG
      )
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    m_head->append(element);
    if ( tag == META_TAG) {
      checkMetaTagHttpEquiv(element);
    }
    // do not push back
  }
  else
  {
    if (tag != HEAD_TAG)
    {
      if (currentNode()->isa(HEAD_TAG))
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
    if (currentNode()->isa(HEAD_TAG))
    {
      m_openElements.pop_back();
    }
    m_insertionMode = AFTER_HEAD;
  }
  else /*if (tag == HTML_TAG)*/
  {
    if (currentNode()->isa(HEAD_TAG))
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

void HtmlDocument::afterHead(const std::string & tag, const AttributeVector & attrs)
{
  if (tag == BODY_TAG)
  {
    HtmlElement * body = ElementFactory::create(tag, attrs);
    insertElement(body);
    m_insertionMode = IN_BODY;
  }
  else if (tag == FRAMESET_TAG)
  {
    HtmlElement * element = ElementFactory::create(tag, attrs);
    insertElement(element);
    m_insertionMode = IN_FRAMESET;
  }
  else if (   tag == "base" 
      or tag == "link"
      or tag == META_TAG
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
    afterHead(BODY_TAG, AttributeVector());
    handleStartTag(tag, attrs);
  }
}

void HtmlDocument::afterHead(const std::string & tag)
{
  // any other tag - pretend body
  afterHead(BODY_TAG, AttributeVector());
  handleEndTag(tag);
}

void HtmlDocument::inBody(const std::string & tag, const AttributeVector & attrs)
{
  if (   tag == "base" 
      or tag == "link"
      or tag == META_TAG
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
    if (currentNode()->isa(BODY_TAG))
    {
      HtmlElement * body(currentNode());
      setNewAttributes(body, attrs);
    }
  }
  else if ( tag == "address"
      or tag == "blockquote"
      or tag == "center"
      or tag == "dir"
      or tag == DIV_TAG
      or tag == "dl"
      or tag == "fieldset"
      or tag == "listing"
      or tag == "menu"
      or tag == "ol"
      or tag == P_TAG
      or tag == UL_TAG
      or tag == "pre"
      )
  {
    if (inScope(P_TAG))
    {
      handleEndTag(P_TAG);
    }
    HtmlElement * element = ElementFactory::create(tag, attrs);
    insertElement(element);
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
      insertElement(m_form);
    }
  }
  else if (tag == LI_TAG)
  {
    startScopeClosedElement(tag);
    insertElement(ElementFactory::create(tag, attrs));
  }
  else if ( tag == DD_TAG or tag == DT_TAG)
  {
    startScopeClosedElement(DD_TAG, DT_TAG);
    insertElement(ElementFactory::create(tag, attrs));
  }
  else if (tag == PLAINTEXT_TAG)
  {
    if (inScope(P_TAG))
    {
      handleEndTag(P_TAG);
    }
    insertElement(ElementFactory::create(tag, attrs));
    setContentModel(PLAINTEXT);
  }
  else if (tag[0] == 'h' and 
      (tag[1] >= '1' and tag[1] <= '6')
      )
  {
    if (inScope(P_TAG))
    {
      handleEndTag(P_TAG);
    }
    while (headerInScope())
    {
      m_openElements.pop_back();
    }
    insertElement(ElementFactory::create(tag, attrs));
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
    reconstructActiveFormatters();

    /* Insert an HTML element for the token. Add that element to the list of
     * active formatting elements.
     */
    HtmlElement * element = ElementFactory::create(tag, attrs);
    insertElement(element);
    addActiveFormatter(element);
  }
  else if ( tag == "b"
      or tag == "big"
      or tag == "em"
      or tag == "font"
      or tag == "i"
      or tag == "nobr"
      or tag == "s"
      or tag == "small"
      or tag == "strike"
      or tag == "strong"
      or tag == "tt"
      or tag == "u" )
  {
    reconstructActiveFormatters();
    HtmlElement * element = ElementFactory::create(tag, attrs);
    insertElement(element);
    addActiveFormatter(element);
  }
  else if (tag == "area"
      or tag == "basefont"
      or tag == "bgsound"
      or tag == BR_TAG
      or tag == "embed"
      or tag == "img"
      or tag == "param"
      or tag == "spacer"
      or tag == "wbr")
  {
    reconstructActiveFormatters();
    HtmlElement * element = ElementFactory::create(tag, attrs);
    insertElement(element);
    m_openElements.pop_back();
  }
  else
  {

  }
}

void HtmlDocument::inBody(const std::string & tag)
{
  if (tag == BODY_TAG)
  {
    if (currentNode()->isa(BODY_TAG))
    {
      m_insertionMode = AFTER_BODY;
      m_openElements.pop_back();
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
  else if ( tag == "address"
      or tag == "blockquote"
      or tag == "center"
      or tag == "dir"
      or tag == DIV_TAG
      or tag == "dl"
      or tag == "fieldset"
      or tag == "listing"
      or tag == "menu"
      or tag == "ol"
      or tag == "pre"
      or tag == UL_TAG
      )
  {
    if (inScope(tag))
    {
      generateImpliedEndTags();
    }
    if (inScope(tag))
    {
      // if there is a tag like this in scope, keep poppin'
      while (inScope(tag))
      {
        // pop until not in scope.
        const HtmlElement * popped = currentNode();
        m_openElements.pop_back();
        if (popped->isa(tag))
          break;
      }
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
      m_openElements.pop_back();
    }
  }
  else if ( tag == A_TAG
         or tag == "b"
         or tag == "big"
         or tag == "em"
         or tag == "font"
         or tag == "i"
         or tag == "nobr"
         or tag == "s"
         or tag == "small"
         or tag == "strike"
         or tag == "strong"
         or tag == "tt"
         or tag == "u" )
  {
    adoptionAgency(tag);
  }
  else if (tag == LI_TAG or tag == DD_TAG or tag == DT_TAG)
  {
    if (inScope(tag)) {
      generateImpliedEndTags(tag);
    }
    /* If the stack of open elements has an element in scope whose tag name
     * matches the tag name of the token, then pop elements from this stack
     * until an element with that tag name has been popped from the stack.  */
    if (inScope(tag))
    {
      while (not currentNode()->isa(tag))
      {
        m_openElements.pop_back();
      }
      m_openElements.pop_back();
    }
  }
  else if (tag[0] == 'h' and 
      (tag[1] >= '1' and tag[1] <= '6')
      )
  {
    if (headerInScope())
    {
      generateImpliedEndTags();
    }
    while (headerInScope())
    {
      m_openElements.pop_back();
    }
  }
  else if (tag == PLAINTEXT_TAG)
  {
    if (currentNode()->isa(PLAINTEXT_TAG))
    {
      m_openElements.pop_back();
    }
    // else ignore.
  }
  else if (tag == "area"
      or tag == "basefont"
      or tag == "bgsound"
      or tag == BR_TAG
      or tag == "embed"
      or tag == "img"
      or tag == "param"
      or tag == "spacer"
      or tag == "wbr")
  {
    // do nothing
  }
  else
  {
    bool popToNode(false);
    HtmlElement * node(0);
    int index(1);
    while (m_openElements.size())
    {
      node = m_openElements[m_openElements.size()-index];
      if (node->isa(tag)) {
        popToNode = true;
        generateImpliedEndTags();
        node = currentNode();
        break;
      }
      else if (not isFormatting(node) and not isPhrasing(node))
      {
        // ignore end tag
        break;
      }
      index++;
    }
    if (popToNode)
    {
      // pop m_openElements up to and including node
      while (m_openElements.size())
      {
        if (currentNode() == node) {
          m_openElements.pop_back();
          break;
        }
        m_openElements.pop_back();
      }
    }
  }

}

void HtmlDocument::afterBody(const std::string & tag, 
                             const AttributeVector & attrs)
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
void HtmlDocument::mainPhase(const std::string & tag, 
                             const AttributeVector & attrs)
{
  if (tag == HTML_TAG)
  {
    if (m_isFirst)
    {
      // add the attributes to the html node..
      HtmlElement * html = currentNode();
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
  }
}


// main phase end tag
void HtmlDocument::mainPhase(const std::string & tag)
{
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

void HtmlDocument::handleStartTag(const std::string & tag, const AttributeVector & attrs)
{
  switch (m_state)
  {
    case INITIAL:
      m_state = ROOT_ELEMENT;
      /* FALL THROUGH */
    case ROOT_ELEMENT:
      m_state = MAIN;
      m_insertionMode = BEFORE_HEAD;
      m_openElements.push_back(ElementFactory::create(HTML_TAG));
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
      m_openElements.push_back(ElementFactory::create(HTML_TAG));
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

void HtmlDocument::mainPhase(unsigned int ucodeChar)
{
  switch (m_insertionMode)
  {
    case BEFORE_HEAD:
      {
        if (not isWhitespace(ucodeChar)) {
          handleStartTag(HEAD_TAG, AttributeVector());
          mainPhase(ucodeChar);
        }
      }
      break;

    case IN_HEAD:
      {
        if (not isWhitespace(ucodeChar)) {
          if (currentNode()->isa(HEAD_TAG))
          {
            handleEndTag(HEAD_TAG);
          }
          m_insertionMode = AFTER_HEAD;
          mainPhase(ucodeChar);
        }
      }
      break;

    case AFTER_HEAD:
      {
        if (not isWhitespace(ucodeChar)) {
          handleStartTag(BODY_TAG, AttributeVector());
          mainPhase(ucodeChar);
        }
      }
      break;

    case IN_BODY:
      {
        if ((int)ucodeChar == EOF)
        {
          generateImpliedEndTags();
          if (inScope(BODY_TAG)) {
            while (not currentNode()->isa(BODY_TAG))
            {
              m_openElements.pop_back();
            }
          }

          if (m_openElements.size() == 2 and currentNode()->isa(BODY_TAG))
          {
            handleEndTag(BODY_TAG);
            handleEndTag(HTML_TAG);
          }
        }
        else
        {
          reconstructActiveFormatters();
          m_dataGot++;
          currentNode()->appendText(ucodeChar);
        }
      }
      break;

    default :
      break;
  }
}


void HtmlDocument::handleEOF()
{
  handleData(EOF);
  //dumpDOM();
}
void HtmlDocument::handleData(unsigned int ucodeChar)
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
      assert(m_openElements.size() == 0);
      m_openElements.push_back(ElementFactory::create(HTML_TAG));
      m_isFirst = true;
      /* FALL THROUGH */
    case TRAILING_END:
      m_state = MAIN;
      /* FALL THROUGH */
    case MAIN:
      mainPhase(ucodeChar);
      break;
    case MAIN_WAITING_TOKEN:
      // WTF is this?
      if (EOF == (int)ucodeChar) {
        mainPhase(ucodeChar);
      } 
      else
      {
        m_dataGot++;
        m_head->lastChild()->appendText(ucodeChar);
      }
      break;
  }
  // m_data += ucodeChar;
}

const HtmlElement * HtmlDocument::rootNode() const
{
  if (not m_openElements.empty()) {
    return currentNode();
  }
  return 0;
}

void HtmlDocument::setNewAttributes(HtmlElement * element, const AttributeVector & attrs)
{
  AttributeVector::const_iterator it(attrs.begin());
  for (; it != attrs.end(); ++it) {
    Attribute * attr(*it);
    element->setAttribute(attr->name, attr->value);
  }
}

bool HtmlDocument::inScope(const std::string & element) const
{
   ElementVector::const_reverse_iterator it(m_openElements.rbegin());
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
  
  ElementList::iterator it = m_activeFormatters.begin();
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
  ElementList::iterator it = find(m_activeFormatters.begin(), m_activeFormatters.end(), element);
  if (it != m_activeFormatters.end())
  {
    m_activeFormatters.erase(it);
  }
}

void HtmlDocument::removeFromOpenElements(HtmlElement* element)
{
  ElementVector::iterator it = find(m_openElements.begin(), m_openElements.end(), element);
  if (it != m_openElements.end())
  {
    m_openElements.erase(it);
  }
}

void HtmlDocument::insertElement(HtmlElement * element)
{
  currentNode()->append(element);
  m_openElements.push_back(element);
}
void HtmlDocument::addActiveFormatter(HtmlElement * element)
{
  m_activeFormatters.push_front(element);
}

void HtmlDocument::generateImpliedEndTags(const std::string & except)
{
  HtmlElement * node(currentNode());
  if (   node->isa(P_TAG)
      or node->isa(DD_TAG)
      or node->isa(DT_TAG)
      or node->isa(LI_TAG)
      or node->isa("td")
      or node->isa("th")
      or node->isa("tr")
      or node->isa(PLAINTEXT_TAG)
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
  // if some unrecognised node
  return false;
}

void HtmlDocument::reconstructActiveFormatters()
{
  if (m_activeFormatters.size() == 0) 
  {
    return;
  }
  // marker?
  ElementVector::const_iterator mostRecent = find(m_openElements.begin(), m_openElements.end(), m_activeFormatters.front());
  if (mostRecent != m_openElements.end())
  {
    // the current active formatter is still open, return
    return;
  }

  ElementList::iterator it(m_activeFormatters.begin());
  //3. Let entry be the last (most recently added) element in the list of active formatting elements.
  for (; it != m_activeFormatters.end(); ++it)
  {
    //4. If there are no entries before entry in the list of active formatting elements, then jump to step 8.
    HtmlElement * entry = *it;
    bool lastEntry(false);
    ElementList::iterator currentIt(it);
    ++it;
    if  ( it != m_activeFormatters.end())
    {
      lastEntry = true;
      //5. Let entry be the entry one earlier than entry in the list of active formatting elements.
      entry = *it;
      //6. If entry is neither a marker nor an element that is also in the stack of open elements, go to step 4.
      ElementVector::const_iterator onOpen = find(m_openElements.begin(), m_openElements.end(), entry);
      if ( not entry->isa("marker") and onOpen == m_openElements.end())
      {
        continue;
      }
      //7. Let entry be the element one later than entry in the list of active formatting elements.
      it = currentIt;
      entry = *it;
    } 
    else
    {
      it = currentIt;
    }
    //8. Perform a shallow clone of the element entry to obtain clone. [DOM3CORE]
    HtmlElement * clone = entry->clone();
    //9. Append clone to the current node and push it onto the stack of open elements so that it is the new current node.
    insertElement(clone);
    //10 Replace the entry for entry in the list with an entry for clone.
    *it = clone;
    //11 If the entry for clone in the list of active formatting elements is not the last entry in the list, return to step 7. 
  }
}


struct Bookmark
{
  const HtmlElement * before;
  const HtmlElement * after;
};

void HtmlDocument::createBookmark(Bookmark & marker, ElementList::iterator & bookmarkIt) const
{
  marker.before = 0;
  marker.after = 0;
  ++bookmarkIt;
  if (bookmarkIt != m_activeFormatters.end())
  {
    marker.after = *bookmarkIt;
  }
  --bookmarkIt;
  --bookmarkIt;
  if (bookmarkIt != m_activeFormatters.end())
  {
    marker.before = *bookmarkIt;
  }
}


void HtmlDocument::adoptionAgency(const std::string & tag)
{
  /*
   * 1.
   * Let the formatting element be the last element in the list of active
   * formatting elements that:
   *
   * * is between the end of the list and the last scope marker in the 
   * list, if any, or the start of the list otherwise, and
   * * has the same tag name as the token. 
   *
   * If there is no such node, or, if that node is also in the stack of open
   * elements but the element is not in scope, then this is a parse error. Abort
   * these steps. The token is ignored.
   *
   * Otherwise, if there is such a node, but that node is not in the stack of open
   * elements, then this is a parse error; remove the element from the list, and
   * abort these steps.
   *
   * Otherwise, there is a formatting element and that element is in the stack and
   * is in scope. If the element is not the current node, this is a parse error. In
   * any case, proceed with the algorithm as written in the following steps.
   */
  for (;;)
  {
    // FIXME - scope for buttons.
    HtmlElement * formattingElement(0);
    {
      ElementList::reverse_iterator activeFormat = 
        find_if( m_activeFormatters.rbegin(), 
            m_activeFormatters.rend(),
            bind2nd( mem_fun(&HtmlElement::isa_ptr), &tag)
            );
      if (activeFormat == m_activeFormatters.rend())
      {
        // ignore - no <tag> open tag in scope
        return;
      }
      formattingElement = *activeFormat;
    }
    ElementVector::iterator openElement = find(m_openElements.begin(), 
        m_openElements.end(), 
        formattingElement);
    if (openElement == m_openElements.end())
    {
      // woops - this is a parse error - <tag> is not on the open elements list.
      removeFromActiveFormat(formattingElement);
      return;
    }
    // have a format element, it is in scope and on the open stack. if it is
    // not the current node, then it is a parse error.
    // but carry on anyway.

    /*
       2.
       Let the furthest block be the topmost node in the stack of open elements
       that is lower in the stack than the formatting element, and is not an
       element in the phrasing or formatting categories. There might not be one.
       */
    HtmlElement * furthestBlock(0);
    // save the iterator for later
    ElementVector::iterator commonAncestorIt(openElement);
    ++openElement;
    for (; openElement != m_openElements.end(); ++openElement)
    {
      HtmlElement * element(*openElement);
      if ( not isFormatting(element) and not isPhrasing(element))
      {
        // found the furthest block;
        furthestBlock = element;
        break;
      }
    }
    if (furthestBlock == 0)
    {
      /* 
         3. If there is no furthest block, then the UA must skip the subsequent steps
         and instead just pop all the nodes from the bottom of the stack of open
         elements, from the current node up to the formatting element, and remove
         the formatting element from the list of active formatting elements.
         */
      while (currentNode() != formattingElement)
      {
        if (isFormatting(currentNode())) {
          removeFromActiveFormat(currentNode());
        }
        m_openElements.pop_back();
      }
      removeFromActiveFormat(currentNode());
      // pop the formatting element too
      m_openElements.pop_back();
      return;
    }

    /* 4.  Let the common ancestor be the element immediately above the
     * formatting element in the stack of open elements.  */
    --commonAncestorIt;
    HtmlElement * commonAncestor = *commonAncestorIt;

    /* 5. If the furthest block has a parent node, then remove the furthest
     * block from its parent node.*/
    if (furthestBlock->parent() != 0)
    {
      furthestBlock->parent()->remove(furthestBlock);
    }

    /* 6.  Let a bookmark note the position of the formatting element in the
     * list of active formatting elements relative to the elements on either
     * side of it in the list.*/
    ElementList::iterator bookmarkIt = find(m_activeFormatters.begin(), 
        m_activeFormatters.end(), 
        formattingElement);
    Bookmark marker;
    createBookmark(marker, bookmarkIt);

    /* 7.  Let node and last node be the furthest block. Follow these steps: */
    HtmlElement * node(furthestBlock);
    HtmlElement * lastNode(furthestBlock);
    for (;;)
    {
      /*
       *   1. Let node be the element immediately prior to node in the stack of
       *      open elements.
       */
      ElementVector::iterator priorNode = find(m_openElements.begin(), 
          m_openElements.end(), 
          node);
      --priorNode;
      node = *priorNode;
      /*
       *   2. If node is not in the list of active formatting elements, then
       *      remove node from the stack of open elements and then go back to step 1.
       */
      ElementList::iterator activeFormat = find(m_activeFormatters.begin(),
          m_activeFormatters.end(), 
          node);
      if (activeFormat == m_activeFormatters.end())
      {
        removeFromOpenElements(node);
        continue;
      }

      /*
       *   3. Otherwise, if node is the formatting element, then go to the next
       *      step in the overall algorithm.
       */
      if (formattingElement == node)
      {
        break;
      }
      /*
       *   4. Otherwise, if last node is the furthest block, then move the
       *      aforementioned bookmark to be immediately after the node in the list of
       *      active formatting elements.
       */
      if (lastNode == furthestBlock)
      {
        // move bookmark 
        createBookmark(marker, activeFormat);
        //bookmark = activeFormat;
        //++bookmark; // or --?
      }
      /*
       *   5. If node has any children, perform a shallow clone of node, replace
       *      the entry for node in the list of active formatting elements with an
       *      entry for the clone, replace the entry for node in the stack of open
       *      elements with an entry for the clone, and let node be the clone.
       */
      if (node->hasChildren())
      {
        HtmlElement * clone = node->clone();
        ElementVector::iterator nodeOnOE = find(m_openElements.begin(), 
            m_openElements.end(), 
            node);
        *nodeOnOE = clone;
        ElementList::iterator nodeOnAF = find(m_activeFormatters.begin(), 
            m_activeFormatters.end(),
            node);
        *nodeOnAF = clone;
        node = clone;
      }
      /*   6. Insert last node into node, first removing it from its previous
       *      parent node if any.
       */
      if (lastNode->parent())
      {
        lastNode->parent()->remove(lastNode);
      }
      node->append(lastNode);

      /*   7. Let last node be node. */
      lastNode = node;
      /*   8. Return to step 1 of this inner set of steps. */
    }

    /* 8.  Insert whatever last node ended up being in the previous step into
     * the common ancestor node, first removing it from its previous parent
     * node if any.  */
    if (lastNode->parent())
    {
      lastNode->parent()->remove(lastNode);
    }
    commonAncestor->append(lastNode);

    /* 9.  Perform a shallow clone of the formatting element. */
    HtmlElement * clone = formattingElement->clone();

    /* 10.  Take all of the child nodes of the furthest block and append them
     *      to the clone created in the last step.  */
    if (furthestBlock->hasChildren())
    {
      ElementList::const_iterator childIt = furthestBlock->children().begin();
      for (; childIt != furthestBlock->children().end(); ++childIt)
      {
        clone->append(*childIt);
      }
      furthestBlock->removeAllChildren();
    }

    /* 11.  Append that clone to the furthest block. */
    furthestBlock->append(clone);
    /* 12.  Remove the formatting element from the list of active formatting
     * elements, and insert the clone into the list of active formatting
     * elements at the position of the aforementioned bookmark. */
    removeFromActiveFormat(formattingElement);
    bool inserted(false);
    if (marker.before == 0 and marker.after == 0)
    {
      addActiveFormatter(clone);
      inserted = true;
    }
    else
    {
      if (marker.before != 0) 
      {
        ElementList::iterator b4 = find(m_activeFormatters.begin(), m_activeFormatters.end(), marker.before);
        if (b4 != m_activeFormatters.end())
        {
          ++b4;
          m_activeFormatters.insert(b4, clone);
          inserted = true;
        }
      }
      if (not inserted and marker.after != 0)
      {
        ElementList::iterator after = find(m_activeFormatters.begin(), m_activeFormatters.end(), marker.after);
        if (after != m_activeFormatters.end())
        {
          ++after;
          m_activeFormatters.insert(after, clone);
          inserted = true;
        }
      }
    }
    assert(inserted);

    /* 13.  Remove the formatting element from the stack of open elements, and
     * insert the clone into the stack of open elements immediately after (i.e.
     * in a more deeply nested position than) the position of the furthest
     * block in that stack.  */
    removeFromOpenElements(formattingElement);
    ElementVector::iterator clonePosition = find(m_openElements.begin(), 
        m_openElements.end(), 
        furthestBlock);
    ++clonePosition;
    m_openElements.insert(clonePosition, clone);

    /* 14.  Jump back to step 1 in this series of steps. */
  }
}

void HtmlDocument::startScopeClosedElement(const std::string & tag, const std::string & alternate)
{
  /* If the stack of open elements has a p element in scope, then act as if an
   * end tag with the tag name p had been seen.  */
  if (inScope(P_TAG))
  {
    handleEndTag(P_TAG);
  }
  /* Run the following algorithm:
   * 1. Initialise node to be the current node (the bottommost node of the
   * stack).  */
  ElementVector::reverse_iterator it(m_openElements.rbegin());
  for (; it != m_openElements.rend(); ++it) {
    HtmlElement * node(*it);
    /* 2. If node is an li element, then pop all the nodes from the current
     * node up to node, including node, then stop this algorithm.  */
    if (node->isa(tag) or node->isa(alternate)  )
    {
      while (currentNode() != node)
      {
        m_openElements.pop_back();
      }
      m_openElements.pop_back();
      break;
    }

    /* 3. If node is not in the formatting category, and is not in the
     * phrasing category, and is not an address or div element, then stop
     * this algorithm.  */
    if (not isFormatting(node) and not isPhrasing(node) and not node->isa("address") and not node->isa(DIV_TAG))
    {
      break;
    }
    /* 4. Otherwise, set node to the previous entry in the stack of open
     * elements and return to step 2.  */
  }
}

bool HtmlDocument::headerInScope() const
{
  return inScope("h1") or inScope("h2") or inScope("h3") or inScope("h4") or inScope("h5") or inScope("h6");
}

