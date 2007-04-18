#include <libgen.h>
#include <iostream>
#include "ndspp.h"
#include "libnds.h"
#include "View.h"
#include "Document.h"
#include "URI.h"
#include "TextArea.h"
#include "Canvas.h"
#include "ControllerI.h"
#include "Keyboard.h"
#include "Link.h"
#include "ViewRender.h"

using namespace std;

View::View(Document & doc, ControllerI & c):
  m_document(doc), 
  m_controller(c),
  m_textArea(new TextArea),
  m_keyboard(new Keyboard(*m_textArea)),
  m_renderer(new ViewRender(this))
{
  m_document.registerView(this);
  keysSetRepeat( 10, 5 );
}

void View::notify()
{
  Document::Status status(m_document.status());

  switch (status) {
    case Document::LOADED:
      {
        m_textArea->setStartLine( (-SCREEN_HEIGHT / m_textArea->font().height()) - 1);
        m_renderer->render();
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
      if (not m_keyboard->result().empty())
      {
        m_controller.doUri(m_keyboard->result());
      }
      else
      {
        m_renderer->render();
      }
    }
  } else {
    u16 keys = keysDownRepeat();
    if (keys & KEY_START) {
      nds::Canvas::instance().fillRectangle(0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, nds::Color(31,31,31));
      m_keyboard->setVisible();
    }
    if (keys & KEY_DOWN) {
      // scroll down ...
      m_textArea->setStartLine(m_textArea->startLine()+10);
      m_renderer->render();
    }
    if (keys & KEY_UP) {
      // scroll up ...
      m_textArea->setStartLine(m_textArea->startLine()-10);
      m_renderer->render();
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
