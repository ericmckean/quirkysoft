#ifndef View_h_seen
#define View_h_seen

#include "ViewI.h"

class Document;
class ControllerI;

class TextArea;
class Keyboard;
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

    /** Show the view, handle key events for one frame.
     */
    void tick();

  private:
    Document & m_document;
    ControllerI & m_controller;
    TextArea * m_textArea;
    Keyboard * m_keyboard;

    void render();
    void walkNode(const HtmlElement * node);
    bool applyFormatting(const HtmlElement * element);
    //! Really add a newline or few to the text area
    void addRealNewline(int count=1);

};
#endif
