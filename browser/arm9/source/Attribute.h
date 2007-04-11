#ifndef Attribute_h_seen
#define Attribute_h_seen
#include <vector>
//! Attribute for an element.
struct Attribute
{
  std::string name;   //!< name of the attribute.
  std::string value;  //!< value, stripped of leading/trailing quotes.
};

typedef std::vector<Attribute*> AttributeVector;
#endif
