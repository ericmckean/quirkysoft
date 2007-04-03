#include "ndspp.h"
#include "TextArea.h"
#include "Canvas.h"
#include "Font.h"
#include "UTF8.h"
#include "File.h"

using namespace nds;

TextArea::TextArea() : 
  m_font(0),
  m_encoding("utf-8")
{
  init("fonts/vera");
}

void TextArea::init(const std::string & fontBase)
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

void TextArea::print(const char * text, int amount, int x, int y)
{
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
