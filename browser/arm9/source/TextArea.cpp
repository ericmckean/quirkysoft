#include "ndspp.h"
#include "TextArea.h"
#include "Canvas.h"
#include "Font.h"
#include "UTF8.h"
#include "File.h"

using namespace nds;
using namespace std;

TextArea::TextArea() : 
  m_font(0),
  m_encoding("utf-8"),
  m_startLine(0)
{
  init("fonts/vera");
}

void TextArea::setStartLine(int line)
{
  if (line < 0)
    line = 0;
  m_startLine = line;
}

int TextArea::startLine() const
{
  return m_startLine;
}

void TextArea::init(const string & fontBase)
{
  m_font=new Font(fontBase);
  setPalette(fontBase+".pal");
}

void TextArea::printAt(Font::Glyph & g, int xPosition, int yPosition)
{
  const unsigned char * data = g.data;
  int dataInc = (m_font->totalWidth() - g.width)/2;
  for (int y = 0; y < g.height; ++y)
  {
    for (int x = 0; x < g.width/2; ++x)
    {
      unsigned char pixelPair = *data++;
      int pix1 = ((pixelPair)&0xf);
      if (pix1)
        Canvas::instance().drawPixel(xPosition+(x*2), yPosition+y, m_palette[pix1]);
      int pix2 = ((pixelPair>>4)&0xf);
      if (pix2)
        Canvas::instance().drawPixel(xPosition+(x*2)+1, yPosition+y, m_palette[pix2]);
    }
    data += dataInc;
  }
}

void TextArea::setCursor(int x, int y)
{
  m_cursorx = x;
  m_cursory = x;
}

void TextArea::incrLine(int height)
{
  m_cursorx = 0; 
  m_cursory += height;
}

void TextArea::printu(const UnicodeString & unicodeString)
{
  // skip until we reach startLine
  int tmpx = m_cursorx;
  int tmpy = m_cursorx;
  m_cursorx = 0;
  m_cursory = 0;
  UnicodeString::const_iterator it(unicodeString.begin());
  int position(0);
  int lines(0);
  for (; it != unicodeString.end() and lines < m_startLine; ++it, ++position)
  {
    unsigned int value(*it);
    if (value == UTF8::MALFORMED) {
      value = '?';
    }
    Font::Glyph g;
    m_font->glyph(value, g);
    if (value == '\n')
    {
      incrLine(m_font->height());
      lines++;
    } 
    else {
      if (m_cursorx + g.width > Canvas::instance().width())
      {
        incrLine(g.height);
        lines++;
      }
      m_cursorx += g.width;
    }
    if (m_cursorx > Canvas::instance().width())
    {
      incrLine(m_font->height());
      lines++;
    }
  }
  m_cursorx = tmpx;
  m_cursory = tmpy;
  printuImpl(unicodeString.substr(position , unicodeString.length()-position));
}

void TextArea::printuImpl(const UnicodeString & unicodeString)
{
  int & x(m_cursorx);
  int & y(m_cursory);
  UnicodeString::const_iterator it(unicodeString.begin());
  for (; it != unicodeString.end(); ++it)
  {
    unsigned int value(*it);
    if (m_palette) {
      // cout << "Value " << (void*)value << "("<< (char)value << ")" << endl;
      if (value == UTF8::MALFORMED) {
        value = '?';
      }
      Font::Glyph g;
      m_font->glyph(value, g);
      if (value == '\n')
      {
        incrLine(g.height);
      } 
      else {
        if (x+g.width > Canvas::instance().width())
        {
          incrLine(g.height);
        }
        if (g.data) {
          printAt(g, x, y);
        }
        x += g.width;
      }
    } else {
      Font::Glyph g;
      m_font->glyph(' ', g);
      Canvas::instance().fillRectangle(x, y, 
          g.width, g.height, Color(31,0,0));
      x += g.width;
    }
    if (x > Canvas::instance().width())
    {
      incrLine(m_font->height());
    }
    if (y > Canvas::instance().height())
    {
      break;
    }
  }
}

void TextArea::print(const char * text, int amount)
{
  int x = m_cursorx;
  int y = m_cursory;
  int total = 0;
  while (total < amount)
  {
    unsigned int value;
    unsigned int read(1);
    if (m_encoding == "utf-8") {
      read = UTF8::decode(text, value);
    } else {
      value = (int)(text[0]&0xff);
    }
    if (m_palette) {
      if (value == UTF8::MALFORMED) {
        value = '?';
      }
      Font::Glyph g;
      m_font->glyph(value, g);
      if (value == '\n')
      {
        x = 0; 
        y += g.height;
      } else {
        if (g.data) {
          printAt(g, x, y);
        }
        x += g.width;
      }
    } else {
      Font::Glyph g;
      m_font->glyph(' ', g);
      Canvas::instance().fillRectangle(x, y, 
          g.width, g.height, Color(31,0,0));
      x += g.width;
    }
    if (x > Canvas::instance().width())
    {
      x = 0;
      y += m_font->height();
    }
    text += read;
    total += read;
  }
  m_cursorx = x;
  m_cursory = y;
}

void TextArea::setPalette(const std::string & fileName)
{
  File palFile;
  palFile.open(fileName.c_str());
  // read the lot
  if (palFile.is_open())
  {
    int size = palFile.size();
    char * data = new char[size+2];
    palFile.read(data);
    data[size] = 0;
    m_palette = (unsigned short*) data;
    Canvas & canvas( Canvas::instance());
    canvas.fillRectangle(0,0,canvas.width(), canvas.height(), m_palette[0]);
  } else {
    Canvas::instance().fillRectangle(130,0,10,128,Color(31,0,0));
  }
}

TextArea::~TextArea()
{
  delete m_font;
  delete [] m_palette;
}

void TextArea::setEncoding(const std::string & encoding)
{
  m_encoding = encoding;
}
