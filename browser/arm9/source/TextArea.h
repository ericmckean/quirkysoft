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

    void setColor(unsigned short color);
    void setStartLine(int line);
    int startLine() const;

  private:
    Font * m_font;
    unsigned short * m_palette;
    unsigned short * m_basePalette;
    int m_paletteLength;
    std::string m_encoding;
    int m_startLine;

    int m_cursorx;
    int m_cursory;
    void printAt(Font::Glyph & g, int xPosition, int yPosition);
    void incrLine();
    void checkLetter(Font::Glyph & g);
    void printuImpl(const UnicodeString & unicodeString);
    bool doSingleChar(unsigned int value);
    int textSize(const UnicodeString & unicodeString) const;
    void printuWord(const UnicodeString & word);

};
#endif
