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

#include <cppunit/extensions/HelperMacros.h>

class BoxLayoutTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( BoxLayoutTest );
  CPPUNIT_TEST( testSimple );
  CPPUNIT_TEST( testPacked );
  CPPUNIT_TEST( testPacked2 );
  CPPUNIT_TEST( testPacked3 );
  CPPUNIT_TEST( testResize );
  CPPUNIT_TEST( testSetLocation );
  CPPUNIT_TEST_SUITE_END();

public:
  BoxLayout *m_layout;
  void setUp();
  void tearDown();

  void testSimple();
  void testPacked();
  void testPacked2();
  void testPacked3();
  void testResize();
  void testSetLocation();

};

class MockComponent: public Component
{
  public:
    virtual void paint(const nds::Rectangle & clip) { }

    virtual bool stylusUp(const Stylus * stylus) { return false; }
    virtual bool stylusDownFirst(const Stylus * stylus) { return false; }
    virtual bool stylusDownRepeat(const Stylus * stylus) { return false; }
    virtual bool stylusDown(const Stylus * stylus) { return false; }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( BoxLayoutTest );

void BoxLayoutTest::setUp()
{
  m_layout = new BoxLayout;
  m_layout->setSize(255, 192*2);
  m_layout->setLocation(0, 0);
}

void BoxLayoutTest::tearDown()
{
  delete m_layout;
}

void BoxLayoutTest::testSimple()
{
  /*
   * Test that the simple case works:
   *
   * comp1 - wide component with lowish height
   * comp2 - wide component with lowish height
   *   +----------+
   *   |  comp1   |
   *   +----------+
   *   |  comp2   |
   *   +----------+
   * comp1 should be at 0,0
   * comp2 should be at 0,comp1.bottom
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 250, 14 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(r1.w, r1.h);

  m_layout->add(comp1);
  m_layout->add(comp2);

  Rectangle r2 = r1;
  r2.y = r1.bottom();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(2U, m_layout->boxCount());
}

void BoxLayoutTest::testPacked()
{
  /*
   * Test that packing 2 in one box works
   *
   * comp1 - small component with lowish height
   * comp2 - small component with lowish height
   *   +-------------+
   *   |comp1 | comp2|
   *   +-------------+
   * comp1 should be at 0,0
   * comp2 should be at comp1.right,0
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 60, 14 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(r1.w, r1.h);

  m_layout->add(comp1);
  m_layout->add(comp2);

  Rectangle r2 = r1;
  r2.x = r1.right();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(1U, m_layout->boxCount());
}

void BoxLayoutTest::testPacked2()
{
  /*
   * Test that packing 3 in one box, with odd heights, works
   *
   * comp1 - small component with lowish height
   * comp2 - small component with taller height
   * comp3 - small component with lowish height
   *   +------------------+
   *   |comp1 |comp2|comp3|
   *   +------{comp2}-----+
   *          +-----+
   * comp1 should be at 0,0
   * comp2 should be at comp1.right,0
   * comp3 should be at comp2.right,0
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;
  MockComponent *comp3 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 60, 14 };
  Rectangle r2 = { 0, 0, 60, 40 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(r2.w, r2.h);
  comp3->setSize(r1.w, r1.h);

  m_layout->add(comp1);
  m_layout->add(comp2);
  m_layout->add(comp3);

  // r1 is as expected
  // r2 should be at r1.right
  r2.x = r1.right();
  // r3 is same as r1, but to the right of r2
  Rectangle r3 = r1;
  r3.x = r2.right();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r3 == comp3->bounds());
  CPPUNIT_ASSERT_EQUAL(1U, m_layout->boxCount());
}

void BoxLayoutTest::testPacked3()
{
  /*
   * Test that packing 3 in one box, with odd heights,
   * then packing one more large comp all works
   *
   * comp1 - small component with lowish height
   * comp2 - small component with taller height
   * comp3 - small component with lowish height
   * comp4 - large component with med height
   *
   *   +------------------+
   *   |comp1 |comp2|comp3|
   *   +------{comp2}-----+
   *   +------+-----+-----+
   *   |    comp4         |
   *   +-------------------
   * comp1 should be at 0,0
   * comp2 should be at comp1.right,0
   * comp3 should be at comp2.right,0
   * comp4 should be at 0,comp2.height
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;
  MockComponent *comp3 = new MockComponent;
  MockComponent *comp4 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 60, 14 };
  Rectangle r2 = { 0, 0, 60, 40 };
  Rectangle larger = { 0, 0, 230, 20 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(r2.w, r2.h);
  comp3->setSize(r1.w, r1.h);
  comp4->setSize(larger.w, larger.h);

  m_layout->add(comp1);
  m_layout->add(comp2);
  m_layout->add(comp3);
  m_layout->add(comp4);

  // r1 is as expected
  // r2 should be at r1.right
  r2.x = r1.right();
  // r3 is same as r1, but to the right of r2
  Rectangle r3 = r1;
  r3.x = r2.right();
  // r4 is same as larger, but at r2.bottom
  Rectangle r4 = larger;
  r4.y = r2.bottom();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 3 bounds", r3 == comp3->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 4 bounds", r4 == comp4->bounds());
  CPPUNIT_ASSERT_EQUAL(2U, m_layout->boxCount());
}

void BoxLayoutTest::testResize()
{
  /*
   * Test that the simple resizing case works:
   *
   * comp1 - wide component with lowish height
   * comp2 - small component initially, then resized
   *   +----------+--+
   *   |  comp1   |c2|
   *   +----------+--+
   * After resize:
   *   +----------+
   *   |  comp1   |
   *   +----------+
   *   |  comp2   |
   *   +----------+
   * comp1 should be at 0,0
   * comp2 should be at 0,comp1.bottom
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 250, 14 };
  Rectangle small = { 0, 0, 0, 0 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(small.w, small.h);

  m_layout->add(comp1);
  m_layout->add(comp2);

  small.x = r1.right();
  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds pre-resize", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds pre-resize", small == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(1U, m_layout->boxCount());

  // now resize..
  comp2->setSize(r1.w, r1.h);

  m_layout->doLayout();

  Rectangle r2 = r1;
  r2.y = r1.bottom();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(2U, m_layout->boxCount());
}

void BoxLayoutTest::testSetLocation()
{
  /*
   * Test that setting the location of the BoxLayout moves all the children but
   * doesn't alter the box count
   *
   * comp1 - wide component with lowish height
   * comp2 - wide component with lowish height
   *   +----------+
   *   |  comp1   |
   *   +----------+
   *   |  comp2   |
   *   +----------+
   * comp1 should be at 0,0
   * comp2 should be at 0,comp1.bottom
   *
   * Then setLocation(0,-10):
   * comp1 should be at 0,-10
   * comp2 should be at 0,comp1.bottom
   */
  MockComponent *comp1 = new MockComponent;
  MockComponent *comp2 = new MockComponent;

  using nds::Rectangle;
  Rectangle r1 = { 0, 0, 250, 14 };
  comp1->setSize(r1.w, r1.h);
  comp2->setSize(r1.w, r1.h);

  m_layout->add(comp1);
  m_layout->add(comp2);

  Rectangle r2 = r1;
  r2.y = r1.bottom();

  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds", r2 == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(2U, m_layout->boxCount());

  m_layout->setLocation(0, -10);
  r1.y = -10;
  r2.y = r1.bottom();
  CPPUNIT_ASSERT_MESSAGE("Component 1 bounds after setLocation", r1 == comp1->bounds());
  CPPUNIT_ASSERT_MESSAGE("Component 2 bounds after setLocation", r2 == comp2->bounds());
  CPPUNIT_ASSERT_EQUAL(2U, m_layout->boxCount());
}
