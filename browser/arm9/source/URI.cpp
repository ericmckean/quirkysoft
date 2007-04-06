#include "URI.h"
#include <algorithm>
#include <functional>

URI::URI()
  : m_protocol(""),m_address("")
{
}

URI::URI(const std::string & uriString):
  m_protocol(""), m_address("")
{
  setUri(uriString);
}

void URI::setUri(const std::string & uriString)
{
  std::string tmpUri = uriString;
  int sep(tmpUri.find(":"));
  if (sep == -1) {
    tmpUri = "http://" + uriString;
    sep = tmpUri.find(":");
  }
  if (sep != -1) {
    m_protocol = tmpUri.substr(0,sep);
    std::transform(m_protocol.begin(), m_protocol.end(), m_protocol.begin(), tolower);
    m_address = tmpUri.substr(sep+3, tmpUri.length());
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
    int firstSlash(m_address.find("/"));
    if (firstSlash == -1) {
      return m_address;
    }
    return m_address.substr(0, firstSlash);
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
    if (m_address.length() == serverName.length()) {
      return "/";
    }
    return m_address.substr(serverName.length(), m_address.length());
  }
}

bool URI::operator==(const URI & other)
{
  return m_protocol == other.m_protocol and m_address==other.m_address;
}
bool URI::operator!=(const URI & other)
{
  return not operator==(other);
}
