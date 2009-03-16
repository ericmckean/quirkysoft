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
  CPPUNIT_TEST_SUITE(TextTest);
  CPPUNIT_TEST(testSimple);
  CPPUNIT_TEST(testUnicode);
  CPPUNIT_TEST(testFindLast);
  CPPUNIT_TEST(testFindLast2);
  CPPUNIT_TEST(testCrap);
  CPPUNIT_TEST(testNewline);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testSimple();
  void testUnicode();
  void testFindLast();
  void testFindLast2();
  void testNewline();
  void testCrap();
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

  std::string result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("this "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("is "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("a "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("string"), result);
}

void TextTest::testUnicode() {
  std::string s("thís ís à üniÇod€ strîñg");
  std::string::const_iterator it(s.begin());
  std::string::const_iterator end_it(s.end());

  std::string result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("thís "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("ís "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("à "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("üniÇod€ "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("strîñg"), result);
}

void TextTest::testCrap() {
  std::string s("\"«»\n");
  std::string::const_iterator it(s.begin());
  std::string::const_iterator end_it(s.end());
  std::string result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(s, result);
}

void TextTest::testFindLast() {
  std::string s(" ñÑaá$Ó");
  static const std::string delimeter(" \r\n\t");
  int lastPosition = findLastNotOf(s, delimeter);
  CPPUNIT_ASSERT_EQUAL(11, lastPosition);

  s = "Ña";
  lastPosition = findLastNotOf(s, delimeter);
  CPPUNIT_ASSERT_EQUAL(2, lastPosition);
}

void TextTest::testFindLast2() {
  std::string s(" Ñ  ");
  static const std::string delimeter(" \t");
  int lastPosition = findLastNotOf(s, delimeter);
  CPPUNIT_ASSERT_EQUAL(3, lastPosition);
}

void TextTest::testNewline() {
  std::string s("line with\nnew line");
  std::string::const_iterator it(s.begin());
  std::string::const_iterator end_it(s.end());
  std::string result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("line "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("with\n"), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("new "), result);
  result = nextWordAdvanceWord(&it, end_it, false);
  CPPUNIT_ASSERT_EQUAL(std::string("line"), result);
}
