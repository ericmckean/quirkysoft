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
#include "Cookie.h"
#include "CookieWriter.h"
#include "URI.h"
#include "File.h"
#include <gtest/gtest.h>
#include <fstream>

using namespace std;

class CookieTest : public testing::Test
{
  protected:
    CookieJar * m_cookieJar;

    void SetUp() {
      m_cookieJar = new CookieJar();
      nds::File::mkdir("data/bunjalloo/cookies");
    }

    void TearDown() {
      nds::File::rmrf("data");
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
  expectedHeader = "Cookie: subcount=1; topcount=2\r\n";
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

TEST_F(CookieTest, ignore_httponly)
{
  m_cookieJar->setAcceptCookies("example.com");
  URI uri("http://example.com/");
  string requestHeader = "mycookie=1234;path=/;HttpOnly\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  requestHeader = "mycookie2=5678;path=/;HttpOnly\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);

  string expectedHeader = "Cookie: mycookie=1234; mycookie2=5678\r\n";
  string resultHeader;
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

TEST_F(CookieTest, cookie_expired)
{
  Cookie c;
  EXPECT_FALSE(c.expired(1));

  Cookie c2("bla", "buzz", 80, "example.com", "/", 99, false);
  EXPECT_FALSE(c2.expired(98));
  EXPECT_FALSE(c2.expired(99));
  EXPECT_TRUE(c2.expired(100));
}

TEST_F(CookieTest, is_session)
{
  Cookie c;
  EXPECT_TRUE(c.session());

  Cookie c2("bla", "buzz", 80, "example.com", "/", 99, false);
  EXPECT_FALSE(c2.session());
}

TEST_F(CookieTest, Expires)
{
  m_cookieJar->setAcceptCookies("example.com");
  // check that cookies expire when they should
  URI uri("http://example.com/");
  string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  string expectedHeader = "Cookie: mycookie=foo\r\n";
  string resultHeader;
  time_t when = 1246611088; // Fri 03 Jul
  m_cookieJar->cookiesForRequest(uri, resultHeader, when);
  EXPECT_EQ(expectedHeader, resultHeader);
  Cookie *c(m_cookieJar->hasCookieForDomain(uri, "mycookie"));
  EXPECT_NE((void*)0, c);
  EXPECT_FALSE(c->expired(when - 1));
  EXPECT_TRUE(c->expired(1246705272 + 1));

  when = 1246811088; // Sun 05 Jul
  resultHeader = "";
  m_cookieJar->cookiesForRequest(uri, resultHeader, when);
  EXPECT_EQ("", resultHeader);
}

TEST_F(CookieTest, cookie_to_string)
{
  m_cookieJar->setAcceptCookies("example.com");
  string requestHeader = "mycookie=foo";
  URI uri("http://example.com/");
  m_cookieJar->addCookieHeader(uri, requestHeader + "\r\n");
  Cookie *c(m_cookieJar->hasCookieForDomain(uri, "mycookie"));
  EXPECT_EQ(requestHeader + ";path=/", c->asString());

  requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT";
  m_cookieJar->addCookieHeader(uri, requestHeader + "\r\n");
  c = m_cookieJar->hasCookieForDomain(uri, "mycookie");
  EXPECT_EQ(requestHeader+ ";path=/", c->asString());
}

TEST_F(CookieTest, cookie_to_string_2)
{
  m_cookieJar->setAcceptCookies("example.com");
  string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT;path=/;secure";
  URI uri("http://example.com/accounts/foo");
  m_cookieJar->addCookieHeader(uri, requestHeader + "\r\n");
  Cookie *c(m_cookieJar->hasCookieForDomain(uri, "mycookie"));
  EXPECT_EQ(requestHeader, c->asString());
}

TEST_F(CookieTest, writes_to_file)
{
  Cookie c("bla", "buzz", 80, "example.com", "/", 99, false);
  nds::File file;
  CookieWriter cw;
  cw(&c);
  EXPECT_TRUE(nds::File::exists("data/bunjalloo/cookies/example.com"));
}

TEST_F(CookieTest, loads_cookies)
{
  m_cookieJar->setAcceptCookies("example.com");
  // create a fake cookie file
  ofstream testFile;
  testFile.open("data/bunjalloo/cookies/example.com", ios::out);
  testFile << "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
  testFile.close();

  URI uri("http://example.com/accounts/foo");
  string expectedHeader = "Cookie: mycookie=foo\r\n";
  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader, 99);
  EXPECT_EQ(expectedHeader, resultHeader);
}

TEST_F(CookieTest, saves_cookies)
{
  m_cookieJar->setAcceptCookies("example.com");
  URI uri("http://example.com/accounts/foo");
  string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  nds::File f;
  f.open("data/bunjalloo/cookies/example.com", "r");
  EXPECT_TRUE(f.is_open());
  std::vector<std::string> lines;
  f.readlines(lines);
  f.close();
  EXPECT_EQ(1U, lines.size());
  EXPECT_EQ(requestHeader.substr(0, requestHeader.length() - 2) + ";path=/accounts", lines[0]);
}

TEST_F(CookieTest, does_not_duplicate_entries)
{
  // check that if we have persistent cookies for example.com
  // and we go to example2.com, that the cookies for example.com are removed
  // this prevents the list of in-memory cookies from becoming huge.
  m_cookieJar->setAcceptCookies("example.com");
  m_cookieJar->setAcceptCookies("two.example.com");
  {
    URI uri("http://example.com/");
    string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
    m_cookieJar->addCookieHeader(uri, requestHeader);
  }
  {
    URI uri("http://two.example.com/");
    string requestHeader = "mycookie2=bar;Expires=Sun, 11 Jul 2009 18:01:12 GMT\r\n";
    m_cookieJar->addCookieHeader(uri, requestHeader);
  }
  nds::File f;
  f.open("data/bunjalloo/cookies/example.com", "r");
  EXPECT_TRUE(f.is_open());
  std::vector<std::string> lines;
  f.readlines(lines);
  f.close();
  EXPECT_EQ(1U, lines.size());
  EXPECT_EQ("mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT;path=/", lines[0]);
}

TEST_F(CookieTest, doesnt_save_session_cookies)
{
  m_cookieJar->setAcceptCookies("example.com");
  URI uri("http://example.com/accounts/foo");
  string requestHeader = "mycookie=foo\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  nds::File f;
  f.open("data/bunjalloo/cookies/example.com", "r");
  EXPECT_FALSE(f.is_open());
}

TEST_F(CookieTest, saves_multiple_cookies)
{
  m_cookieJar->setAcceptCookies("example.com");
  URI uri("http://example.com/accounts/foo");
  string requestHeader = "mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  requestHeader = "mycookie2=bar;Expires=Sun, 05 Jul 2009 13:22:00 GMT;path=/\r\n";
  m_cookieJar->addCookieHeader(uri, requestHeader);
  nds::File f;
  f.open("data/bunjalloo/cookies/example.com", "r");
  EXPECT_TRUE(f.is_open());
  std::vector<std::string> lines;
  f.readlines(lines);
  f.close();
  ASSERT_EQ(2U, lines.size());
  EXPECT_EQ("mycookie=foo;Expires=Sat, 04 Jul 2009 12:01:12 GMT;path=/accounts", lines[0]);
  EXPECT_EQ("mycookie2=bar;Expires=Sun, 05 Jul 2009 13:22:00 GMT;path=/", lines[1]);

  string resultHeader;
  m_cookieJar->cookiesForRequest(uri, resultHeader, 0);
  string expectedHeader = "Cookie: mycookie=foo; mycookie2=bar\r\n";
  EXPECT_EQ(expectedHeader, resultHeader);
}
