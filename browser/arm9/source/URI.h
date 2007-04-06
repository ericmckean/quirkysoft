#ifndef URI_h_seen
#define URI_h_seen
#include <string>
class URI
{
  public:
    URI();
    URI(const std::string & uriString);

    void setUri(const std::string & uriString);
    bool isFile() const;
    bool isValid() const;
    std::string server() const;
    const std::string fileName() const;

    bool operator==(const URI & other);
    bool operator!=(const URI & other);

  private:
    std::string m_protocol;
    std::string m_address;
};
#endif
