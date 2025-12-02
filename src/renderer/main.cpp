#include "tgaimage.hpp"
#include <algorithm>
#include <cmath>

void draw_line1(const int sx,const int sy,const int dx, const int dy, TGAImage &img, const TGAColor &color, const float step = 0.001F)
{
  const auto nsteps = static_cast<int>(1.0F / step);
  for (int i = 0; i <= nsteps; i++) {
    const float t_param = static_cast<float>(i) * step;
    const int x_to_draw = sx + static_cast<int>(t_param * static_cast<float>(dx - sx));
    const int y_to_draw = sy + static_cast<int>(t_param * static_cast<float>(dy - sy));
    img.set(x_to_draw, y_to_draw, color);
  }
}

void draw_line2(const int sx,const int sy,const int dx, const int dy, TGAImage &img, const TGAColor &color)
{

  const int x_min = std::min(sx, dx);
  const int x_max = std::max(sx, dx);
  for (int x_t = x_min; x_t <= x_max; x_t++) {
    const auto t_param = static_cast<float>(x_t-sx)/static_cast<float>(dx-sx);
    const int y_to_draw = sy + static_cast<int>(t_param * static_cast<float>(dy - sy));
    img.set(x_t, y_to_draw, color);
  }
}

void draw_line3(int sx, int sy, int dx,  int dy, TGAImage &img, const TGAColor &color)
{

  bool switched = false;
  const auto slope = static_cast<float>(dy - sy) / static_cast<float>(dx - sx);
  
  if(std::abs(slope) > 1){
	std::swap(sx, sy);
	std::swap(dx, dy);
	switched = true;
  }

    if (sx > dx) {
    std::swap(sx, dx);
    std::swap(sy, dy);
  }
  
  const int x_min = std::min(sx, dx);
  const int x_max = std::max(sx, dx);

  for (int x_t = x_min; x_t <= x_max; x_t++) {
    const auto t_param = static_cast<float>(x_t - sx) / static_cast<float>(dx - sx);
    const int y_to_draw = sy + static_cast<int>(t_param * static_cast<float>(dy - sy));
    if (switched) {
      img.set(y_to_draw, x_t, color);
    } else {
      img.set(x_t, y_to_draw, color);
    }
  }
}

int main([[maybe_unused]]int argc,[[maybe_unused]] const char** argv){

  const TGAColor white  (255, 255, 255, 255); // attention, BGRA order
  const TGAColor green  (  0, 255,   0, 255);
  const TGAColor red    (  0,   0, 255, 255);
  const TGAColor blue   (255, 128,  64, 255);
  const TGAColor yellow (  0, 200, 255, 255);
 
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);


	//points
    constexpr int ax =  7;
	constexpr int ay =  3;
	
    constexpr int bx = 12;
	constexpr int by = 37;
	
    constexpr int cx = 62;
	constexpr int cy = 53;

	//lines
	draw_line3(ax, ay, bx, by, framebuffer, blue);
	draw_line3(bx, by, ax, ay, framebuffer, yellow);
	draw_line3(cx, cy, bx, by, framebuffer, green);
	draw_line3(ax, ay, cx, cy, framebuffer, red);
	draw_line3(cx, cy, ax, ay, framebuffer, yellow);
	draw_line3(ax, ay, bx, by, framebuffer, green);

	framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);
	
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
