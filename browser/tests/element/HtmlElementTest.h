#ifndef HtmlElementTest_h_seen
#define HtmlElementTest_h_seen

#include <cppunit/extensions/HelperMacros.h>

class HtmlElement;
class HtmlElementTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( HtmlElementTest );
  CPPUNIT_TEST( test0 );
  CPPUNIT_TEST( testAttributes );
  CPPUNIT_TEST( testAppend );
  CPPUNIT_TEST( testIsa );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();
  void test0();
  void testAttributes();
  void testAppend();
  void testIsa();

private:
  HtmlElement * m_element;
};

#endif
