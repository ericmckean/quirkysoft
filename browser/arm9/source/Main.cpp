#include "ndspp.h"
#include "libnds.h"
#include "Controller.h"

using namespace nds;
int main(int argc, char * argv[])
{

  Video & mainScreen = Video::instance();
  Video & subScreen  = Video::instance(1);
  mainScreen.mode(0);
  subScreen.mode(0);
  Background bg(1,0,0,31);
  bg.enable();
  Palette palette(1);
  palette[255] = Color(31,31,31);
  irqInit();
  irqSet(IRQ_VBLANK,(VoidFunctionPointer)0);

#ifdef ARM9
  consoleInitDefault(bg.mapData(), bg.tileData(),16);
  iprintf("Starting html\n");
#endif
  if (argc > 1) {
    std::string fileName(argv[1]);
    Controller c(fileName);
    c.mainLoop();
  } else {
    std::string fileName("http://www.google.com/");
    Controller c(fileName);
    c.mainLoop();
  }

}
