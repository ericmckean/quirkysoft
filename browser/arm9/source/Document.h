#ifndef Document_h_seen
#define Document_h_seen
#include <string>
#include <vector>
#include "ViewI.h"

class Document
{
  public:
    Document(std::string & uri);

    void setData(const char * data, int size); 
    const std::string & uri() const;
    const char * asText() const;

    void registerView(ViewI * v);
    void unregisterView(ViewI * v);

  private:
    std::string m_uri;
    char * m_data;
    std::vector<ViewI*> m_views;
};
#endif
