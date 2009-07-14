#include <cstdio>
#include "CookieWriter.h"
#include "Cookie.h"
#include "File.h"
#include "config_defs.h"

static const char COOKIE_DIR[] = DATADIR"/cookies/";

void CookieWriter::operator()(Cookie *cookie)
{
  if (cookie->session())
    return;
  if (cookie->saved())
    return;
  std::string filename(COOKIE_DIR);
  const std::string &domain(cookie->domain());
  if (domain[0] == '.')
    filename += domain.substr(1, domain.size()-1);
  else
    filename += domain;

  nds::File f;
  f.open(filename.c_str(), "a");
  if (f.is_open()) {
    f.write(cookie->asString().c_str());
    f.write("\n");
  }
  cookie->setSaved(true);
}

void CookieWriter::remove(const char *domain)
{
  std::string filename(COOKIE_DIR);
  filename += domain;
  nds::File::rmrf(filename.c_str());
}
