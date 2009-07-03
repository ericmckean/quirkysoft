#include "CacheControl.h"


void CacheControl::setSeconds(unsigned int time)
{
}

void CacheControl::setCacheControl(const std::string &value)
{
  /*
  if (value.find("no-cache") != std::string::npos)
  {
    if (not m_cacheFile.empty())
    {
      m_cache = false;
    }
  }
  */
}

void CacheControl::setExpires(const std::string &value)
{
}

bool CacheControl::shouldCache() const
{
  return true;
}
