#include "Canvas.h"
#include "Palette.h"
#include "InputText.h"
#include "Rectangle.h"
#include "TextArea.h"
#include "HtmlElement.h"

using nds::Canvas;
using nds::Color;
using std::string;

InputText::InputText(HtmlElement * element, const TextArea * textArea)
  : FormControl(element)
{
  string sizeText = unicode2string(m_element->attribute("size"));
  int size(0);
  if (not sizeText.empty())
  {
    size = strtol(sizeText.c_str(), 0, 0);
    size *= textArea->font().height();
  }
  if (size <= 0)
    size = MIN_SIZE;
  if (size > MAX_SIZE)
    size = MAX_SIZE;
  m_size->w = size;
}

void InputText::draw(TextArea * textArea)
{
  FormControl::draw(textArea);
  // print the button
  textArea->printu(m_element->attribute("value"));
}

FormControl::InputType InputText::inputType() const
{
  return KEYBOARD;
}

void InputText::input(const UnicodeString & str)
{
  m_element->setAttribute("value", str);
}
