#include "tgaimage.hpp"
#include "math.hpp"
//#include "objreader.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <print>
#include <cstdlib>
#include <random>
#include <limits>

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

  void viewport_transform(const TGAImage &img)
  {
    const int half_height = img.height() / 2;
	const int half_width =  img.width()  / 2;
	const float half_z = static_cast<float>(UINT8_MAX) / 2;
	for (auto &vert : vertices) {
	   auto current_x = vert.get_x();
	   auto current_y = vert.get_y();
	   auto current_z = static_cast<float>(vert.get_z());
	   vert.vertex_coords.at(0) = (current_x + static_cast<T>(1)) * half_width;
	   vert.vertex_coords.at(1) = (current_y + static_cast<T>(1)) * half_height;
	   vert.vertex_coords.at(2) = std::round((current_z + 1.0F) * half_z);
	}
	  
  }

};

template<typename T>
class Rectangle{
private:
  std::array<T, 4> vertices;
public:
  Rectangle (T x_min, T x_max, T y_min, T y_max) : vertices({x_min, x_max, y_min, y_max}) {}
  [[nodiscard]] T& get_xmin() {return vertices.at(0);}
  [[nodiscard]] T& get_xmax() {return vertices.at(1);}
  [[nodiscard]] T& get_ymin() {return vertices.at(2);}  
  [[nodiscard]] T& get_ymax() {return vertices.at(3);}

  [[nodiscard]] T& get_xmin() const {return vertices.at(0);}
  [[nodiscard]] T& get_xmax() const {return vertices.at(1);}
  [[nodiscard]] T& get_ymin() const {return vertices.at(2);}  
  [[nodiscard]] T& get_ymax() const {return vertices.at(3);}
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
	  const std::string trig_data = line.substr(symbol_end_index);
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
  
  if(std::abs(slope) > 1.0F){
	std::swap(sx, sy);
	std::swap(dx, dy);
	switched = true;
  }

    if (sx > dx) {
    std::swap(sx, dx);
    std::swap(sy, dy);
  }
  
  for (int x_t = sx; x_t <= dx; x_t++) {
    const float t_param = static_cast<float>(x_t - sx) / static_cast<float>(dx - sx);
	const float dy_to_draw = std::round(t_param * static_cast<float>(dy - sy));
    const int y_to_draw = sy + static_cast<int>(dy_to_draw);
    if (switched) {
      img.set(y_to_draw, x_t, color);
    } else {
      img.set(x_t, y_to_draw, color);
    }
  }
}

template<typename T> void draw_triangles(TGAImage &img, const OBJObject<T> &obj, const TGAColor &color, TGAImage& zbuffer)
{

  std::random_device rand_colors_distribution("default");
  
  for (const auto &faces : obj.faces) {
    auto tri1_index = static_cast<size_t>(faces.face_vertices.at(0) - 1);
    auto tri2_index = static_cast<size_t>(faces.face_vertices.at(1) - 1);
    auto tri3_index = static_cast<size_t>(faces.face_vertices.at(2) - 1);
	

    fill_triangle_zbuffer(obj.vertices.at(tri1_index).get_x(),
						  obj.vertices.at(tri1_index).get_y(),
						  obj.vertices.at(tri1_index).get_z(),
						  obj.vertices.at(tri2_index).get_x(),
						  obj.vertices.at(tri2_index).get_y(),
						  obj.vertices.at(tri2_index).get_z(),						  
						  obj.vertices.at(tri3_index).get_x(),
						  obj.vertices.at(tri3_index).get_y(),
						  obj.vertices.at(tri3_index).get_z(),						  
						  img,
						  zbuffer,
						  rand_colors_distribution
						  );

    //std::print("drawing indices: {0}, {1}, {2}\n", tri1_index, tri2_index, tri3_index);
    
	//draw_line3(obj.vertices.at(tri1_index).get_x(),
	//		   obj.vertices.at(tri1_index).get_y(),
	//		   obj.vertices.at(tri2_index).get_x(),
	//		   obj.vertices.at(tri2_index).get_y(),img,color);
	//
	//draw_line3(obj.vertices.at(tri2_index).get_x(),
	//		   obj.vertices.at(tri2_index).get_y(),
	//		   obj.vertices.at(tri3_index).get_x(),
	//		   obj.vertices.at(tri3_index).get_y(),img,color);
	//
	//draw_line3(obj.vertices.at(tri1_index).get_x(),
	//		   obj.vertices.at(tri1_index).get_y(),
	//		   obj.vertices.at(tri3_index).get_x(),
	//		   obj.vertices.at(tri3_index).get_y(),img, color);
		
  }
}

void draw_triangle(const int ax,
  const int ay,
  const int bx,
  const int by,
  const int cx,
  const int cy,
  TGAImage &img,
  const TGAColor &clr)
{
  draw_line3(ax, ay, bx, by, img, clr);
  draw_line3(ax, ay, cx, cy, img, clr);
  draw_line3(cx, cy, bx, by, img, clr);
}



float s_triangle_area(const int ax, const int ay, const int bx, const int by, const int cx, const int cy)
{
  // equation for triangle area with vertex coordinates
  return 0.5F * static_cast<float>(((ax - cx)*(by - ay)) - ((ax - bx)*(cy - ay)));
}

template<typename T>
Rectangle<T> get_bounding_box(int ax, int ay, int bx, int by, int cx, int cy){
  T x_min = std::min({ax, bx, cx});
  T y_min = std::min({ay, by, cy});
  T x_max = std::max({ax, bx, cx});
  T y_max = std::max({ay, by, cy});  
  return Rectangle(x_min, x_max, y_min, y_max);
}

void fill_triangle_shader(const int ax,
				   const int ay,
				   const int az,
				   const int bx,
				   const int by,
				   const int bz,
				   const int cx,
				   const int cy,
				   const int cz,
				   TGAImage &img)
{
  float sarea_total = s_triangle_area(ax, ay, bx, by, cx, cy);
  Rectangle<int> bounding_box = get_bounding_box<int>(ax, ay, bx, by, cx, cy);
  for (int i = bounding_box.get_xmin(); i <= bounding_box.get_xmax(); ++i) {
    for (int j = bounding_box.get_ymin(); j <= bounding_box.get_ymax(); ++j) {
      // img.set(i,j,clr);
	  /// TODO: clean dis shi up
      float sareaPBC = s_triangle_area(i, j, bx, by, cx, cy);
      float sareaAPC = s_triangle_area(ax, ay, i, j, cx, cy);
      float sareaABP = s_triangle_area(ax, ay, bx, by, i, j);
      float lam1 = sareaPBC / sarea_total;
      float lam2 = sareaAPC / sarea_total;
      float lam3 = sareaABP / sarea_total;
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F) {
		uint8_t red_value = lam1 * 255;
		uint8_t blue_value = lam2 * 255;
		uint8_t green_value = lam3 * 255;
		TGAColor shade(red_value, blue_value, green_value, 255);
		img.set(i,j,shade);
	  }
	}
  }
}

void fill_triangle(const int ax,
				   const int ay,
				   const int bx,
				   const int by,
				   const int cx,
				   const int cy,
				   TGAImage &img,
				   const TGAColor &clr)
{
  float sarea_total = s_triangle_area(ax, ay, bx, by, cx, cy);
  Rectangle<int> bounding_box = get_bounding_box<int>(ax, ay, bx, by, cx, cy);
  for (int i = bounding_box.get_xmin(); i <= bounding_box.get_xmax(); ++i) {
    for (int j = bounding_box.get_ymin(); j <= bounding_box.get_ymax(); ++j) {
      // img.set(i,j,clr);
	  /// TODO: clean dis shi up
	  auto rand_r = static_cast<uint8_t>(std::rand() % UINT8_MAX);
	  auto rand_g = static_cast<uint8_t>(std::rand() % UINT8_MAX);
	  auto rand_b = static_cast<uint8_t>(std::rand() % UINT8_MAX);
    
	  TGAColor rndColor(rand_r, rand_g, rand_b, 255);
	  
      float sareaPBC = s_triangle_area(i, j, bx, by, cx, cy);
      float sareaAPC = s_triangle_area(ax, ay, i, j, cx, cy);
      float sareaABP = s_triangle_area(ax, ay, bx, by, i, j);
      float lam1 = sareaPBC / sarea_total;
      float lam2 = sareaAPC / sarea_total;
      float lam3 = sareaABP / sarea_total;
	  
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F) {
		img.set(i,j,rndColor);
	  }
	}
  }
}

void fill_triangle_zbuffer(const int ax,
						   const int ay,
						   const int az,
						   const int bx,
						   const int by,
						   const int bz,
						   const int cx,
						   const int cy,
						   const int cz,
						   TGAImage &img,
						   TGAImage &zbuffer,
						   std::random_device &ran_dev)
{

  auto rand_r = ran_dev() % UINT8_MAX;
  auto rand_g = ran_dev() % UINT8_MAX;
  auto rand_b = ran_dev() % UINT8_MAX;
    
  TGAColor rndColor(rand_r, rand_g, rand_b, UINT8_MAX);
  
  float sarea_total = s_triangle_area(ax, ay, bx, by, cx, cy);
  
  Rectangle<int> bounding_box = get_bounding_box<int>(ax, ay, bx, by, cx, cy);
  for (int i = bounding_box.get_xmin(); i <= bounding_box.get_xmax(); ++i) {
    for (int j = bounding_box.get_ymin(); j <= bounding_box.get_ymax(); ++j) {
	  /// TODO: clean dis shi up
      float sareaPBC = s_triangle_area(i, j, bx, by, cx, cy);
      float sareaAPC = s_triangle_area(ax, ay, i, j, cx, cy);
      float sareaABP = s_triangle_area(ax, ay, bx, by, i, j);
      float lam1 = sareaPBC / sarea_total;
      float lam2 = sareaAPC / sarea_total;
      float lam3 = sareaABP / sarea_total;
	  auto z_val = static_cast<uint8_t>((lam1 * az) + (lam2 * bz) + (lam3 * cz));
	  //uint8_t z_val = std::round((az + bz + cz) / 3);
	  TGAColor z_color(z_val, z_val, z_val, UINT8_MAX);      
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F) {
		if(zbuffer.get(i,j)[0] < z_val){
		  zbuffer.set(i, j, z_color);
		  img.set(i, j, rndColor);
		}
	  }
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
  TGAImage framebuffer_z(width, height, TGAImage::GRAYSCALE);
  TGAImage diablo_fb(800, 800, TGAImage::RGB);
  TGAImage diablo_fb_z(800, 800, TGAImage::GRAYSCALE);  

  constexpr int ax = 17, ay =  4, az =  255;
  constexpr int bx = 55, by = 39, bz = 255;
  constexpr int cx = 23, cy = 59, cz = 255;

  fill_triangle_shader(ax, ay, az,
				bx, by, bz,
				cx, cy, cz,
				framebuffer);

  //framebuffer.write_tga_file("shaded_triangle.tga");

  //OBJObject<int> triangle_filling;
  //draw_triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
  //draw_triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
  //draw_triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
  //
  //
  //draw_line3(45, 110, 120, 35, framebuffer, yellow);
  //framebuffer.write_tga_file("triangles.tga");

  //fill_triangle(  7, 45, 35, 100, 45,  60, framebuffer, red);
  //fill_triangle(120, 35, 90,   5, 45, 110, framebuffer, white);
  //fill_triangle(115, 83, 80, 90, 85, 120, framebuffer, green);
  //
  //framebuffer.write_tga_file("filled_triangles.tga");  

  //points
  //constexpr int ax =  7;
  //constexpr int ay =  3;
  //	
  //constexpr int bx = 12;
  //constexpr int by = 37;
  //	
  //constexpr int cx = 62;
  //constexpr int cy = 53;
  //
  //
  //OBJObject<int> triangles{};
  //triangles.vertices.emplace_back(ax, ay, 0); 
  //triangles.vertices.emplace_back(bx, by, 0);
  //triangles.vertices.emplace_back(cx, cy, 0);
  //triangles.faces.emplace_back(1, 2, 3);
  //
  //draw_triangles(framebuffer, triangles, blue);
  //fill_triangles(framebuffer, triangles, green);
  //
  //framebuffer.set(ax, ay, white);
  //framebuffer.set(bx, by, white);
  //framebuffer.set(cx, cy, white);
  //
  //framebuffer.write_tga_file("filling_test.tga");


  OBJObject<float> diablo_pose {};
  read_obj("assets/diablo3_pose.obj", diablo_pose);
  diablo_pose.viewport_transform(diablo_fb);
  diablo_pose.printVertices();
  draw_triangles(diablo_fb, diablo_pose, red, diablo_fb_z);

  diablo_fb.write_tga_file("diablo_img.tga");
  diablo_fb_z.write_tga_file("diablo_img_z.tga");

  Vec2<float> vec1(1.0F,2.0F);
  Vec2<float> vec2(3.0F,4.0F);
  float result = vec1&vec2;
  std::cout << result << std::endl;

  Matrix<int, 2, 2> TwoByTwo {1,2,3,4};
  Matrix<int, 2, 2> OtherTwoByTwo {5,6,7,8};
  
  std::cout << "matrix1:\n";
  TwoByTwo.print();
  
  std::cout << "matrix2:\n";
  OtherTwoByTwo.print();
  std::cout << std::endl;
  
  Matrix<int, 2,2> matrix_sum = TwoByTwo + OtherTwoByTwo;
  std::cout << "result:\n";
  matrix_sum.print();

  std::cout << std::endl;
  Matrix<int, 4, 4> three_matrix {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  three_matrix.Tr();
  three_matrix.print();
  
  return 0;
}
