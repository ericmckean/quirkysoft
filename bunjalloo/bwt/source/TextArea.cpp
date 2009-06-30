/*
  Copyright (C) 2007,2008 Richard Quirk

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <assert.h>
#include "libnds.h"
#include "ndspp.h"
#include "File.h"
#include "Font.h"
#include "string_utils.h"
#include "Canvas.h"
#include "Link.h"
#include "Palette.h"
#include "TextArea.h"
#include "utf8.h"

using namespace nds;
using namespace std;
const static nds::Color EDGE(20,20,20);
const static nds::Color SHADOW(28,28,28);
const static unsigned char NEWLINE('\n');

static const char intDelimiters[] = {0x20, 0x09, 0x0a, 0x0b, 0x0c, 0x0d};
static const std::string s_delimiters(intDelimiters,6);
static const int INDENT(16);

TextArea::TextArea(Font * font) :
  m_appendPosition(0),
  m_font(0),
  m_palette(0),
  m_basePalette(0),
  m_paletteLength(0),
  m_parseNewline(true),
  m_bgCol(0),
  m_fgCol(0),
  m_underLine(false)
{
  setFont(font);
  m_document.clear();
  m_preferredHeight = m_font->height();
  m_preferredWidth = Canvas::instance().width();
  m_bounds.w = Canvas::instance().width();
}


void TextArea::setFont(Font * font)
{
  m_font = font;
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
  if (m_underLine)
  {
    // draw underline
    Canvas::instance().horizontalLine(xPosition,
                                      yPosition+m_font->height()-1,
                                      g.width,
                                      m_palette[m_paletteLength-1]);

  }
}

void TextArea::document(std::string & returnString) const
{
  std::vector<std::string>::const_iterator it(m_document.begin());
  for (; it != m_document.end(); ++it)
  {
    returnString.append(*it);
  }
}

void TextArea::clearText()
{
  // Could do the following to completely free the memory, but there
  // is a fair chance it will be reused anyway... speed vs memory again :-/
  std::vector<std::string> tmp;
  m_document.swap(tmp);
  m_appendPosition = 0;
  m_preferredWidth = -1;
  m_preferredHeight = m_font->height();
  currentLine();
}

static std::string shorterWordFromLong(
    std::string::const_iterator *it,
    const std::string::const_iterator &end_it,
    int width,
    Font *font,
    int *size)
{
  // This is a very long word, split it up
  std::string shorterWord;
  *size = 0;
  while (*it != end_it)
  {
    // store the start of the unicode code point
    std::string::const_iterator a(*it);
    uint32_t value = utf8::next(*it, end_it);
    if (value == 0xfffd)
      value = '?';
    Font::Glyph g;
    font->glyph(value, g);
    if ( (*size + g.width) >= width) {
      // rewind to the start of that last code point
      *it = a;
      return shorterWord;
    }
    for (; a != *it; ++a) {
      shorterWord += *a;
    }
    *size += g.width;
  }
  return shorterWord;
}

void TextArea::appendText(const std::string &unicodeString)
{
  if (m_document.empty())
  {
    m_preferredWidth = 0;
  }
  // append text, adding in new lines as needed to wrap.
  // int currPosition = 0;
  // find the next space character
  std::string::const_iterator it = unicodeString.begin();
  std::string::const_iterator end_it = unicodeString.end();
  while (it != end_it)
  {

    std::string::const_iterator backup_it(it);
    std::string word(nextWordAdvanceWord(&it, end_it, m_parseNewline));
    int size = textSize(word);
    if (size > width() and word.size() > 1)
    {
      it = backup_it;
      word = shorterWordFromLong(&it, end_it, width(), m_font, &size);
    }

    // if the word ends with a new line, then increment the height.
    // otherwise, if we go off the end of the line, increment the height.
    if ((m_appendPosition + size) > width())
    {
      // trim spaces from the end of the line
      // this word overflows the line - make a new line to hold the text.
      m_document.push_back(std::string());
      m_appendPosition = 0;
      m_preferredHeight += m_font->height();
    }
    if (m_preferredWidth < 0)
      m_preferredWidth = 0;
    m_preferredWidth += size;
    if (not m_parseNewline and word[word.length()-1] == NEWLINE) {
      word[word.length() - 1] = ' ';
    }
    currentLine().append(word);
    m_appendPosition += size;
    // if the word ended in a NEWLINE, then go onto the next line.
    if (m_parseNewline and word[word.length()-1] == NEWLINE)
    {
      m_appendPosition = 0;
      m_document.push_back(std::string());
      m_preferredHeight += font().height();
    }
  }
}

void TextArea::layoutText()
{
  // need to shuffle the document about... this requires a new copy of it.
  std::string tmp;
  bool pnl = parseNewline();
  setParseNewline();
  document(tmp);
  clearText();
  if (not tmp.empty())
    appendText(tmp);
  if (m_preferredHeight == 0)
    m_preferredHeight = m_font->height();
  setParseNewline(pnl);
}

void TextArea::setSize(unsigned int w, unsigned int h)
{
  if (m_bounds.w != (int)w) {
    Component::setSize(w, h);
    layoutText();
  } else {
    Component::setSize(w, h);
  }
}

void TextArea::setCursor(int x, int y)
{
  m_cursorx = x;
  m_cursory = y;
  m_initialCursorx = x;
}

void TextArea::incrLine()
{
  m_cursorx = m_initialCursorx;
  m_cursory += m_font->height();
}

void TextArea::checkLetter(Font::Glyph & g)
{
  if ( (m_cursorx + g.width) > m_bounds.right())
  {
    incrLine();
  }
}

void TextArea::printu(const std::string & unicodeString)
{
  std::string::const_iterator it(unicodeString.begin());
  for (; it != unicodeString.end() and m_cursory < m_bounds.bottom(); ++it)
  {
    unsigned int value(*it);
    if (doSingleChar(value))
    {
      break;
    }
  }
}

int TextArea::textSize(const std::string &unicodeString) const
{
  std::string::const_iterator it(unicodeString.begin());
  std::string::const_iterator end_it(unicodeString.end());
  int size(0);
  while (it != end_it)
  {
    uint32_t value = utf8::next(it, end_it);
    if (value != NEWLINE)
    {
      if (value == 0xfffd)
        value = '?';
      Font::Glyph g;
      m_font->glyph(value, g);
      size += g.width;
    }
  }
  return size;
}

bool TextArea::doSingleChar(unsigned int value)
{
  if (value == 0xfffd) {
    value = '?';
  }
  Font::Glyph g;
  m_font->glyph(value, g);
  if (value != NEWLINE) {
    checkLetter(g);
    if (g.data) {
      printAt(g, m_cursorx, m_cursory);
    }
    m_cursorx += g.width;
  }
  // else ignore new line character. New lines are parsed at entry time
  return (m_cursory > m_bounds.bottom());
}

void TextArea::setDefaultColor()
{
  m_bgCol = m_basePalette[0];
  m_fgCol = 0;
  setTextColor(m_fgCol);
}

void TextArea::setBackgroundColor(unsigned short color)
{
  if (color != m_bgCol)
  {
    m_bgCol = color;
    setTextColor(m_fgCol);
  }
}

unsigned short TextArea::backgroundColor() const
{
  return m_bgCol;
}

void TextArea::setTextColor(unsigned short color)
{
  m_fgCol = color;
  Color newColor(color);
  Color bgCol(m_bgCol);
  // assuming m_basePalette is black and white...
  int mred = bgCol.red() - newColor.red();
  int mgreen = bgCol.green() - newColor.green();
  int mblue = bgCol.blue() - newColor.blue();

  for (int i = 0; i < m_paletteLength; ++i)
  {
    // convert from grey scale to colour scale.
    Color c(m_basePalette[i]);

    // y |       ____   y = mx+c
    //  c|___----
    //   |___________
    //                x
    //  c = initial colour      (intercept)
    //  m = 31 - initial  / 32  (gradient)
    //  x = grey value

    int yred   = (mred * c.red())/32 + newColor.red();
    int ygreen = (mgreen * c.green())/32 + newColor.green();
    int yblue  = (mblue * c.blue())/32 + newColor.blue();
    c.red(yred);
    c.green(ygreen);
    c.blue(yblue);

    m_palette[i] = c;

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
    char * baseData = new char[size+2];
    copy(data, data+size, baseData);
    m_basePalette = (unsigned short*)baseData;
    m_paletteLength = size/2;
    setBackgroundColor(m_palette[0]);
  } else {
    /* If the font is not opened, then set the background red */
    Canvas::instance().fillRectangle(0,0,256,192,Color(31,0,0));
    setBackgroundColor(Color(31,0,0));
  }
}

void TextArea::setPalette(const char * data, unsigned int size)
{
  m_palette = new unsigned short[size/2];
  copy(data, data+size, (char*)m_palette);
  char * baseData = new char[size+2];
  copy(data, data+size, baseData);
  m_basePalette = (unsigned short*)baseData;
  m_paletteLength = size/2;
  setBackgroundColor(m_palette[0]);
}

TextArea::~TextArea()
{
  //delete m_font;
  delete [] m_basePalette;
  delete [] m_palette;
}

int TextArea::linesToSkip() const
{
  return - ( (m_bounds.y-(m_font->height()/2))/m_font->height() );
}

/** Paint the text area. */
void TextArea::paint(const nds::Rectangle & clip)
{
  // TextArea wants to paint from 0,0 to 256,END_Y
  // ScrollView says "start at position X" instead... (the clip square)
  // TextArea then thinks "position X, that is really line N"
  // and draws there until it runs out of space.
  //
  // Theres "width" which is the possible width (wraps at)
  // then theres clip-width which is where it should draw to.
  setCursor(m_bounds.x, m_bounds.y);
  Canvas::instance().fillRectangle(clip.x, clip.y, clip.w, clip.h, m_bgCol);
  // work out the number of lines to skip
  std::vector<std::string>::const_iterator it(m_document.begin());
  int skipLines = linesToSkip();
  if (skipLines > 0)
  {
    setCursor(m_bounds.x, m_bounds.y + skipLines*m_font->height());
    it += skipLines;
    if (skipLines >= (int)m_document.size())
    {
      return;
    }
  }

  for (; it != m_document.end() and (m_cursory < m_bounds.bottom()) and (m_cursory < clip.bottom()); ++it)
  {
    printu(*it);
    incrLine();
  }
}

std::string TextArea::asString() const
{
  std::string returnString;
  bool needComma(false);
  std::vector<std::string>::const_iterator it(m_document.begin());
  for (; it != m_document.end(); ++it)
  {
    if (needComma)
      returnString += ",\n";
    returnString += "[\"";
    returnString.append(*it);
    returnString += "\"]";
    needComma = true;
  }

  return returnString;
}

