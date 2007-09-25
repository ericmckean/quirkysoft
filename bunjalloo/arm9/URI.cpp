/*
  Copyright (C) 2007 Richard Quirk

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
#include "URI.h"
#include <algorithm>
#include <vector>
#include <functional>

using namespace std;

static const char * const HTTP_STR = "http";
static const char * const HTTPS_STR = "https";
static const char * const FILE_STR = "file";
static const char * const CONFIG_STR = "config";
static const char * const GET_STR = "GET";

URI::URI()
  : m_method(GET_STR), m_protocol(""),m_address(""),m_requestHeader("")
{
}

URI::URI(const std::string & uriString, bool fix):
  m_method(GET_STR), m_protocol(""), m_address(""), m_fix(fix)
{
  setUri(uriString);
}

void URI::setUri(const std::string & uriString)
{
  string tmpUri = uriString;
  unsigned int sep(tmpUri.find(":"));
  if (sep != string::npos)
  {
    // check it isn't the port seperator and we are missing a http: at the start
    // e.g. like this ->  myproxy.com:8080
    if (tmpUri[sep+1] != '/')
    {
      // is it a port?
      unsigned int nextSlash(tmpUri.find("/"));
      if (nextSlash == string::npos) {
        sep = string::npos;
      } else {
        string maybePort = tmpUri.substr(sep+1, nextSlash-1-sep);
        int port = strtol(maybePort.c_str(), 0, 10);
        if (port != 0)
        {
          // it was a port - make sure we add HTTP to the URL
          sep = string::npos;
        }
      }
    }
  }

  if (sep == string::npos) {
    if (not m_fix) {
      return;
    }
    tmpUri = HTTP_STR;
    tmpUri += "://" + uriString;
    sep = tmpUri.find(":");
  }
  if (sep != string::npos) {
    m_protocol = tmpUri.substr(0,sep);
    transform(m_protocol.begin(), m_protocol.end(), m_protocol.begin(), tolower);
    m_address = tmpUri.substr(sep+3, tmpUri.length());
  }
}

URI::Protocol_t URI::protocol() const
{
  if (m_protocol == FILE_STR) return FILE_PROTOCOL;
  if (m_protocol == HTTP_STR) return HTTP_PROTOCOL;
  if (m_protocol == HTTPS_STR) return HTTPS_PROTOCOL;
  if (m_protocol == CONFIG_STR) return CONFIG_PROTOCOL;
  return UNKNOWN_PROTOCOL;
}

bool URI::isValid() const
{
  if (m_address == "") 
  {
    return false;
  }
  switch (protocol())
  {
    case FILE_PROTOCOL:
    case HTTPS_PROTOCOL:
    case HTTP_PROTOCOL:
    case CONFIG_PROTOCOL:
      return true;
    default:
      return false;
  }
}

int URI::port() const
{
  int firstSlash(m_address.find("/"));
  int portDots(m_address.find(":"));
  int defaultPort = protocol() == HTTPS_PROTOCOL?443:80;
  if (portDots == -1)
  {
    return defaultPort;
  }
  if (firstSlash == -1 or (firstSlash > portDots))
  {
    int amount = firstSlash==-1?m_address.length():firstSlash;
    amount -= portDots;
    if (amount == 1) {
      return defaultPort;
    }
    string portStr = m_address.substr(portDots+1, amount);
    return strtol(portStr.c_str(), 0, 0);
  }
  return defaultPort;
}

std::string URI::server() const
{
  if (isValid() and (protocol() == HTTP_PROTOCOL or protocol() == HTTPS_PROTOCOL))
  {
    int firstSlash(m_address.find("/"));
    int portDots(m_address.find(":"));
    if (firstSlash == -1 and portDots == -1) {
      return m_address;
    }
    // has / but not : -> http://server/
    if (firstSlash != -1 and portDots == -1)
    {
      return m_address.substr(0, firstSlash);
    }
    // has : but not / ->  http://server:8080
    if (portDots != -1 and (firstSlash == -1 or portDots < firstSlash)) {
      return m_address.substr(0, portDots);
    }
    if (firstSlash != -1) {
      return m_address.substr(0, firstSlash);
    }
    return m_address.substr(0, m_address.length());
  }
  return "";
}

const std::string URI::fileName() const
{
  switch(protocol())
  {
    case FILE_PROTOCOL:
    case CONFIG_PROTOCOL:
      return m_address;
    case HTTP_PROTOCOL:
      {
        // strip off server
        int firstSlash(m_address.find("/"));
        if (firstSlash == -1)
        {
          return "/";
        }
        else
        {
          return m_address.substr(firstSlash, m_address.length()-firstSlash);
        }
        string serverName(server());
        if (m_address.length() == serverName.length()) {
          return "/";
        }
        return m_address.substr(serverName.length(), m_address.length());
      }
      break;
    default:
      return "";
  }
}

static void tokenize(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

URI URI::navigateTo(const std::string & newFile ) const
{
  if (newFile.length() == 0)
    return *this;

  // first see if the newFile is a valid URI
  URI tmp(*this);
  tmp.m_address = "";
  tmp.m_protocol = "";
  tmp.m_fix = false;

  tmp.setUri(newFile);
  if (tmp.isValid())
  {
    if ((protocol() == HTTP_PROTOCOL or protocol() == HTTPS_PROTOCOL) 
        and (tmp.protocol() != HTTP_PROTOCOL and tmp.protocol() != HTTPS_PROTOCOL))
    {
      // Security problem - remote files shouldn't be able to link to local one.
      return *this;
    }
    // else FILE / CONFIG can link to HTTP.
    // return the new URI 
    tmp.m_fix = m_fix;
    return tmp;
  }
  tmp = *this;
  // first change this: www.server.com/foo/bar -> navigateTo("/path/to/file") 
  // should go to www.server.com/path/to/file
  // string newURI(m_protocol);
  // newURI += "://";
  string newURI;
  if (newFile[0] == '/')
  {
    // ok, so strip off the last part and try again
    int firstSlash(tmp.m_address.find("/"));
    if (firstSlash == -1)
    {
      newURI += tmp.m_address + newFile;
    }
    else
    {
      newURI += tmp.m_address.substr(0,firstSlash) + newFile;
    }
  }
  else
  {
    // strip off last part of file and go here.
    int lastSlash(tmp.m_address.rfind("/"));
    if (lastSlash == -1)
    {
      newURI += tmp.m_address + "/" + newFile;
    }
    else
    {
      newURI += tmp.m_address.substr(0,lastSlash) +"/"+ newFile;
    }
  }
  // if contains dots -> strip them out
  vector<string> pathElements;
  vector<string> newPath;
  tokenize(newURI, pathElements, "/");

  vector<string>::const_iterator it(pathElements.begin());
  for (; it != pathElements.end();++it)
  {
    if ( (*it) == "..")
    {
      // don't pop the last segment (servername)
      if (newPath.size() > 1)
      {
        newPath.pop_back();
      }
    }
    else
    {
      newPath.push_back(*it);
    }
  }
  it = newPath.begin();
  newURI = tmp.m_protocol + "://";
  bool needSep(false);
  if (protocol() == FILE_PROTOCOL)
    needSep = true;

  for (; it != newPath.end();++it)
  {
    if (needSep)
    {
      newURI += '/';
    }
    newURI += *it;
    needSep = true;
  }
  // Make sure not to lose trailing slash
  if (newFile[newFile.length()-1] == '/')
  {
    newURI += "/";
  }
  
  tmp.setUri(newURI);
  return tmp;
}

const std::string URI::asString() const
{
  return m_protocol+"://"+m_address;
}

bool URI::operator==(const URI & other)
{
  return m_protocol == other.m_protocol and m_address==other.m_address;
}
bool URI::operator!=(const URI & other)
{
  return not operator==(other);
}

static bool isEscapable(unsigned int value)
{
  return ::isblank(value) or value == '&' or value == '#' or value == '?';
}

UnicodeString URI::escape(const UnicodeString & input)
{
  // escape non URI values like space and stuff.
  UnicodeString output;
  UnicodeString::const_iterator it(input.begin());
  char buffer[4];
  for ( ; it != input.end(); ++it)
  {
    unsigned int value = *it;
    if ( isEscapable(value))
    {
#ifdef ARM9
      siprintf(buffer, "%%%02X", value);
#else
      sprintf(buffer, "%%%02X", value);
#endif
      char * src = buffer;
      while (*src != 0) {
        output += *src;
        ++src;
      }
    }
    else
    {
      output += value;
    }
  }
  return output;
}

const std::string URI::method() const
{
  return m_method;
}

void URI::setMethod(const std::string & method)
{
  if (not method.empty())
  {
    m_method = method;
  }
  else
  {
    m_method = GET_STR;
  }
  transform(m_method.begin(), m_method.end(), m_method.begin(), ::toupper);
}

void URI::setRequestHeader(const std::string & headerData)
{
  m_requestHeader = headerData;
}

std::string URI::requestHeader() const
{
  return m_requestHeader;
}