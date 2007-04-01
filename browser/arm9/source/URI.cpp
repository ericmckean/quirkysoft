#include "URI.h"
#include <algorithm>
#include <functional>


URI::URI(std::string & uriString):
  m_protocol(""), m_address("")
{
  int sep(uriString.find(":"));
  if (sep != -1) {
    m_protocol = uriString.substr(0,sep);
    std::transform(m_protocol.begin(), m_protocol.end(), m_protocol.begin(), tolower);
    m_address = uriString.substr(sep+3, uriString.length());
  }
}

bool URI::isFile() const
{
  return m_protocol == "file";
}

bool URI::isValid() const
{
  return m_address != "" and m_protocol != "";
}

std::string URI::server() const
{
  if (isValid() and not isFile())
  {
    return m_address.substr(0, m_address.find("/"));
  }
  return "";
}

const std::string URI::fileName() const
{
  if (isFile()) {
    return m_address;
  } else {
    // strip off server
    std::string serverName(server());
    return m_address.substr(serverName.length(), m_address.length());
  }
}
