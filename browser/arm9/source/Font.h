#ifndef Font_h_seen
#define Font_h_seen

#include <string>
#include <vector>

class Range;

class Font
{
  public:

    struct Glyph
    {
      int width;
      int height;
      const unsigned char * data;
    };

    Font(const std::string & fileName);
    Font(unsigned char * imageData, unsigned char * mapData);
    ~Font();

    void textSize(const char * text,
        int textLength, 
        int & width, 
        int & height, 
        const std::string & encoding="utf-8") const;
    void glyph(unsigned int glyphId, Glyph & g) const;

    inline int totalWidth() const;
    inline int height() const;

  private:

    //! Total width of the image.
    unsigned int m_width;
    unsigned int m_height;
    unsigned char * m_glyphData;
    unsigned char * m_glyphSize;
    unsigned short * m_glyphPosition;
    unsigned short * m_palette;
    std::vector<Range*> m_range;
    std::string m_encoding;

    int valueToIndex(unsigned int glyphId) const;
    int minGlyph() const;
    void init(unsigned char * imageData, unsigned char * mapData);
};

inline int Font::totalWidth() const
{
  return m_width;
}
inline int Font::height() const
{
  return m_height;
}
#endif
