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
  if (!this) return;

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
#if 0
http://www.w3.org/Protocols/rfc2616/rfc2616-sec13.html#sec13.2
  response_time = time(0);
  request_time = time(0) at time of request
  date_value = Date: header from response
  age_value = Age: header from response
  max_age_value = max-age: header from response

  apparent_age = max(0, response_time - date_value);
  corrected_received_age = max(apparent_age, age_value);
  response_delay = response_time - request_time;
  corrected_initial_age = corrected_received_age + response_delay;
  resident_time = now - response_time;
  current_age   = corrected_initial_age + resident_time;
  if have max_age:
    freshness_lifetime = max_age_value
  else
    freshness_lifetime = expires_value - date_value
  response_is_fresh = (freshness_lifetime > current_age)
#endif
}
