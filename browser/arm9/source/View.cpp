#include "ndspp.h"
#include "libnds.h"
#include <iostream>
#include "View.h"
#include "Document.h"


View::View(Document & doc, ControllerI & c):m_document(doc), m_controller(c)
{
  m_document.registerView(this);
}

void View::notify()
{
  std::cout << "URI: " << m_document.uri() << std::endl;
  //std::cout << "Content: " << m_document.asText() << std::endl;
}

void View::mainLoop()
{
  for(;;) {
    scanKeys();
    swiWaitForVBlank();
  }
}
