#ifndef View_h_seen
#define View_h_seen

#include "ViewI.h"

class Document;
class ControllerI;

class TextArea;
class Keyboard;
class ViewRender;

/** Handle the displaying of HTML data.*/
class View : public ViewI
{
  friend class ViewRender;
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
    ViewRender * m_renderer;


};
#endif
