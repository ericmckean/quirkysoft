#ifndef View_h_seen
#define View_h_seen

#include "ViewI.h"

class Document;
class ControllerI;

class TextArea;

class View : public ViewI
{
  public:
    View(Document &, ControllerI &);

    void notify();

    /** Show the view, handle key events, etc.
     */
    void mainLoop();

  private:
    Document & m_document;
    ControllerI & m_controller;
    TextArea * m_textArea;

};
#endif
