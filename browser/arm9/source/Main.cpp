#include "ndspp.h"
#include "libnds.h"
#include "Controller.h"

using namespace nds;
int main(int argc, char * argv[])
{
  irqInit();
  irqSet(IRQ_VBLANK,0);
  Controller * c = new Controller;
  if (argc > 1) {
    std::string fileName(argv[1]);
    c->doUri(fileName);
  }
  else
  {
    c->showLicence();
  }
  c->mainLoop();
}
