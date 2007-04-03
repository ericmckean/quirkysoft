#ifndef Document_h_seen
#define Document_h_seen
#include <string>
#include <vector>
#include "ViewI.h"

class Document
{
  public:

    enum Status {
      NOTHING,
      LOADED,
      INPROGRESS
    };
    Document();

    void setUri(const std::string & uriString);
    void setData(const char * data, int size); 
    const std::string & uri() const;
    const char * asText() const;
    Status status() const;
    void setLoading(int amount);

    void registerView(ViewI * v);
    void unregisterView(ViewI * v);

  private:
    std::string m_uri;
    char * m_data;
    Status m_status;
    int m_amount;
    std::vector<ViewI*> m_views;

    void notifyAll() const;
};
#endif
