#include <list>
#include <iostream>
#include <wchar.h>
#include "ndspp.h"
#include "libnds.h"
#include "View.h"
#include "Document.h"
#include "URI.h"
#include "HtmlElement.h"
#include "TextArea.h"
#include "Canvas.h"
#include "ControllerI.h"
#include "Keyboard.h"
#include "Link.h"

using namespace std;

View::View(Document & doc, ControllerI & c):m_document(doc), m_controller(c)
{
  m_document.registerView(this);
  m_textArea = new TextArea(/*"fonts/vera"*/);
}

void View::addRealNewline(int count)
{
  m_textArea->setParseNewline(true);
  UnicodeString newlines(count, '\n');
  m_textArea->printu(newlines);
  m_textArea->setParseNewline(false);
}

void View::walkNode(const HtmlElement * node)
{
  if (node->isa("li"))
  {
    UnicodeString p;
    p += ' ';
    p += '*';
    p += ' ';
    m_textArea->printu(p);
  }
  if (node->isa("ul"))
  {
    addRealNewline(1);
  }
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
  if (node->isa("li"))
  {
    m_textArea->setParseNewline(true);
    UnicodeString p;
    p += '\n';
    m_textArea->printu(p);
    m_textArea->setParseNewline(false);
  }
  if (node->isa("a"))
  {
    m_textArea->setColor(nds::Color(0,0,0));
    m_textArea->setLink(false);
  }
}

void View::render()
{
  nds::Canvas::instance().fillRectangle(0, 0, SCREEN_WIDTH, 2*SCREEN_HEIGHT, nds::Color(31,31,31));
  m_textArea->setCursor(0, 0);
  m_textArea->setParseNewline(false);
  const HtmlElement * root = m_document.rootNode();
  const HtmlElement * head = root->firstChild();
  const HtmlElement * body = root->lastChild();
  walkNode(body);
}

bool View::applyFormatting(const HtmlElement * element)
{
  if (element->isa("a"))
  {
    m_textArea->setColor(nds::Color(0,0,31));
    m_textArea->addLink(element);
  } 
  if (element->text().size())
  {
    m_textArea->printu(element->text());
  }
  if (element->isa("p"))
  {
    addRealNewline(2);
  }
  if (element->isa("br"))
  {
    addRealNewline();
  }
  if (element->isa("script") or element->isa("style"))
  {
    return false;
  }
  return true;
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

void View::mainLoop()
{
  Keyboard * keyboard = new Keyboard(*m_textArea);
  for(;;) {
    scanKeys();
    if (keyboard->visible()) {
      keyboard->handleInput();
      if (not keyboard->visible())
      {
        nds::Canvas::instance().fillRectangle(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, nds::Color(31,31,31));
        m_controller.doUri(keyboard->result());
      }
    } else {
      u16 keys = keysDownRepeat();
      if (keys & KEY_START) {
        keyboard->setVisible();
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
      if (keys & KEY_TOUCH) {
        touchPosition tp = touchReadXY();
        Link * clicked = m_textArea->clickLink(tp.px, tp.py+SCREEN_HEIGHT);
        if (clicked != 0)
        {
          string s = clicked->href();
          string original = m_document.uri();
          URI tmpURI(original);
          tmpURI.navigateTo(s);
          cout << "Do uri! " << tmpURI.asString() << endl;
          m_textArea->setStartLine(0);
          m_controller.doUri( tmpURI.asString() );
        }
      }
    }
    swiWaitForVBlank();
  }
}
