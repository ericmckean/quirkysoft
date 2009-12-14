#include <gtest/gtest.h>
#include <unistd.h>
#include <memory>
#include <sstream>
#define private public
#include "Controller.h"
#undef private
#include "Cache.h"
#include "Config.h"
#include "Document.h"
#include "HttpClient.h"
#include "HtmlElement.h"
#include "HtmlParser.h"
#include "HtmlDocument.h"

void runServer(int times)
{
  std::stringstream dbg;
  dbg << "python server.py " << times << " &";
  system(dbg.str().c_str());
  // hackily sleep until the server starts. ugh.
  sleep(1);
}

Controller *createController()
{
  Controller *c(new Controller());
  c->cache()->setUseCache(true);
  c->m_config = new Config();
  c->m_httpClient->setController(c);
  return c;
}

TEST(UpdaterTest, test_updates)
{
  runServer(1);
  std::auto_ptr<Controller> c(createController());
  URI uri("http://localhost:8000/data/version.txt");
  c->doUri(uri);
  EXPECT_EQ(HtmlParser::TEXT_PLAIN, c->m_document->htmlDocument()->mimeType());

  EXPECT_TRUE(c->m_httpClient->hasPage());
  EXPECT_EQ("# update file\nversion=0.8\n\nURL=http://localhost/data/newversion\n",
      c->m_document->htmlDocument()->data());
}
