#include <assert.h>
#include "ndspp.h"
#include "libnds.h"
#include "Canvas.h"
#include "Document.h"
#include "ViewRender.h"
#include "View.h"
#include "TextArea.h"
#include "HtmlElement.h"
#include "HtmlImageElement.h"
#include "HtmlBodyElement.h"
#include "UnicodeString.h"
#include "File.h"

using namespace std;

ViewRender::ViewRender(View * self):
  m_self(self), 
  m_pendingNewLines(0),
  m_haveShownSomething(false)
{
}

void ViewRender::addRealNewline(int count)
{
  m_self->m_textArea->setParseNewline(true);
  UnicodeString newlines(count, '\n');
  m_self->m_textArea->printu(newlines);
  m_self->m_textArea->setParseNewline(false);
}

void ViewRender::preNodeFormatting(const HtmlElement * node)
{
  if (node->isa("li"))
  {
    HtmlElement p("p");
    p.appendText(' ');
    p.appendText('*');
    p.appendText(' ');
    applyFormatting(p.firstChild());
  }
  else if (node->isa("ul"))
  {
    if ( node->parent()->isa("li") or node->parent()->isa("ul"))
      m_pendingNewLines = 0;
    else
      m_pendingNewLines++;
  }
  else if (node->tagName()[0] == 'h' and (node->tagName()[1] >= '1' and node->tagName()[1] <= '6'))
  {
    if (m_pendingNewLines < 2)
      m_pendingNewLines++;;
  }
  else if (node->isa("p"))
  {
    m_pendingNewLines+=2;
  }
}

void ViewRender::postNodeFormatting(const HtmlElement * node)
{
  if (node->isa("li"))
  {
    m_pendingNewLines++;
  }
  else if (node->isa("a"))
  {
    m_self->m_textArea->setTextColor(nds::Color(0,0,0));
    m_self->m_textArea->setLink(false);
  }
  else if (node->isa("div"))
  {
    m_pendingNewLines++;
  }
  else if (node->tagName()[0] == 'h' and (node->tagName()[1] >= '1' and node->tagName()[1] <= '6'))
  {
    m_pendingNewLines += 2;
  } 
  else if (node->isa("ul"))
  {
    if (node->parent()->isa("li") or node->parent()->isa("ul"))
      m_pendingNewLines = 0;
    else
      m_pendingNewLines++;
  }
  else if (node->isa("p"))
  {
    m_pendingNewLines+=2;
  }
}

void ViewRender::flushNewlines()
{
  if (m_pendingNewLines and m_haveShownSomething)
  {
    addRealNewline(m_pendingNewLines>2?2:m_pendingNewLines);
  }
  m_pendingNewLines = 0;
}

bool ViewRender::applyFormatting(const HtmlElement * element)
{
  if (element->isa("a"))
  {
    m_self->m_textArea->setTextColor(nds::Color(0,0,31));
    m_self->m_textArea->addLink(element);
  } 
  else if (element->text().size())
  {
    UnicodeString delims((unsigned int)' ',1); delims += '\n';
    unsigned int pos = element->text().find_first_not_of(delims);
    if (pos != UnicodeString::npos) {
      // if grandparent is a li then change of model
      if ( ( element->parent()->isa("ul") or element->parent()->isa("ol") ) 
          and element->parent()->parent()->isa("li"))
        m_pendingNewLines = 1;
      flushNewlines();
      m_haveShownSomething = true;
      m_self->m_textArea->printu(element->text());
      if (element->parent()->isa("ul"))
        m_pendingNewLines++;
    }
  }
  else if (element->isa("script") or element->isa("style"))
  {
    return false;
  }
  else if (element->isa("img"))
  {
    // hurrah for alt text. some people set it to "", which screws up any
    // easy way to display it (see w3m google.com - Google [hp1] [hp2] [hp3]... huh?)
    flushNewlines();
    m_haveShownSomething = true;
    const UnicodeString & altText = element->attribute("alt");
    bool hasAltText = ((HtmlImageElement*)element)->hasAltText();
    if (hasAltText) {
      if (not altText.empty()) {
        m_self->m_textArea->setTextColor(nds::Color(0,31,0));
        m_self->m_textArea->printu(altText);
        m_self->m_textArea->setTextColor(nds::Color(0,0,0));
      }
      return true;
    }
    const UnicodeString & srcText = element->attribute("src");
    m_self->m_textArea->setTextColor(nds::Color(0,31,0));
    if (not srcText.empty())
    {
      string tmp = unicode2string(srcText);
      const char * bname = nds::File::base(tmp.c_str());
      string bnamestr(bname);
      bnamestr = "["+ bnamestr+"]";
      UnicodeString ustr ( string2unicode(bnamestr));
      m_self->m_textArea->printu(ustr);
      m_self->m_textArea->setTextColor(nds::Color(0,0,0));
      return true;
    }
    UnicodeString ustr( string2unicode("[IMG]"));
    m_self->m_textArea->printu(ustr);
    m_self->m_textArea->setTextColor(nds::Color(0,0,0));
  }
  else if (element->isa("br"))
  {
    flushNewlines();
    m_pendingNewLines++;
    flushNewlines();
  } 
  return true;
}

void ViewRender::walkNode(const HtmlElement * node)
{
  preNodeFormatting(node);
  if (node->hasChildren())
  {
    const ElementList & theChildren = node->children();
    ElementList::const_iterator it(theChildren.begin());
    for (; it != theChildren.end(); ++it)
    {
      const HtmlElement * element(*it);
      if (applyFormatting(element)) {
        walkNode(element);
      }
    }
  }
  postNodeFormatting(node);
}

void ViewRender::setBgColor(const HtmlElement * body)
{
  UnicodeString bgcolor = body->attribute("bgcolor");
  if (not bgcolor.empty())
  {
    unsigned int rgb8 = ((HtmlBodyElement*)body)->bgColor();
    nds::Color col( ((rgb8 >> 16)&0xff)/8, ((rgb8 >> 8)&0xff)/8, (rgb8&0xff)/8);
    m_self->m_textArea->setBackgroundColor(col);
  }
}

void ViewRender::render()
{
  nds::Canvas::instance().fillRectangle(0, 0, SCREEN_WIDTH, 2*SCREEN_HEIGHT, nds::Color(31,31,31));
  m_self->m_textArea->setCursor(0, 0);
  m_self->m_textArea->setParseNewline(false);
  m_pendingNewLines = 0;
  m_haveShownSomething = false;
  const HtmlElement * root = m_self->m_document.rootNode();
  assert(root->isa("html"));
  assert(root->hasChildren());
  const HtmlElement * body = root->lastChild();
  setBgColor(body);
  m_self->m_textArea->clear();
  if (body->hasChildren())
  {
    walkNode(body);
  }
}


