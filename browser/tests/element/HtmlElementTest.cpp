#include "HtmlElementTest.h"
#include "HtmlElement.h"
#include "ElementFactory.h"

using namespace std;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( HtmlElementTest );

void HtmlElementTest::tearDown()
{
  delete m_element;
}
void HtmlElementTest::setUp()
{
}

void HtmlElementTest::test0()
{
  m_element = ElementFactory::create("html");
  CPPUNIT_ASSERT( m_element != 0);
}

void HtmlElementTest::testAttributes()
{
  m_element = ElementFactory::create("html");

  string expected("en");
  m_element->setAttribute("lang", expected);
  string result = m_element->attribute("lang");
  CPPUNIT_ASSERT_EQUAL( expected, result);

  expected = "newId";
  m_element->setAttribute("id", expected);
  result = m_element->attribute("id");
  CPPUNIT_ASSERT_EQUAL( expected, result);

  expected = "Test title";
  m_element->setAttribute("title", expected);
  result = m_element->attribute("title");
  CPPUNIT_ASSERT_EQUAL( expected, result);

  expected = "fichero";
  m_element->setAttribute("dir", expected);
  result = m_element->attribute("dir");
  CPPUNIT_ASSERT_EQUAL( expected, result);

  // try setting some unsupported attribute
  m_element->setAttribute("test", expected);
  result = m_element->attribute("test");
  expected = "";
  CPPUNIT_ASSERT_EQUAL( expected, result);


}


void HtmlElementTest::testAppend()
{
  m_element = ElementFactory::create("html");
  HtmlElement * head = ElementFactory::create("head");
  m_element->append(head);
  CPPUNIT_ASSERT( m_element->hasChildren());
}

void HtmlElementTest::testIsa()
{
  m_element = ElementFactory::create("html");
  CPPUNIT_ASSERT( m_element->isa("html"));
  delete m_element;
  m_element = ElementFactory::create("head");
  CPPUNIT_ASSERT( m_element->isa("head"));
}
