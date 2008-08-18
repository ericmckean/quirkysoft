/*
  Copyright (C) 2008 Richard Quirk

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

#include "BoxLayout.h"
#include "Delete.h"
#include <set>
#include <algorithm>
#include <functional>

using nds::Rectangle;

class BoxLayout::Box
{
  public:
    static int MAX_WIDTH;

    Box()
    {
      m_bounds.x = 0;
      m_bounds.y = 0;
      m_bounds.w = 0;
      m_bounds.h = 0;
    }

    ~Box()
    {
      for_each(m_children.begin(), m_children.end(), delete_ptr());
    }

    bool tryAdd(Component *child)
    {
      if ((child->width() + m_bounds.w) > MAX_WIDTH)
      {
        return false;
      }
      addPrivate(child);
      return true;
    }

    void setPosition(int x, int y)
    {
      m_bounds.x = x;
      m_bounds.y = y;
    }

    Rectangle bounds() const
    {
      return m_bounds;
    }

    bool hasChanged()
    {
      // see if any child components need redoing
      BoundComponentSet::iterator it(find_if(m_children.begin(), m_children.end(), std::mem_fun(&BoundComponent::hasChanged)));
      return it != m_children.end();
    }

    void move(int dx, int dy)
    {
      for (BoundComponentSet::iterator it(m_children.begin());
          it != m_children.end(); ++it)
      {
        BoundComponent *c(*it);
        c->move(dx, dy);
      }
      m_bounds.x -= dx;
      m_bounds.y -= dy;
    }

  private:
    Rectangle m_bounds;
    class BoundComponent
    {
      public:
        BoundComponent(Component *c):
          m_component(c),m_bounds(c->bounds()) {}

        bool hasChanged() const
        {
          return m_component->bounds() != m_bounds;
        }

        void move(int dx, int dy)
        {
          m_bounds.x -= dx;
          m_bounds.y -= dy;
        }

        inline Component *component() const
        {
          return m_component;
        }

      private:
        Component *m_component;
        Rectangle m_bounds;
    };
    typedef std::set<BoundComponent*> BoundComponentSet;
    BoundComponentSet m_children;

    bool initialised() const
    {
      return m_bounds.h != 0;
    }

    // on adding a child, this recalculates the width and the height
    // that the box uses and stores it in m_bounds
    // width is total width of all components
    // height is height of tallest child component
    void recalcSize()
    {
      int w(0);
      int h(-1);
      for (BoundComponentSet::iterator it(m_children.begin());
          it != m_children.end();
          ++it) {
        BoundComponent *child(*it);
        w += child->component()->width();
        if (child->component()->height() > h)
          h = child->component()->height();
      }
      m_bounds.w = w;
      m_bounds.h = h;
    }

    void addPrivate(Component *child)
    {
      child->setLocation(m_bounds.right(), m_bounds.top());
      m_children.insert(new BoundComponent(child));
      recalcSize();
    }
};

int BoxLayout::Box::MAX_WIDTH(255);

BoxLayout::BoxLayout()
{
  initBoxes();
}

void BoxLayout::initBoxes()
{
  Box *first(new Box);
  first->setPosition(0,0);
  m_boxes.push_front(first);
}

BoxLayout::~BoxLayout()
{
  for_each(m_boxes.begin(), m_boxes.end(), delete_ptr());
}

void BoxLayout::addToLayout(Component *child)
{
  // mostly copied from RichTextArea...
  // now see *where* this should go in box terms
  // oh! idea: add a "setSizeFixed()" method to prevent RichTextArea and
  // friends from resizing after being fixed into position?
  Box *lastBox(m_boxes.front());
  if (not lastBox->tryAdd(child))
  {
    Rectangle lbb(lastBox->bounds());
    lastBox = new Box;
    lastBox->setPosition(lbb.x, lbb.bottom());
    lastBox->tryAdd(child);
    m_boxes.push_front(lastBox);
  }
}

void BoxLayout::add(Component *child)
{
  Component::add(child);
  // Fudge size vs preferredSize
  const Rectangle &bounds(child->preferredSize());
  int w(bounds.w);
  if (m_bounds.w < w) {
    w = m_bounds.w;
  }
  child->setSize(w, bounds.h);
  addToLayout(child);
}

void BoxLayout::paint(const nds::Rectangle & clip)
{
  using nds::Rectangle;
  // paint all children.
  for (std::vector<Component*>::iterator it(m_children.begin());
      it != m_children.end();
      ++it)
  {
    Component * c(*it);
    Rectangle bounds(c->bounds());
    Rectangle thisClip(clip.intersect(bounds));
    if (thisClip.w == 0 and thisClip.h == 0)
      continue;
    c->paint(thisClip);
    // nds::Canvas::instance().setClip(clip);
  }
}

void BoxLayout::setLocation(int x, int y)
{
  // work out dx, dy
  int dx = this->x() - x;
  int dy = this->y() - y;
  Component::setLocation(x, y);
  if (dx == 0 and dy == 0)
    return;

  for (std::vector<Component*>::iterator it(m_children.begin());
      it != m_children.end();
      ++it)
  {
    Component * child(*it);
    child->setLocation( child->x() - dx, child->y() - dy);
  }
  for (std::list<Box*>::iterator it(m_boxes.begin());
      it != m_boxes.end();
      ++it)
  {
    Box *box(*it);
    box->move(dx, dy);
  }
}

unsigned int BoxLayout::boxCount() const
{
  return m_boxes.size();
}

void BoxLayout::doLayout()
{
  // redo all boxes layout
  std::list<Box*>::iterator it = find_if(
      m_boxes.begin(), m_boxes.end(),
      std::mem_fun(&Box::hasChanged));
  if (it != m_boxes.end())
  {
    // needs redoing
    for_each(m_boxes.begin(), m_boxes.end(), delete_ptr());
    m_boxes.clear();
    initBoxes();
    for_each(m_children.begin(), m_children.end(), std::bind1st(std::mem_fun(&BoxLayout::addToLayout), this));
  }
}

