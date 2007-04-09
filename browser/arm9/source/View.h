#ifndef View_h_seen
#define View_h_seen

#include "ViewI.h"

class Document;
class ControllerI;

class TextArea;
class HtmlElement;

/** Handle the displaying of HTML data.*/
class View : public ViewI
{
  public:
    /** Create a View for the given document and controller.
     * @param doc the model.
     * @param controller the controller.
     */
    View(Document & doc, ControllerI & controller);

    void notify();

    /** Show the view, handle key events, etc. Never returns.
     */
    void mainLoop();

  private:
    Document & m_document;
    ControllerI & m_controller;
    TextArea * m_textArea;

    void render();
    void walkNode(const HtmlElement * node);

};
#endif
