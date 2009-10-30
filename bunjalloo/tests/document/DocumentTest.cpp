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
#include <vector>
#include <iostream>
#include <list>
#include <fstream>
#include "HtmlParser.h"
#include "HtmlElement.h"
#include "HtmlDocument.h"
#include "HeaderParser.h"
#include "Document.h"
#include "string_utils.h"
#include <gtest/gtest.h>

using namespace std;

class DocumentTest : public testing::Test
{
  protected:
    Document * m_document;
    char * m_data;
    unsigned int m_length;

    void readFile(const char * fileName);

    void TearDown() {
      delete m_document;
      delete [] m_data;
    }

    void SetUp() {
      m_data = 0;
      m_length = 0;
      m_document = new Document();
    }
};

void DocumentTest::readFile(const char * fileName)
{
  ifstream testFile;
  string inputFileName("input/");
  inputFileName += fileName;
  testFile.open(inputFileName.c_str(), ios::in);
  EXPECT_TRUE(testFile.is_open());
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
  EXPECT_TRUE(m_data != 0);
  EXPECT_TRUE(m_length != 0);
}

TEST_F(DocumentTest, 0)
{
  const string expected("file:///test0.txt");
  m_document->setUri(expected);
  string result = m_document->uri();
  EXPECT_EQ( expected , result);
}

TEST_F(DocumentTest, 1)
{
  readFile("test1.txt");
  m_document->appendData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * result = m_document->rootNode();
  EXPECT_TRUE( result != 0);
}

TEST_F(DocumentTest, HtmlAttributes)
{
  readFile("html-attrs.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  string expected("en");
  string attribResult = result->attribute("lang");
  EXPECT_EQ(expected, attribResult);
}

TEST_F(DocumentTest, Head)
{
  readFile("head.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  EXPECT_TRUE(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));
}

TEST_F(DocumentTest, Empty)
{
  readFile("empty.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  EXPECT_TRUE(result != 0);
}

TEST_F(DocumentTest, Head2)
{
  readFile("head2.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  EXPECT_TRUE(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));
}

TEST_F(DocumentTest, Head3)
{
  readFile("head3.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * result = m_document->rootNode();
  EXPECT_TRUE(result->hasChildren());
  const HtmlElement * child = result->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));
  const HtmlElement * meta = child->firstChild();
  EXPECT_TRUE(meta != 0);
  EXPECT_TRUE(meta->isa("meta"));

  ElementList::const_iterator it(result->children().begin());
  int index(0);
  for (; it != result->children().end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 0) {
    EXPECT_TRUE(element->isa("head"));
    }
    else if (index == 1)
    {
    EXPECT_TRUE(element->isa("body"));
    }
  }
}

TEST_F(DocumentTest, Title)
{
  readFile("title.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root->hasChildren());
  const HtmlElement * child = root->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));
  const HtmlElement * meta = child->firstChild();
  EXPECT_TRUE(meta != 0);
  EXPECT_TRUE(meta->isa("title"));

  const ElementList & rootChilds = root->children();
  EXPECT_TRUE(rootChilds.size() > 1);
  ElementList::const_iterator it(rootChilds.begin());
  int index(0);
  for (; it != rootChilds.end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 1) {
      EXPECT_TRUE(element->isa("body"));
    }
  }

}

TEST_F(DocumentTest, Google)
{
  readFile("google.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
}


TEST_F(DocumentTest, Anchor)
{
  readFile("anchor.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);

  const HtmlElement * child = root->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));

  const ElementList & rootChilds = root->children();
  ElementList::const_iterator it(rootChilds.begin());
  int index(0);
  for (; it != rootChilds.end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 1) {
      EXPECT_TRUE(element->isa("body"));
      EXPECT_TRUE(element->hasChildren());
      const HtmlElement * a = element->firstChild();
      EXPECT_TRUE(a != 0);
      EXPECT_TRUE(a->isa("a"));
      string href = a->attribute("href");
      string expected("anchor");
      EXPECT_EQ(expected, href);
    }
  }
}


TEST_F(DocumentTest, BrokenAnchor)
{
  readFile("anchor-broken.html");
  m_document->appendLocalData(m_data, m_length);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);

  const HtmlElement * child = root->firstChild();
  EXPECT_TRUE(child != 0);
  EXPECT_TRUE(child->isa("head"));

  const ElementList & rootChilds = root->children();
  ElementList::const_iterator it(rootChilds.begin());
  int index(0);
  for (; it != rootChilds.end(); ++it,++index)
  {
    HtmlElement * element(*it);
    if (index == 1) {
      EXPECT_TRUE(element->isa("body"));
      EXPECT_TRUE(element->hasChildren());
      const HtmlElement * a = element->firstChild();
      EXPECT_TRUE(a != 0);
      EXPECT_TRUE(a->isa("a"));
      string href = a->attribute("href");
      string expected("anchor");
      EXPECT_EQ(expected, href);
    }
  }
}

TEST_F(DocumentTest, CharacterStart)
{
  readFile("character-start.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
}
TEST_F(DocumentTest, EndTagStart)
{
  readFile("endtag-start.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  string rootType = root->tagName();
  string expected("html");
  EXPECT_EQ(expected, rootType);
}

TEST_F(DocumentTest, SimpleBodyA)
{
  readFile("simple.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  string expected("html");
  EXPECT_EQ( expected, root->tagName());

  EXPECT_TRUE(root->hasChildren());

  const ElementList & children = root->children();
  ElementList::const_iterator childIt(children.begin());
  expected = "head";
  EXPECT_EQ(expected, (*childIt)->tagName());
  // now check the head:
  {
    // meta and title
    HtmlElement * head = *childIt;
    ElementList::const_iterator headIt(head->children().begin());
    expected = "meta";
    EXPECT_EQ(expected, (*headIt)->tagName());
    expected = "content-type";
    EXPECT_EQ(expected, (*headIt)->attribute("http-equiv"));
    ++headIt;
    expected = "title";
    EXPECT_EQ(expected, (*headIt)->tagName());
    HtmlElement * title = *headIt;
    EXPECT_TRUE(title->hasChildren());
    expected = "#TEXT";
    EXPECT_EQ(expected, title->firstChild()->tagName());
    ++headIt;
  }
  ++childIt;
  expected = "body";
  EXPECT_EQ(expected, (*childIt)->tagName());
  ++childIt;
  EXPECT_TRUE(children.end() == childIt);
}

TEST_F(DocumentTest, MismatchFormat)
{
  readFile("mismatch-format.html");
  // should produce this:
  // html >
  //   head >
  //     body >
  //        #text
  //        b > 
  //           #text (bold)
  //           i > 
  //              #text (bolditalic)
  //        i >
  //           #text (italic)
  //           a > 
  //               #text (link?italic?) 
  //        #text  ()
  //        a >
  //            #text ()
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  string rootType = root->tagName();
  string expected("html");
  EXPECT_EQ(expected, rootType);

}

TEST_F(DocumentTest, Li)
{
  readFile("test-li.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
  EXPECT_TRUE(body->firstChild()->isa("li"));
}

TEST_F(DocumentTest, DD)
{
  readFile("test-dd.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
  EXPECT_TRUE(body->firstChild()->isa("dd"));
}

TEST_F(DocumentTest, Plaintext)
{
  readFile("plaintext.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}


TEST_F(DocumentTest, Font)
{
  readFile("font.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}

TEST_F(DocumentTest, Font2)
{
  readFile("font2.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}

TEST_F(DocumentTest, Pin8)
{
  readFile("pineight.txt");
  // tests for end script tag after end of chunk
  m_document->appendData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}


TEST_F(DocumentTest, Clarin)
{
  readFile("clarin.txt");
  // tests for end script tag after end of chunk
  // send bytes in chunks, emulates http request.
  int length = 1360;
  char * data = m_data;
  m_document->appendData(data, length);
  data += length;
  m_document->appendData(data, length);
  data += length;
  m_document->appendData(data, length);
  data += length;
  length = 4080;
  m_document->appendData(data, length);
  data += length;
  length = 1360;
  m_document->appendData(data, length);
  data += length;
  length = 2720;
  m_document->appendData(data, 2720);
  data += length;
  length = 6800;
  m_document->appendData(data, 6800);
  data += length;
  length = 1360;
  m_document->appendData(data, 1360);
  data += length;
  m_document->appendData(data, 1360);
  data += length;
  length = 2252;
  m_document->appendData(data, 2252);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  EXPECT_TRUE(root->hasChildren());
}


TEST_F(DocumentTest, Adoption)
{
  readFile("adoption.html");
  // test the adoption algorithm
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  const ElementList & children = body->children();
  int childcount = children.size();
  EXPECT_EQ(2, childcount);
}

TEST_F(DocumentTest, Adoption2)
{
  readFile("adoption2.html");
  // test the adoption algorithm
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}

TEST_F(DocumentTest, Header)
{
  readFile("header1.html");
  // test the adoption algorithm
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
  const HtmlElement * h1(0);
  ElementList::const_iterator it(body->children().begin());
  for (; it != body->children().end(); ++it)
  {
    const HtmlElement * element(*it);
    if ( element->isa("h1"))
    {
      h1 = element;
      break;
    }
  }
  EXPECT_TRUE(it != body->children().end());
  EXPECT_TRUE(h1 != 0);
}

TEST_F(DocumentTest, Header2)
{
  readFile("header2.html");
  // test the adoption algorithm
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
  const HtmlElement * h1(0);
  const HtmlElement * h2(0);
  ElementList::const_iterator it(body->children().begin());
  for (; it != body->children().end(); ++it)
  {
    const HtmlElement * element(*it);
    if ( element->isa("h1"))
    {
      h1 = element;
    }
    if ( element->isa("h2"))
    {
      h2 = element;
    }
  }
  EXPECT_TRUE(h1 != 0);
  EXPECT_TRUE(h2 != 0);
}

TEST_F(DocumentTest, Attribs)
{
  readFile("attrib.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
}

#if 0
TEST_F(DocumentTest, Unicode2String)
{
  UnicodeString uc;
  uc += 0xa9;
  string c = unicode2string(uc);
  string expected = "%C2%A9";
  EXPECT_EQ(expected, c);
}
#endif

TEST_F(DocumentTest, ActiveFormatters)
{
  readFile("issue29.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));

  // get the body node
  ElementList bodyNodes = root->elementsByTagName("body");
  EXPECT_TRUE(bodyNodes.size() == 1);

  const HtmlElement * body = bodyNodes.front();
  const HtmlElement * bold = body->firstChild();
  EXPECT_TRUE(bold->isa("b"));

  ElementList pNodes = bold->elementsByTagName("p");
  EXPECT_EQ(2, pNodes.size());

  // now see if the 2nd p node has multiple children
  const HtmlElement * p = pNodes.back();
  EXPECT_EQ(1, p->children().size());


  const HtmlElement * child = p->firstChild();
  // see how many recursive children the p has
  int depth = 0;
  while (child->hasChildren())
  {
    child = child->firstChild();
    depth++;
  }
  EXPECT_EQ(1, depth);
}

TEST_F(DocumentTest, History)
{
  readFile("attrib.html");
  m_document->setUri("attrib.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setPosition(10);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));

  delete [] m_data;
  readFile("anchor.html");
  m_document->setUri("anchor.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  EXPECT_EQ(-1, m_document->position());

  string prev = m_document->gotoPreviousHistory();

  string expected = "attrib.html";
  EXPECT_EQ(expected, prev);
  EXPECT_EQ(10, m_document->position());
}

TEST_F(DocumentTest, BodyEnd)
{
  readFile("body.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
}

TEST_F(DocumentTest, HistoryWithConfig)
{
  m_document->setUri("attrib.html");
  m_document->setUri("anchor.html");
  m_document->setUri("config://index.html");
  m_document->setUri("config://update?language=en");
  m_document->clearConfigHistory();
  string result = m_document->uri();
  string expected = "anchor.html";
  EXPECT_EQ(expected, result);
  EXPECT_TRUE( not m_document->hasNextHistory());

}

TEST_F(DocumentTest, Tokenize)
{
  string str("ABCD:EFáGH");
  vector<string> strTokens;
  tokenize(str, strTokens, string(":"));
  string expected("ABCD");
  EXPECT_EQ(expected, strTokens[0]);
  expected = "EFáGH";
  EXPECT_EQ(expected, strTokens[1]);

}

TEST_F(DocumentTest, NoCacheHtml)
{
  readFile("nocache.txt");
  // tests for no-cache and mime type breakage
  m_document->appendData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  HtmlParser::MimeType mimeType = m_document->htmlDocument()->mimeType();
  HtmlParser::MimeType expected = HtmlParser::TEXT_HTML;
  EXPECT_EQ(expected, mimeType);
}

TEST_F(DocumentTest, Entities)
{
  readFile("entities.html");
  // tests for entities that are nasty
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
}

TEST_F(DocumentTest, redirect)
{
  const string data(
      "HTTP/1.1 302 MOVED TEMPORARILY\r\n"
      "content-type:text/html; charset=UTF-8\r\n"
      "location:/new/location/\r\n"
      "\r\n"
      "<html>test</html>");
  const string initial("http://www.example.com/foo/bar");
  m_document->setUri(initial);
  m_document->appendData(data.c_str(), data.length());
  string result = m_document->uri();
  EXPECT_EQ(Document::REDIRECTED, m_document->status());
  EXPECT_EQ("http://www.example.com/new/location/", result);
}

TEST_F(DocumentTest, issue112_no_space_after_link)
{
  readFile("issue112.html");
  m_document->appendLocalData(m_data, m_length);
  m_document->setStatus(Document::LOADED_HTML);
  const HtmlElement * root = m_document->rootNode();
  EXPECT_TRUE(root != 0);
  EXPECT_TRUE(root->isa("html"));
  const HtmlElement * body = root->lastChild();
  EXPECT_TRUE(body != 0);
  EXPECT_TRUE(body->isa("body"));
  const ElementList &nodes = body->children();
  std::vector<const HtmlElement*> elements;
  copy(nodes.begin(), nodes.end(), back_inserter(elements));
  EXPECT_EQ(3, elements.size());
  const HtmlElement *a(elements[0]);
  EXPECT_EQ("a", a->tagName());
  const HtmlElement *text(elements[1]);
  EXPECT_EQ("#TEXT", text->tagName());
  const HtmlElement *br(elements[2]);
  EXPECT_EQ("br", br->tagName());

  std::string linkText = a->firstChild()->text();
  EXPECT_EQ("'foo'", linkText);

  std::string postLinkText = text->text();
  EXPECT_EQ(" bar", postLinkText);
}
