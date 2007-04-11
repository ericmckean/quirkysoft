#include <list>
#include "ndspp.h"
#include "libnds.h"
#include "View.h"
#include "Document.h"
#include "HtmlElement.h"
#include "TextArea.h"
#include "Canvas.h"
#include "ControllerI.h"
#include "Keyboard.h"

using namespace std;

View::View(Document & doc, ControllerI & c):m_document(doc), m_controller(c)
{
  m_document.registerView(this);
  m_textArea = new TextArea(/*"fonts/vera"*/);
}

void View::walkNode(const HtmlElement * node)
{
  if (node->hasChildren())
  {
    const ElementList & theChildren = node->children();
    ElementList::const_iterator it(theChildren.begin());
    for (; it != theChildren.end(); ++it)
    {
      const HtmlElement * element(*it);
      if (element->isa("a"))
      {
        m_textArea->setColor(nds::Color(0,0,31));
      } 
      if (element->text().size())
      {
        m_textArea->printu(element->text());
        m_textArea->setColor(nds::Color(0,0,0));
      }
      if (element->isa("p"))
      {
        m_textArea->setParseNewline(true);
        m_textArea->print("\n",1);
        m_textArea->setParseNewline(false);
      }
      walkNode(element);
    }
  }
}

void View::render()
{
  nds::Canvas::instance().fillRectangle(0, 0, SCREEN_WIDTH, 2*SCREEN_HEIGHT, nds::Color(31,31,31));
  m_textArea->setCursor(0, 0);
  m_textArea->setParseNewline(false);
  const HtmlElement * root = m_document.rootNode();
  walkNode(root);
}

void View::notify()
{
  //std::cout << "URI: " << m_document.uri() << std::endl;
  //std::cout << "Content: " << m_document.asText() << std::endl;
  Document::Status status(m_document.status());

  switch (status) {
    case Document::LOADED:
      {
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
        m_textArea->setStartLine(m_textArea->startLine()+10);
        notify();
      }
      if (keys & KEY_UP) {
        // scroll up ...
        m_textArea->setStartLine(m_textArea->startLine()-10);
        notify();
      }

    }
    swiWaitForVBlank();
  }
}
