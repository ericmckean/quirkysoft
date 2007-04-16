#include <assert.h>
#include <libgen.h>
#include <iostream>
#include <list>
#include "ndspp.h"
#include "libnds.h"
#include "View.h"
#include "Document.h"
#include "URI.h"
#include "HtmlElement.h"
#include "HtmlImageElement.h"
#include "TextArea.h"
#include "Canvas.h"
#include "ControllerI.h"
#include "Keyboard.h"
#include "Link.h"
#include "File.h"

using namespace std;

View::View(Document & doc, ControllerI & c):
  m_document(doc), 
  m_controller(c),
  m_textArea(new TextArea),
  m_keyboard(new Keyboard(*m_textArea)),
  m_pendingNewLines(0),
  m_haveShownSomething(false)
{
  m_document.registerView(this);
  keysSetRepeat( 10, 5 );
}

void View::addRealNewline(int count)
{
  m_textArea->setParseNewline(true);
  UnicodeString newlines(count, '\n');
  m_textArea->printu(newlines);
  m_textArea->setParseNewline(false);
}

void View::preNodeFormatting(const HtmlElement * node)
{
  if (node->isa("li"))
  {
    HtmlElement p("p");
    p.appendText(' ');
    p.appendText('*');
    p.appendText(' ');
    applyFormatting(p.firstChild());
  }
  else if (node->isa("ul"))
  {
    if ( node->parent()->isa("li") or node->parent()->isa("ul"))
      m_pendingNewLines = 0;
    else
      m_pendingNewLines++;
  }
  else if (node->tagName()[0] == 'h' and (node->tagName()[1] >= '1' and node->tagName()[1] <= '6'))
  {
    if (m_pendingNewLines < 2)
      m_pendingNewLines++;;
  }
  else if (node->isa("p"))
  {
    m_pendingNewLines+=2;
  }
}

void View::postNodeFormatting(const HtmlElement * node)
{
  if (node->isa("li"))
  {
    m_pendingNewLines++;
  }
  else if (node->isa("a"))
  {
    m_textArea->setColor(nds::Color(0,0,0));
    m_textArea->setLink(false);
  }
  else if (node->isa("div"))
  {
    m_pendingNewLines++;
  }
  else if (node->tagName()[0] == 'h' and (node->tagName()[1] >= '1' and node->tagName()[1] <= '6'))
  {
    m_pendingNewLines += 2;
  } 
  else if (node->isa("ul"))
  {
    if (node->parent()->isa("li") or node->parent()->isa("ul"))
      m_pendingNewLines = 0;
    else
      m_pendingNewLines++;
  }
  else if (node->isa("p"))
  {
    m_pendingNewLines+=2;
  }
  else if (node->isa("br"))
  {
    flushNewlines();
    m_pendingNewLines++;
    flushNewlines();
  } 
}

void View::flushNewlines()
{
  if (m_pendingNewLines and m_haveShownSomething)
  {
    addRealNewline(m_pendingNewLines>2?2:m_pendingNewLines);
  }
  m_pendingNewLines = 0;
}

bool View::applyFormatting(const HtmlElement * element)
{
  if (element->isa("a"))
  {
    m_textArea->setColor(nds::Color(0,0,31));
    m_textArea->addLink(element);
  } 
  else if (element->text().size())
  {
    UnicodeString delims((unsigned int)' ',1); delims += '\n';
    unsigned int pos = element->text().find_first_not_of(delims);
    if (pos != UnicodeString::npos) {
      // if grandparent is a li then change of model
      if ( ( element->parent()->isa("ul") or element->parent()->isa("ol") ) 
          and element->parent()->parent()->isa("li"))
        m_pendingNewLines = 1;
      flushNewlines();
      m_haveShownSomething = true;
      m_textArea->printu(element->text());
      if (element->parent()->isa("ul"))
        m_pendingNewLines++;
    }
  }
  else if (element->isa("script") or element->isa("style"))
  {
    return false;
  }
  else if (element->isa("img"))
  {
    // hurrah for alt text. some people set it to "", which screws up any
    // nice ways to display it (see w3m google.com - Google [hp1] [hp2] [hp3]... huh?)
    flushNewlines();
    const UnicodeString & altText = element->attribute("alt");
    bool hasAltText = ((HtmlImageElement*)element)->hasAltText();
    if (hasAltText) {
      if (not altText.empty()) {
        m_textArea->setColor(nds::Color(0,31,0));
        m_textArea->printu(altText);
        m_textArea->setColor(nds::Color(0,0,0));
      }
      return true;
    }
    const UnicodeString & srcText = element->attribute("src");
    m_textArea->setColor(nds::Color(0,31,0));
    if (not srcText.empty())
    {
      string tmp = unicode2string(srcText);
      const char * bname = nds::File::base(tmp.c_str());
      string bnamestr(bname);
      bnamestr = "["+ bnamestr+"]";
      UnicodeString ustr ( string2unicode(bnamestr));
      m_textArea->printu(ustr);
      m_textArea->setColor(nds::Color(0,0,0));
      return true;
    }
    UnicodeString ustr( string2unicode("[IMG]"));
    m_textArea->printu(ustr);
    m_textArea->setColor(nds::Color(0,0,0));
    return true;
  }
  return true;
}

void View::walkNode(const HtmlElement * node)
{
  preNodeFormatting(node);
  if (node->hasChildren())
  {
    const ElementList & theChildren = node->children();
    ElementList::const_iterator it(theChildren.begin());
    for (; it != theChildren.end(); ++it)
    {
      const HtmlElement * element(*it);
      if (applyFormatting(element)) {
        walkNode(element);
      }
    }
  }
  postNodeFormatting(node);
}

void View::render()
{
  nds::Canvas::instance().fillRectangle(0, 0, SCREEN_WIDTH, 2*SCREEN_HEIGHT, nds::Color(31,31,31));
  m_textArea->setCursor(0, 0);
  m_textArea->setParseNewline(false);
  m_pendingNewLines = 0;
  m_haveShownSomething = false;
  const HtmlElement * root = m_document.rootNode();
  assert(root->isa("html"));
  assert(root->hasChildren());
  const HtmlElement * body = root->lastChild();
  assert(body->hasChildren());
  walkNode(body);
}


void View::notify()
{
  Document::Status status(m_document.status());

  switch (status) {
    case Document::LOADED:
      {
        m_textArea->setStartLine( (-SCREEN_HEIGHT / m_textArea->font().height()) - 1);
        render();
        swiWaitForVBlank();
      }
      break;
    case Document::INPROGRESS:
      {
        const char * l = "Loading..";
        m_textArea->setCursor(0, 0);
        m_textArea->print(l, strlen(l));
        unsigned int pc = m_document.percentLoaded();
        nds::Canvas::instance().fillRectangle(0,40, SCREEN_WIDTH, 20, nds::Color(31,31,31));
        nds::Canvas::instance().fillRectangle(0,40, pc*SCREEN_WIDTH / 100, 20, nds::Color(30,20,0));
        swiWaitForVBlank();
      }
      break;
    default:
      break;
  }
}

void View::tick()
{
  scanKeys();
  if (m_keyboard->visible()) {
    m_keyboard->handleInput();
    if (not m_keyboard->visible())
    {
      nds::Canvas::instance().fillRectangle(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, nds::Color(31,31,31));
      m_controller.doUri(m_keyboard->result());
    }
  } else {
    u16 keys = keysDownRepeat();
    if (keys & KEY_START) {
      m_keyboard->setVisible();
    }
    if (keys & KEY_DOWN) {
      // scroll down ...
      m_textArea->setStartLine(m_textArea->startLine()+1);
      render();
    }
    if (keys & KEY_UP) {
      // scroll up ...
      m_textArea->setStartLine(m_textArea->startLine()-1);
      render();
    }
    if (keys & KEY_A) {
      // render the node tree
      m_document.dumpDOM();
    }
    if (keys & KEY_TOUCH) {
      touchPosition tp = touchReadXY();
      Link * clicked = m_textArea->clickLink(tp.px, tp.py+SCREEN_HEIGHT);
      if (clicked != 0)
      {
        string s = clicked->href();
        string original = m_document.uri();
        URI tmpURI(original);
        tmpURI.navigateTo(s);
        m_textArea->setStartLine(0);
        cout << "Navigated to " << tmpURI.asString() << endl;
        m_controller.doUri( tmpURI.asString() );
      }
    }
  }
}
