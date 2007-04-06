#ifndef TextArea_h_seen
#define TextArea_h_seen

#include <string>

#include "Font.h"

class TextArea
{
  public:
    TextArea();
    ~TextArea();

    void init(const std::string & fontBase);
    void setPalette(const std::string & fileName);
    void print(const char * data, int amount, int x, int y);
    void printu(const std::basic_string<unsigned int> & unicodeString, int x, int y);
    void printAt(Font::Glyph & g, int xPosition, int yPosition);

    void setEncoding(const std::string & encoding="utf-8");
    void setCursor(int x, int y);

    inline const Font & font() const {
      return *m_font;
    }

  private:
    Font * m_font;
    unsigned short * m_palette;
    std::string m_encoding;
    int m_cursorx;
    int m_cursory;
};
#endif
