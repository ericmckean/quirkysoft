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
#include "CookieJar.h"
#include "URI.h"
#include <gtest/gtest.h>

using namespace std;

class CookieTest : public testing::Test
{
  protected:
    CookieJar * m_cookieJar;

    void SetUp() {
      m_cookieJar = new CookieJar();
    }

    void TearDown() {
      delete m_cookieJar;
    }
};

TEST_F(CookieTest, Basic)
{
  URI uri("http://localhost/");
  m_cookieJar->setAcceptCookies("localhost");
  // this is from the server Set-Cookie (from HeaderParser):
  string requestHeader = "a=b\r\n";
  // this is what we return next time:
  string expectedHeader ="Cookie: a=b\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);
}

TEST_F(CookieTest, Repeats)
{
  URI uri("http://localhost/");
  m_cookieJar->setAcceptCookies("localhost");
  // this is from the server Set-Cookie (from HeaderParser):
  string requestHeader = "a=b\r\n";
  // this is what we return next time:
  string expectedHeader ="Cookie: a=b\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  m_cookieJar->addCookieHeader(uri, requestHeader);

  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);
}

TEST_F(CookieTest, Accept)
{
  const string server("foobar.com");
  bool expected = false;
  bool result = m_cookieJar->acceptCookies(server);
  EXPECT_EQ(expected, result) << "Should reject foobar.com";
  // now set to allow
  m_cookieJar->setAcceptCookies(server, true);
  expected = true;
  result = m_cookieJar->acceptCookies(server);
  EXPECT_EQ(expected, result) << "Should allow foobar.com";
}

TEST_F(CookieTest, CalcTopLevel)
{
  const string alreadyTop("toplevel.com");
  string result = CookieJar::topLevel(alreadyTop);
  EXPECT_EQ(alreadyTop, result);

  const string subdomain("www.someurl.com");
  string expected = "someurl.com";
  result = CookieJar::topLevel(subdomain);
  EXPECT_EQ(expected, result);

  const string dotcom(".com");
  expected = ".com";
  result = CookieJar::topLevel(dotcom);
  EXPECT_EQ(expected, result);

  const string justcom("com");
  expected = "com";
  result = CookieJar::topLevel(justcom);
  EXPECT_EQ(expected, result);
}

TEST_F(CookieTest, SubDomain)
{
  const string settingServer("www.domain.com");
  const string subDomain("sub.domain.com");
  // accept cookies for all *.domain.com pages
  m_cookieJar->setAcceptCookies("domain.com");

  bool expected = true;
  bool result = m_cookieJar->acceptCookies(settingServer);
  EXPECT_EQ(expected, result) << "Should allow www.domain.com";
  result = m_cookieJar->acceptCookies(subDomain);
  EXPECT_EQ(expected, result) << "Should allow sub.domain.com";

  URI uri("http://sub.domain.com/");
  // now set cookie for sub domain - www.domain should not read it
  string requestHeader = "subcount=1\r\n";
  string expectedHeader = "Cookie: subcount=1\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);

  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);
  // check www.domain.com doesn't return it
  uri.setUri("http://www.domain.com");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  expectedHeader = "";
  EXPECT_TRUE(m_cookieJar->hasCookieForDomain(URI("sub.domain.com"),"subcount") != 0);
  EXPECT_EQ(expectedHeader, resultHeader);

  // check that setting a cookie readable across all domains works
  // set for uri www.domain.com, but with domain=domain.com, ie. top level.
  requestHeader = "topcount=2;domain=domain.com;path=/\r\n";
  expectedHeader = "Cookie: topcount=2\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  uri.setUri("http://some.domain.com");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_TRUE(m_cookieJar->hasCookieForDomain(URI("domain.com"),"topcount") != 0);
  EXPECT_TRUE(m_cookieJar->hasCookieForDomain(URI("domain.com"),"subcount") == 0);
  EXPECT_EQ(expectedHeader, resultHeader);

  // test that for domain.com it includes the top level cookie
  uri.setUri("http://domain.com");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

  // now test that for sub.domain.com it returns the domain.com cookie AND the
  // sub.domain.com specific cookie.
  expectedHeader = "Cookie: subcount=1\r\nCookie: topcount=2\r\n";
  uri.setUri("http://sub.domain.com");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

}

TEST_F(CookieTest, Path)
{
  const string settingServer("www.domain.com");
  const string subDomain("sub.domain.com");
  // accept cookies for all *.domain.com pages
  m_cookieJar->setAcceptCookies("domain.com");
  string requestHeader = "topcount=2;domain=domain.com;path=/accounts/\r\n";

  // add a path specific cookie
  URI uri("http://sub.domain.com/accounts/");
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string expectedHeader = "Cookie: topcount=2\r\n";
  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  // check it works
  EXPECT_EQ(expectedHeader, resultHeader);

  // check some other path cannot read it
  uri.setUri("http://sub.domain.com/");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  expectedHeader = "";
  EXPECT_EQ(expectedHeader, resultHeader);

  // check a sub dir can read it
  uri.setUri("http://sub.domain.com/accounts/mine");
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  expectedHeader = "Cookie: topcount=2\r\n";
  EXPECT_TRUE(m_cookieJar->hasCookieForDomain(uri,"topcount") != 0);
  EXPECT_EQ(expectedHeader, resultHeader);

  // test adding some unrelated site's cookie
  m_cookieJar->setAcceptCookies("elsewhere.com");
  requestHeader = "SD=richard;domain=elsewhere.com;path=/\r\n";
  uri.setUri("http://www.elsewhere.com/login");
  m_cookieJar->addCookieHeader(uri, requestHeader);

  expectedHeader = "Cookie: SD=richard\r\n";
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);


  m_cookieJar->setAcceptCookies("mail.gmail.com");
  requestHeader = "EMAIL_AT=BIG_Scary_H4sh; Path=/mail; Secure\r\n";
  uri.setUri("https://mail.gmail.com/mail/?whatever");
  m_cookieJar->addCookieHeader(uri, requestHeader);

  expectedHeader = "Cookie: EMAIL_AT=BIG_Scary_H4sh\r\n";
  resultHeader = "";
  uri.setUri("https://mail.gmail.com/mail/?zx=fizzbuzz&sva=1");
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

}

TEST_F(CookieTest, GoogleLogin)
{
  // this test checks that setting a cookie from a cgi file correctly
  // changes the path to the directory name.

  m_cookieJar->setAcceptCookies("google.com");
  string requestHeader = "GALX=1234;Secure\r\n";
  URI uri("https://www.google.com/accounts/ServiceLogin?service=mail");
  m_cookieJar->addCookieHeader(uri, requestHeader);

  string expectedHeader = "Cookie: GALX=1234\r\n";
  string resultHeader;

  uri.setUri("https://www.google.com/accounts/ServiceLoginAuth?service=mail");
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

}

TEST_F(CookieTest, Secure)
{
  // don't send secure cookies via non secure connection
  m_cookieJar->setAcceptCookies("domain.com");

  URI uri("https://sub.domain.com/accounts/");
  string requestHeader = "LSID=ff9123;domain=domain.com;path=/accounts/;secure\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string expectedHeader = "Cookie: LSID=ff9123\r\n";
  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

  // what about non secure?
  uri.setUri("http://sub.domain.com/accounts/");
  resultHeader = "";
  expectedHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);
}

TEST_F(CookieTest, ExpireRenew)
{
  // make sure that we renew cookies
  m_cookieJar->setAcceptCookies("domain.com");

  URI uri("https://sub.domain.com/accounts/foo");
  string requestHeader = "LSID=EXPIRED;Secure\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string expectedHeader = "Cookie: LSID=EXPIRED\r\n";
  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

  // now renew it
  uri.setUri("https://sub.domain.com/accounts/foo");
  requestHeader = "LSID=newval;secure\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  expectedHeader = "Cookie: LSID=newval\r\n";
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader);
  EXPECT_EQ(expectedHeader, resultHeader);

}

TEST_F(CookieTest, Expires)
{
  m_cookieJar->setAcceptCookies("example.com");
  // check that cookies expire when they should
  URI uri("http://example.com/accounts/foo");
  string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string expectedHeader = "Cookie: mycookie=foo\r\n";
  string resultHeader;
  time_t when = 1246611088; // Fri 03 Jul
  m_cookieJar->cookiesForRequest(uri, resultHeader, when);
  EXPECT_EQ(expectedHeader, resultHeader);

  when = 1246811088; // Sun 05 Jul
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader, when);
  EXPECT_EQ("", resultHeader);
}
