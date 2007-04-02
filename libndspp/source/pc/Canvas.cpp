#include "Canvas.h"
#include "Video.h"
#include "SDLhandler.h"

using namespace nds;

Canvas & Canvas::instance()
{
  static Canvas s_canvas;
  return s_canvas;
}

Canvas::~Canvas(){}

Canvas::Canvas()
{
  Video & main(Video::instance());
  Video & sub(Video::instance(1));
  sub.setMode(5);
  main.setMode(5);
}

void Canvas::drawPixel(int x, int y, int colour)
{
  int layer = ( (y < 192) ? 0:1 );
  if (layer) {
    y-=192;
  }
  SDLhandler::instance().drawPixel(x,y,layer,colour);
}
