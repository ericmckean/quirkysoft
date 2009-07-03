#include <stdio.h>
#include "config_defs.h"
#include "CacheControl.h"
#include "ParameterSet.h"


CacheControl::CacheControl()
: m_maxAge(0),
  m_time(0),
  m_noCache(false),
  m_noStore(false)
{}
void CacheControl::reset()
{
  m_noCache = false;
  m_noStore = false;
  m_time = 0;
  m_maxAge = 0;
}

void CacheControl::setSeconds(unsigned int time)
{
  m_time = time;
}

void CacheControl::setCacheControl(const std::string &value)
{
  if (!this)
    return;

  // split on commas
  ParameterSet paramSet(value, ',');
  std::string ma;
  m_time = 0;
  m_maxAge = 0;
  if (paramSet.parameter("max-age", ma)) {
    // got
    int matches = sscanf_platform(ma.c_str(), "%d", &m_maxAge);
    if (matches != 1) {
      m_maxAge = 0;
    }
  }

  m_noCache = paramSet.hasParameter("no-cache");
  m_noStore = paramSet.hasParameter("no-store");
}

void CacheControl::setExpires(const std::string &value)
{
}

bool CacheControl::shouldCache() const
{
  if (m_noCache) {
    return false;
  }
  if (m_noStore) {
    return false;
  }
  return m_maxAge >= m_time;
}
