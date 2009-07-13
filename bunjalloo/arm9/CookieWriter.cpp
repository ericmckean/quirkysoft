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
  std::string filename(COOKIE_DIR);
  filename += cookie->domain();
  nds::File f;
  f.open(filename.c_str(), "w");
  if (f.is_open()) {
    f.write(cookie->asString().c_str());
  }
}
