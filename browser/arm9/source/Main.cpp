#include "ndspp.h"
#include "libnds.h"
#include "Controller.h"

using namespace nds;
int main(int argc, char * argv[])
{

  irqInit();
  irqSet(IRQ_VBLANK,(VoidFunctionPointer)0);
  if (argc > 1) {
    std::string fileName(argv[1]);
    Controller c(fileName);
    c.mainLoop();
  } else {
    std::string fileName("http://www.gbadev.org/");
    Controller c(fileName);
    c.mainLoop();
  }

}
