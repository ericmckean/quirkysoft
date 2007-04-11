#include "HtmlElementTest.h"
#include "HtmlElement.h"
#include "HtmlMetaElement.h"
#include "HtmlAnchorElement.h"
#include "ElementFactory.h"

using namespace std;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( HtmlElementTest );

void HtmlElementTest::tearDown()
{
  delete m_element;
  delete m_clone;
}
void HtmlElementTest::setUp()
{
  m_element = 0;
  m_clone = 0;
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


void HtmlElementTest::testAnchor()
{
  m_element = ElementFactory::create("a");
  CPPUNIT_ASSERT( m_element->isa("a"));
  HtmlAnchorElement * a = dynamic_cast<HtmlAnchorElement*>(m_element);
  CPPUNIT_ASSERT( a != 0 );
}


void HtmlElementTest::testClone()
{
  m_element = ElementFactory::create("a");
  m_element->setAttribute("href", "http://localhost");
  m_element->setAttribute("id", "myElement");
  m_clone = m_element->clone();
  CPPUNIT_ASSERT( m_clone != 0);
  CPPUNIT_ASSERT( m_clone->isa("a"));
  CPPUNIT_ASSERT_EQUAL(m_element->attribute("href"), m_clone->attribute("href"));
  CPPUNIT_ASSERT_EQUAL(m_element->attribute("id"), m_clone->attribute("id"));
}


void HtmlElementTest::testParent()
{
  // add a node, check its parent is the right one
  m_element = ElementFactory::create("html");
  HtmlElement * child = ElementFactory::create("head");
  m_element->append(child);

  CPPUNIT_ASSERT( child->parent() == m_element);
}

void HtmlElementTest::testAppendText()
{
  // parentNode -> add text, add node, add text 
  // check the text nodes are correct
  m_element = ElementFactory::create("body");
  m_element->appendText('T');
  CPPUNIT_ASSERT(m_element->hasChildren());
  HtmlElement * text = m_element->firstChild();
  string expected("#text");
  CPPUNIT_ASSERT_EQUAL(expected, text->tagName());
  HtmlElement * a = ElementFactory::create("a");
  a->appendText('L');
  m_element->append(a);
  CPPUNIT_ASSERT_EQUAL(a, m_element->lastChild());
  CPPUNIT_ASSERT(a->hasChildren());
  m_element->appendText('2');
  CPPUNIT_ASSERT_EQUAL(expected, m_element->lastChild()->tagName());

}

void HtmlElementTest::testMeta()
{
  // create a meta node, then check the type
  m_element = ElementFactory::create("meta");
  CPPUNIT_ASSERT( m_element->isa("meta"));
  HtmlMetaElement * meta = dynamic_cast<HtmlMetaElement*>(m_element);
  CPPUNIT_ASSERT( meta != 0 );

  string expected("content-type");
  m_element->setAttribute("http-equiv", "content-type");
  CPPUNIT_ASSERT_EQUAL(expected, m_element->attribute("http-equiv"));
}

void HtmlElementTest::testRemove()
{
  // create a node, add another, check removal
  m_element = ElementFactory::create("body");
  m_element->appendText('T');
  HtmlElement * a = ElementFactory::create("a");
  HtmlElement * b = ElementFactory::create("b");
  m_element->append(a);
  m_element->append(b);
  size_t expected(3);
  CPPUNIT_ASSERT_EQUAL(expected, m_element->children().size());

  m_element->remove(a);
  expected = 2;
  CPPUNIT_ASSERT_EQUAL(expected, m_element->children().size());

  string expectedText("#text");
  CPPUNIT_ASSERT_EQUAL(expectedText, m_element->firstChild()->tagName());
  expectedText = "b";
  CPPUNIT_ASSERT_EQUAL(expectedText, m_element->lastChild()->tagName());
  delete a;
}

