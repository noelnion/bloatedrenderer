#ifndef TGAIMAGE_HPP
#define TGAIMAGE_HPP
#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push,1)
struct TGAHeader {
    std::uint8_t  idlength = 0;
    std::uint8_t  colormaptype = 0;
    std::uint8_t  datatypecode = 0;
    std::uint16_t colormaporigin = 0;
    std::uint16_t colormaplength = 0;
    std::uint8_t  colormapdepth = 0;
    std::uint16_t x_origin = 0;
    std::uint16_t y_origin = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t  bitsperpixel = 0;
    std::uint8_t  imagedescriptor = 0;
};
#pragma pack(pop)

struct TGAColor {
    std::uint8_t bgra[4] = {0,0,0,0};
    std::uint8_t bytespp = 4;
    std::uint8_t& operator[](const int i) { return bgra[i]; }
    const std::uint8_t& operator[](const int i) const { return bgra[i]; }
public:
  TGAColor(std::uint8_t blue,std::uint8_t green, std::uint8_t red, std::uint8_t alpha);
  TGAColor(std::uint8_t blue,std::uint8_t green, std::uint8_t red, std::uint8_t alpha, uint8_t bpp);
  TGAColor() = default;
  
};

struct TGAImage {
    enum Format { GRAYSCALE=1, RGB=3, RGBA=4 };
    enum Datatypecode {UNCOMPRESSED_RGB = 2, UNCOMPRESSED_WB = 3, RLE_RGB = 10, COMPRESSED_WB=11};
  enum Imagedescriptor : std::uint8_t {TOP_LEFT=0x20, BOTTOM_RIGHT=0x10, BOTTOM_LEFT=0x00};
  
    TGAImage() = default;
    TGAImage(const int w, const int h, const int bpp, TGAColor c = {});
    bool  read_tga_file(const std::string& filename);
    bool write_tga_file(const std::string& filename, const bool vflip=true, const bool rle=true) const;
    void flip_horizontally();
    void flip_vertically();
    TGAColor get(const int x, const int y) const;
    void set(const int x, const int y, const TGAColor &c);
    int width()  const;
    int height() const;
private:
    bool   load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out) const;
    int w = 0, h = 0;
    std::uint8_t bpp = 0;
    std::vector<std::uint8_t> data = {};
};

#endif //TGAIMAGE_HPP
