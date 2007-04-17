#ifndef ViewRender_h_seen
#define ViewRender_h_seen

class View;
class TextArea;
class HtmlElement;

class ViewRender
{
  public:
    ViewRender(View * self);

    void render();

  private:
    View * m_self;
    int m_pendingNewLines;
    bool m_haveShownSomething;

    void walkNode(const HtmlElement * node);
    bool applyFormatting(const HtmlElement * element);
    void preNodeFormatting(const HtmlElement * node);
    void postNodeFormatting(const HtmlElement * node);
    //! Really add a newline or few to the text area
    void addRealNewline(int count=1);
    void flushNewlines();

    void setBgColor(const HtmlElement * body);
};
#endif
