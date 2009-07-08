#include <stdio.h>
#include "config_defs.h"
#include "CacheControl.h"
#include "ParameterSet.h"


CacheControl::CacheControl()
: m_noCache(false),
  m_noStore(false),
  m_ageValue(-1),
  m_date(-1),
  m_expires(-1),
  m_maxAge(-1),
  m_requestTime(0),
  m_responseTime(0)
{}

void CacheControl::reset()
{
  if (!this) return;

  m_ageValue = -1;
  m_date = -1;
  m_expires = -1;
  m_maxAge = -1;
  m_requestTime = 0;
  m_responseTime = 0;
  m_noCache = false;
  m_noStore = false;
}

void CacheControl::setAge(time_t age)
{
  m_ageValue = age;
}

void CacheControl::setCacheControl(const std::string &value)
{
  if (!this)
    return;

  // split on commas
  ParameterSet paramSet(value, ',');
  std::string ma;
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

void CacheControl::setDate(time_t date)
{
  m_date = date;
}

void CacheControl::setExpires(time_t expires)
{
  m_expires = expires;
}

void CacheControl::setRequestTime(time_t request)
{
  m_requestTime = request;
}

void CacheControl::setResponseTime(time_t response)
{
  m_responseTime = response;
}

bool CacheControl::shouldCache(time_t now) const
{
  if (m_noCache) {
    return false;
  }
  if (m_noStore) {
    return false;
  }

  if (m_date != -1) {
    // http://www.w3.org/Protocols/rfc2616/rfc2616-sec13.html#sec13.2
    int apparent_age = std::max(0L, m_responseTime - m_date);
    int corrected_received_age = std::max(apparent_age, m_ageValue);
    int response_delay = m_responseTime - m_requestTime;
    int corrected_initial_age = corrected_received_age + response_delay;
    int resident_time = now - m_responseTime;
    int current_age = corrected_initial_age + resident_time;
    int freshness_lifetime = -1;
    if (m_maxAge != -1) {
      freshness_lifetime = m_maxAge;
    }
    else {
      freshness_lifetime = m_expires - m_date;
    }
    return freshness_lifetime > current_age;
  } else {
    // server didn't send the date header.
    // Cache for half a minute.
    return (m_responseTime + m_maxAge + 30) > now;
  }
}
