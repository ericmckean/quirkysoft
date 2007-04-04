#include "ndspp.h"
#include "libnds.h"
#include "Controller.h"

using namespace nds;
int main(int argc, char * argv[])
{

  irqInit();
  irqSet(IRQ_VBLANK,(VoidFunctionPointer)0);
  Controller c;
  if (argc > 1) {
    std::string fileName(argv[1]);
    c.doUri(fileName);
  } else {
    //std::string fileName("http://www.gbadev.org/");
    //c.doUri(fileName);
  }
  c.mainLoop();

}
