#ifndef ControllerI_h_seen
#define ControllerI_h_seen

#include <string>
class ControllerI
{
  public:
    virtual void doUri(const std::string & uri) = 0;
    virtual ~ControllerI() {}
};
#endif
