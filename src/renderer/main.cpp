#include "tgaimage.hpp"
//#include "objreader.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <print>

template <typename T>
class OBJVertex
{
public:
  std::array<T, 3> vertex_coords;
  OBJVertex() = default;
  OBJVertex(const T x_, const T y_, const T z_) : vertex_coords({ x_, y_, z_ }) {};

  [[nodiscard]] T get_x() const { return vertex_coords.at(0); }
  [[nodiscard]] T get_y() const { return vertex_coords.at(1); }
  [[nodiscard]] T get_z() const { return vertex_coords.at(2); }  

  void print() const {std::print("x = {0}, y = {1}, z = {2}\n", vertex_coords.at(0), vertex_coords.at(1), vertex_coords.at(2));}
};

class OBJFaceElements
{
public:
  std::array<int,3> face_vertices {};
  OBJFaceElements() = default;
  OBJFaceElements(const int tri1, const int tri2, const int tri3)
  {
    face_vertices.at(0) = tri1;
    face_vertices.at(1) = tri2;
    face_vertices.at(2) = tri3;    
    }
  void print() const {std::print("tri1 = {0}, tri2 = {1}, tri3 = {2}\n", face_vertices.at(0), face_vertices.at(1), face_vertices.at(2));}  
};

template <typename T>
class OBJObject
{
public:
  std::vector<OBJVertex<T>> vertices;
  std::vector<OBJFaceElements> faces;

  OBJObject() = default;
  
  void printVertices() const {
	for (const auto& vertex : vertices){
	  vertex.print();
	  }
  }

  void printFaces() const
  {
    for (const auto &face : faces) { face.print(); }
  }    
};

template <typename T>
bool read_obj(const std::filesystem::path &obj_file_path, OBJObject<T>& obj_object)
{
  std::ifstream obj_file_stream(obj_file_path);
  if (!obj_file_stream.is_open()) { return false; }
  std::string line {};
  std::string symbol {};  
  std::stringstream line_stream{};
  while (std::getline(obj_file_stream, line, '\n')) {
	line_stream.clear();
	line_stream.str(line);
	const size_t symbol_end_index = line.find_first_of(' ');
	if (!(std::string::npos == symbol_end_index)) {
	    symbol = line.substr(0, symbol_end_index);
		line_stream.seekg(symbol_end_index);
	}
	if ("v"==symbol) {
	  T x_coord {};
	  T y_coord {};
	  T z_coord {};
	  line_stream >> x_coord >> y_coord >> z_coord;
	  obj_object.vertices.emplace_back(x_coord,y_coord,z_coord);
	}
	else if("f" == symbol){
	  int tri1 {};
	  int tri2 {};
	  int tri3 {};
	  std::string trig_data = line.substr(symbol_end_index);
	  std::stringstream trig_data_stream(trig_data);
	  trig_data_stream >> tri1;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
	  trig_data_stream >> tri2;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
	  trig_data_stream >> tri3;
	  trig_data_stream.clear();
      //std::print("tri1 = {0}, tri2 = {1}, tri3 = {2}\n", tri1, tri2, tri3);
	  obj_object.faces.emplace_back(tri1, tri2, tri3);
	}
  }    
  obj_file_stream.close();
  return true;
}

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

template<typename T> void draw_triangles(TGAImage &img, const OBJObject<T> &obj, const TGAColor &color)
{

  for (const auto &faces : obj.faces) {
    auto tri1_index = static_cast<size_t>(faces.face_vertices.at(0) - 1);
    auto tri2_index = static_cast<size_t>(faces.face_vertices.at(1) - 1);
    auto tri3_index = static_cast<size_t>(faces.face_vertices.at(2) - 1);

    std::print("drawing indices: {0}, {1}, {2}\n", tri1_index, tri2_index, tri3_index);
    
	draw_line3(obj.vertices.at(tri1_index).get_x(),
					   obj.vertices.at(tri1_index).get_y(),
					   obj.vertices.at(tri2_index).get_x(),
			   obj.vertices.at(tri2_index).get_y(),img,color);

	draw_line3(obj.vertices.at(tri2_index).get_x(),
					   obj.vertices.at(tri2_index).get_y(),
					   obj.vertices.at(tri3_index).get_x(),
			   obj.vertices.at(tri3_index).get_y(),img,color);

	draw_line3(obj.vertices.at(tri1_index).get_x(),
          obj.vertices.at(tri1_index).get_y(),
          obj.vertices.at(tri3_index).get_x(),
			   obj.vertices.at(tri3_index).get_y(),img, color);
		
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
  //draw_line3(ax, ay, bx, by, framebuffer, blue);
  //draw_line3(bx, by, ax, ay, framebuffer, yellow);
  //draw_line3(cx, cy, bx, by, framebuffer, green);
  //draw_line3(ax, ay, cx, cy, framebuffer, red);
  //draw_line3(cx, cy, ax, ay, framebuffer, yellow);
  //draw_line3(ax, ay, bx, by, framebuffer, green);
  

  OBJObject<float> diablo_pose {};
  read_obj("assets/diablo3_pose.obj", diablo_pose);
  // diablo_pose.printVertices();
  // diablo_pose.printFaces();

  OBJObject<int> triangles{};
  triangles.vertices.emplace_back(ax, ay, 0); 
  triangles.vertices.emplace_back(bx, by, 0);
  triangles.vertices.emplace_back(cx, cy, 0);
  triangles.faces.emplace_back(1, 2, 3);
  triangles.printFaces();
  triangles.printVertices();

  draw_triangles(framebuffer, triangles, blue);

  framebuffer.set(ax, ay, white);
  framebuffer.set(bx, by, white);
  framebuffer.set(cx, cy, white);
  
	
  framebuffer.write_tga_file("framebuffer.tga");
  return 0;
}
