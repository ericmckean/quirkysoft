#ifndef TextArea_h_seen
#define TextArea_h_seen

#include "Font.h"
#include "UnicodeString.h"

class TextArea
{
  public:
    TextArea();
    ~TextArea();

    void init(const std::string & fontBase);
    void setPalette(const std::string & fileName);

    void setCursor(int x, int y);
    void print(const char * data, int amount);
    void printu(const UnicodeString & unicodeString);

    inline const Font & font() const {
      return *m_font;
    }

    void setEncoding(const std::string & encoding="utf-8");

    void setStartLine(int line);
    int startLine() const;

  private:
    Font * m_font;
    unsigned short * m_palette;
    std::string m_encoding;
    int m_startLine;

    int m_cursorx;
    int m_cursory;
    void printAt(Font::Glyph & g, int xPosition, int yPosition);
    void incrLine(int height);

    void printuImpl(const UnicodeString & unicodeString);

};
#endif
