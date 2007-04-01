#ifndef Controller_h_seen
#define Controller_h_seen

class Document;
class View;
class URI;

#include <string>
#include "ControllerI.h"

class Controller : public ControllerI
{
  public:
    Controller(std::string & uri);

    void mainLoop();
    void keyPress();

  private:
    Document & m_document;
    View & m_view;

    void localFile(const std::string &);
    void fetchHttp(const URI &);
};
#endif
