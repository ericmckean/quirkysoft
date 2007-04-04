#ifndef Controller_h_seen
#define Controller_h_seen

class Document;
class View;
class URI;

#include "ControllerI.h"

class Controller : public ControllerI
{
  public:
    Controller();

    void doUri(const std::string & uriString);
    void mainLoop();

  private:
    Document & m_document;
    View & m_view;

    void localFile(const std::string &);
    void fetchHttp(const URI &);
};
#endif
