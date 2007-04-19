#include "Canvas.h"
#include "HtmlElement.h"
#include "ControllerI.h"
#include "Palette.h"
#include "FormControl.h"
#include "Rectangle.h"
#include "TextArea.h"

using nds::Canvas;
using nds::Color;
const int FormControl::MAX_SIZE(120);
const int FormControl::MIN_SIZE(8);

FormControl::FormControl(HtmlElement * element)
  : m_size(new Rectangle), m_element(element)
{
  m_size->x = 0;
  m_size->y = 0;
  m_size->w = 1;
  m_size->h = 1;
}

FormControl::~FormControl()
{
  delete m_size;
}

void FormControl::setPosition(int x, int y)
{
  m_size->x = x;
  m_size->y = y;
}

void FormControl::setHeight(int h)
{
  m_size->h = h;
}
int FormControl::width() const
{
  return m_size->w+MIN_SIZE;
}

bool FormControl::hitTest(int x, int y) const
{
  return m_size->hit(x,y);
}

void FormControl::draw(TextArea * gfx)
{
  Canvas::instance().drawRectangle(m_size->x, m_size->y, m_size->w, m_size->h, Color(15,25,20));
}

FormControl::InputType FormControl::inputType() const
{
  return ONE_CLICK;
}

void FormControl::input(const UnicodeString & str)
{
}

void FormControl::input(int x, int y, ControllerI & controller)
{
  // need to walk up m_element until we find the form father or the html element.
  HtmlElement * currentNode = m_element->parent();
  while (not currentNode->isa("html"))
  {
    if (currentNode->isa("form"))
      break;
    currentNode = currentNode->parent();
  }
  if (currentNode->isa("html"))
  {
    return;
  }
  // else select the "successful controls" and post them
  //controller.doUri();
}
