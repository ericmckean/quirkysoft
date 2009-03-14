/*
  Copyright (C) 2009 Richard Quirk

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
#include "UnicodeString.h"

#include <cppunit/extensions/HelperMacros.h>

class TextTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TextTest );
  CPPUNIT_TEST( testSimple );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSimple();

};

CPPUNIT_TEST_SUITE_REGISTRATION(TextTest);

void TextTest::setUp()
{
}

void TextTest::tearDown()
{
}

void TextTest::testSimple()
{
  std::string s("this is a string");
  std::string::const_iterator it(s.begin());
  std::string::const_iterator end_it(s.end());

  std::string w1 = nextWordAdvanceWord(it, end_it);
  CPPUNIT_ASSERT_EQUAL(std::string("this"), w1);

}
