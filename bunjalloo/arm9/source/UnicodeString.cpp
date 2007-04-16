#include "UnicodeString.h"
std::string unicode2string(const UnicodeString & ustr)
{
  std::string str;
  UnicodeString::const_iterator it(ustr.begin());
  for ( ; it != ustr.end() ; ++it)
  {
    str += *it;
  }
  return str;
}

UnicodeString string2unicode(const std::string & str)
{
  UnicodeString ustr;
  std::string::const_iterator it(str.begin());
  for ( ; it != str.end() ; ++it)
  {
    ustr += *it;
  }
  return ustr;
}

