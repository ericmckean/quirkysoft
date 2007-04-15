#ifndef Controller_h_seen
#define Controller_h_seen

class Document;
class View;
class URI;

#include "ControllerI.h"

/** Controller maps user actions to model updates. For example, responds to setting the URI.
 */
class Controller : public ControllerI
{
  public:
    //!Constructor.
    Controller();
    //! Destructor
    ~Controller();

    /** Handle the user request for setting a URI.
     * @param uriString the URI string to use.
     */
    void doUri(const std::string & uriString);
    
    /** Show the software licence.*/
    void showLicence();

    /** Loops forever.*/
    void mainLoop();

  private:
    Document * m_document;
    View * m_view;

    void localFile(const std::string &);
    void fetchHttp(URI &);
};
#endif
