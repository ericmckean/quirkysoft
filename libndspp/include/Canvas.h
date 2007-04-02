#ifndef Canvas_h_seen
#define Canvas_h_seen

namespace nds
{
  class Background;

  class Canvas
  {
    public:
      static Canvas & instance();
      void drawPixel(int x, int y, int color);

    private:
      Background * m_bgMain;
      Background * m_bgSub;

      Canvas();
      ~Canvas();

  };
}
#endif
