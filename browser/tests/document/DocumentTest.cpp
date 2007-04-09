#include <vector>
#include <iostream>
#include <list>
#include <fstream>
#include "Document.h"
#include "HtmlParser.h"
#include "DocumentTest.h"
#include "HtmlElement.h"

using namespace std;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( DocumentTest );

void DocumentTest::readFile(const char * fileName)
{
  ifstream testFile;
  string inputFileName("input/");
  inputFileName += fileName;
  testFile.open(inputFileName.c_str(), ios::in);
  CPPUNIT_ASSERT(testFile.is_open());
  if (testFile.is_open())
  {
    testFile.seekg(0, ios::end);
    m_length = testFile.tellg();
    m_data = new char[m_length+2];
    testFile.seekg(0, ios::beg);
    testFile.read(m_data, m_length);
    m_data[m_length] = 0;
    testFile.close();
  }
  CPPUNIT_ASSERT(m_data != 0);
  CPPUNIT_ASSERT(m_length != 0);
}

void DocumentTest::tearDown()
{
  
  delete m_document;
  if (m_data != 0)
  {
    delete [] m_data;
  }
}

void DocumentTest::setUp()
{
  m_data = 0;
  m_length = 0;
  m_document = new Document;
}

void DocumentTest::test0()
{
  const string expected("file:///test0.txt");
  m_document->setUri(expected);
  string result = m_document->uri();
  CPPUNIT_ASSERT_EQUAL( expected , result);
}

void DocumentTest::test1()
{
  readFile("test1.txt");
  m_document->appendData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  CPPUNIT_ASSERT( result != 0);
}

void DocumentTest::testHtmlAttributes()
{
  readFile("html-attrs.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  string expected("en");
  string attribResult = result->attribute("lang");
  CPPUNIT_ASSERT_EQUAL(expected, attribResult);
}

void DocumentTest::testHead()
{
  readFile("head.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  CPPUNIT_ASSERT(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  CPPUNIT_ASSERT(child != 0);
  CPPUNIT_ASSERT(child->isa("head"));
}

void DocumentTest::testEmpty()
{
  readFile("empty.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  CPPUNIT_ASSERT(result == 0);
}

void DocumentTest::testHead2()
{
  readFile("head2.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  CPPUNIT_ASSERT(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  CPPUNIT_ASSERT(child != 0);
  CPPUNIT_ASSERT(child->isa("head"));
}

void DocumentTest::testHead3()
{
  readFile("head3.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  CPPUNIT_ASSERT(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  CPPUNIT_ASSERT(child != 0);
  CPPUNIT_ASSERT(child->isa("head"));
  const HtmlElement * meta = child->firstChild();
  CPPUNIT_ASSERT(meta != 0);
  CPPUNIT_ASSERT(meta->isa("meta"));

  list<HtmlElement*>::const_iterator it(result->children().begin());
  int index(0);
  for (; it != result->children().end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 0) {
    CPPUNIT_ASSERT(element->isa("head"));
    }
    else if (index == 1)
    {
    CPPUNIT_ASSERT(element->isa("body"));
    }
  }
}

void DocumentTest::testTitle()
{
  readFile("title.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * root = m_document->rootNode();
  CPPUNIT_ASSERT(root->hasChildren());
  const HtmlElement * child = root->firstChild();
  CPPUNIT_ASSERT(child != 0);
  CPPUNIT_ASSERT(child->isa("head"));
  const HtmlElement * meta = child->firstChild();
  CPPUNIT_ASSERT(meta != 0);
  CPPUNIT_ASSERT(meta->isa("title"));

  const list<HtmlElement*> & rootChilds = root->children();
  CPPUNIT_ASSERT(rootChilds.size() > 1);
  list<HtmlElement*>::const_iterator it(rootChilds.begin());
  int index(0);
  for (; it != rootChilds.end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 1) {
      CPPUNIT_ASSERT(element->isa("body"));
    }
  }

}

void DocumentTest::testGoogle()
{
  readFile("google.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * root = m_document->rootNode();
  CPPUNIT_ASSERT(root != 0);
}
