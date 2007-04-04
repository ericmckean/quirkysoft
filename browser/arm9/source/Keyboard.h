#ifndef Keyboard_h_seen
#define Keyboard_h_seen
class TextArea;
class Key;

namespace nds{
class Sprite;
}
#include <string>
#include <vector>

class Keyboard
{
  public:

    Keyboard(TextArea & textArea);
    ~Keyboard();

    void setVisible(bool visible=true);
    bool visible() const;
    void handleInput();
    std::string result() const;

  private:
    int m_shift;
    TextArea & m_textArea;
    std::vector<Key*> m_keys;
    nds::Sprite * m_cursor;
    bool m_visible;
    bool m_redraw;
    int m_ticks;
    std::string m_result;

    void initKeys();
    void initCursor();
    void handleButtons(unsigned short keys);
    void checkPress(int x, int y);

    void doPress(Key * key);
    void setShift(int shift);
    void checkRedraw();
    void undrawResult();
    void deleteLetter();
    void drawResult();
    void clearResult();
    std::string selected();
};
#endif
