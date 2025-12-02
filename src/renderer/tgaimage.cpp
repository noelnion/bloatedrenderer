#include <iostream>
#include <cstring>
#include <span>
#include <cstdint>
#include <fstream>
#include "tgaimage.hpp"
#include <string>
#include <vector>
#include <array>
#include <string_view>
#include <bit>

TGAImage::TGAImage(const int width, const int height, const int bpp, TGAColor color)
  : w(width), h(height), bpp(bpp)
{
  auto data_size = static_cast<std::size_t>(w * h * static_cast<int>(bpp));
  data = std::vector<std::uint8_t>(data_size, 0);
  for (int j=0; j<height; j++){
	for (int i=0; i<width; i++){
            set(i, j, color);
	}
  }
}

 TGAColor::TGAColor(std::uint8_t blue, std::uint8_t green, std::uint8_t red, std::uint8_t alpha)
{
  bgra[0] = blue;
  bgra[1] = green;
  bgra[2] = red;
  bgra[3] = alpha;
}

TGAColor::TGAColor(std::uint8_t blue, std::uint8_t green, std::uint8_t red, std::uint8_t alpha, uint8_t bpp) : bytespp(bpp)
{
  bgra[0] = blue;
  bgra[1] = green;
  bgra[2] = red;
  bgra[3] = alpha;

}

bool TGAImage::read_tga_file(const std::string& filename) {
    std::ifstream instream;
    instream.open(filename, std::ios::binary);
    if (!instream.is_open()) {
        std::cerr << "can't open file " << filename << "\n";
        return false;
    }
    TGAHeader header;
    instream.read(reinterpret_cast<char *>(&header), sizeof(header));
    if (!instream.good()) {
        std::cerr << "an error occured while reading the header\n";
        return false;
    }
    w   = header.width;
    h   = header.height;
    bpp = header.bitsperpixel>> (static_cast<uint8_t>(3));
    if (w<=0 || h<=0 || (bpp!=GRAYSCALE && bpp!=RGB && bpp!=RGBA)) {
        std::cerr << "bad bpp (or width/height) value\n";
        return false;
    }
    const auto nbytes = static_cast<size_t>(static_cast<int>(bpp)*w*h);
    data = std::vector<std::uint8_t>(nbytes, 0);
    if (Datatypecode::UNCOMPRESSED_WB==header.datatypecode || Datatypecode::UNCOMPRESSED_RGB==header.datatypecode) {
	  instream.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(nbytes));
        if (!instream.good()) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    } else if (Datatypecode::RLE_RGB==header.datatypecode||Datatypecode::COMPRESSED_WB==header.datatypecode) {
        if (!load_rle_data(instream)) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
    } else {
	  std::cerr << "unknown file format " << static_cast<int>(header.datatypecode) << "\n";
        return false;
    }
    if (!(static_cast<bool>(header.imagedescriptor & Imagedescriptor::TOP_LEFT))){
        flip_vertically();
	}
    if (static_cast<bool>(header.imagedescriptor & Imagedescriptor::BOTTOM_RIGHT)){
        flip_horizontally();
	}
	int bits_in_byte = 8;
    std::cerr << w << "x" << h << "/" << bpp*bits_in_byte << "\n";
    return true;
}

bool TGAImage::load_rle_data(std::ifstream &instream) {
  const auto pixelcount = static_cast<size_t>(w*h);
  const int maxpixelcount = 128;
    size_t currentpixel = 0;
    size_t currentbyte  = 0;
    TGAColor colorbuffer {};;
    do {
        std::uint8_t chunkheader = 0;
        chunkheader = instream.get();
        if (!instream.good()) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
        if (chunkheader<maxpixelcount) {
            chunkheader++;
            for (unsigned int i=0; i<chunkheader; i++) {
                instream.read(reinterpret_cast<char *>(colorbuffer.bgra), bpp);
                if (!instream.good()) {
                    std::cerr << "an error occured while reading the header\n";
                    return false;
                }
                for (int channel=0; channel <static_cast<int>(bpp); channel++){
                    data[currentbyte++] = colorbuffer.bgra[channel];
				}
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        } else {
		  chunkheader -= (maxpixelcount-1);
            instream.read(reinterpret_cast<char *>(colorbuffer.bgra), bpp);
            if (!instream.good()) {
                std::cerr << "an error occured while reading the header\n";
                return false;
            }
            for (int i=0; i<static_cast<int>(chunkheader); i++) {
			  for (int channel=0; channel<static_cast<int>(bpp); channel++){
                    data[currentbyte++] = colorbuffer.bgra[channel];
			  }
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
    } while (currentpixel < pixelcount);
    return true;
}

bool TGAImage::write_tga_file(const std::string& filename, const bool vflip, const bool rle) const
{
  constexpr std::array<std::uint8_t, 4> developer_area_ref = { 0, 0, 0, 0 };
  constexpr std::array<std::uint8_t, 4> extension_area_ref = { 0, 0, 0, 0 };
  constexpr std::array<std::uint8_t, 18> footer = {
    'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'
  };
  std::ofstream out;
  out.open(filename, std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "can't open file " << filename << "\n";
    return false;
  }
  TGAHeader header = {};
  header.bitsperpixel = bpp << static_cast<uint8_t>(3);
  header.width = static_cast<uint16_t>(w);
  header.height = static_cast<uint16_t>(h);
  header.datatypecode = (bpp == GRAYSCALE ? (rle ? Datatypecode::COMPRESSED_WB : Datatypecode::UNCOMPRESSED_WB) : (rle ? Datatypecode::RLE_RGB : Datatypecode::UNCOMPRESSED_RGB));
  header.imagedescriptor = vflip ? Imagedescriptor::BOTTOM_LEFT : Imagedescriptor::TOP_LEFT;// top-left or bottom-left origin
  out.write(reinterpret_cast<const char *>(&header), sizeof(header));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  if (!rle) {
    const std::span<const uint8_t> data_span(data);
    auto bytes = std::as_bytes(data_span);
    // out.write(reinterpret_cast<const char *>(data.data()), w*h*bpp);
    out.write(std::bit_cast<const char *>(bytes.data()), w * h * bpp);
    if (!out.good()) {
      std::cerr << "can't dump the tga file\n";
      return false;
    }
  } else if (!unload_rle_data(out)) {
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  out.write(reinterpret_cast<const char *>(developer_area_ref.data()), sizeof(developer_area_ref));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  out.write(reinterpret_cast<const char *>(extension_area_ref.data()), sizeof(extension_area_ref));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  out.write(reinterpret_cast<const char *>(footer.data()), sizeof(footer));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  return true;
}

bool TGAImage::unload_rle_data(std::ofstream &out) const {
    const std::uint8_t max_chunk_length = 128;
    const auto npixels = static_cast<size_t>(w*h);
    size_t curpix = 0;
    while (curpix<npixels) {
	    size_t chunkstart = curpix*static_cast<size_t>(bpp);
        size_t curbyte = curpix*static_cast<size_t>(bpp);
        std::uint8_t run_length = 1;
        bool raw = true;
        while (curpix+run_length<npixels && run_length<max_chunk_length) {
            bool succ_eq = true;
            for (int channel=0; succ_eq && channel<static_cast<int>(bpp); channel++){
			  succ_eq = (data[curbyte+static_cast<size_t>(channel)]==data[curbyte+static_cast<size_t>(channel+bpp)]);
			}
            curbyte += bpp;
            if (1==run_length){
			  raw = !succ_eq;
			}
            if (raw && succ_eq) {
                run_length--;
                break;
            }
            if (!raw && !succ_eq){
                break;
			}
            run_length++;
        }
        curpix += run_length;
        out.put(raw ? run_length-1 : run_length+max_chunk_length-1);
        if (!out.good()) {return false;}
        out.write(reinterpret_cast<const char *>(data.data()+chunkstart), (raw?run_length*bpp:bpp));
        if (!out.good()) {return false;}
    }
    return true;
}

TGAColor TGAImage::get(const int x, const int y) const {
  if (!data.size() || x<0 || y<0 || x>=w || y>=h) {return {};}
  TGAColor ret(0, 0, 0, 0, bpp);
    const std::uint8_t *pointer = data.data()+((x+y*w)*bpp);
    for (int i=bpp; static_cast<bool>(i--); ret.bgra[i] = pointer[i]) {;}
    return ret;
}

void TGAImage::set(int x, int y, const TGAColor &color) {
  if (!data.size() || x<0 || y<0 || x>=w || y>=h) {
	return;
  }
    memcpy(data.data()+((x+y*w)*bpp), color.bgra, bpp);
}

void TGAImage::flip_horizontally() {
  for (int i=0; i<w/2; i++){
	for (int j=0; j<h; j++){
	  for (int channel=0; channel<bpp; channel++){
                std::swap(data[((i+j*w)*bpp)+channel], data[((w-1-i+j*w)*bpp)+channel]);
	  }
	}
  }
}

void TGAImage::flip_vertically() {
  for (int i=0; i<w; i++){
	for (int j=0; j<h/2; j++){
	  for (int channel=0; channel<bpp; channel++){
                std::swap(data[((i+j*w)*bpp)+channel], data[((i+(h-1-j)*w)*bpp)+channel]);
	  }
	}
  }
}

int TGAImage::width() const {
    return w;
}

int TGAImage::height() const {
    return h;
}

