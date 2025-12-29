#include "tgaimage.hpp"
#include "math.hpp"
//#include "objreader.hpp"

#include <numbers>
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
Vec3<T> rotate(Vec3<T> &vec_to_rotate){
  float a = std::numbers::pi_v<float>/6;
  Matrix<T, 3,3> Ry({std::cos(a), 0, std::sin(a),
	                           0,       1,      0,
				    		-std::sin(a), 0, std::cos(a)});
  return Ry*vec_to_rotate;
}

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

Vec3<float> viewport_transform(const Vec3<float> &vec_to_viewport_transform, const TGAImage &img)
{
  const int half_height = img.height() / 2;
  const int half_width =  img.width()  / 2;
  const float half_z = static_cast<float>(UINT8_MAX) / 2;
  float x_updated = (vec_to_viewport_transform.x() + static_cast<float>(1)) * half_width;
  float y_updated = (vec_to_viewport_transform.y() + static_cast<float>(1)) * half_height;
  float z_updated = std::round((vec_to_viewport_transform.z() + 1.0F) * half_z);
  return Vec3(x_updated, y_updated, z_updated);
}

Matrix<float, 4, 4> viewport_matrix(const int x, const int y, const int w, const int h){
  return {static_cast<float>(w/2), 0, 0, x + static_cast<float>(w/2),
		  0, static_cast<float>(h/2), 0, y + static_cast<float>(h/2),
		  0, 0, 1, 0,
		  0, 0, 0, 1};
}

Vec3<float> central_projection_transform(const Vec3<float> &vec_to_viewport_transform) {
  constexpr float camera_dist = 2.5F;
  auto z_coord = static_cast<float>(vec_to_viewport_transform.z());
  return vec_to_viewport_transform * (1.0F / (1.0F - (z_coord/camera_dist)));
}

Matrix<float, 4, 4> perspective_matrix(const float focal_len){
  return {1, 0, 0, 0,
		  0, 1, 0, 0,
		  0, 0, 1, 0,
		  0, 0, -(1/focal_len), 1};
}

Matrix<float, 4, 4> modelview_matrix(const Vec3<float> eye, const Vec3<float> center, const Vec3<float> up){
  Vec3<float> eye_center_diff = eye - center;
  Vec3<float> n_basis         = eye_center_diff * (1.0F/eye_center_diff.mag());

  Vec3<float> up_n_cross = up ^ n_basis;
  Vec3<float> l_basis    = up_n_cross * (1.0F/up_n_cross.mag());

  Vec3<float> n_l_cross  = n_basis ^ l_basis;
  Vec3<float> m_basis    = n_l_cross * (1.0F/n_l_cross.mag());

  Matrix<float, 4, 4> MT{l_basis.x(), l_basis.y(), l_basis.z(), 0,
						 m_basis.x(), m_basis.y(), m_basis.z(), 0,
						 n_basis.x(), n_basis.y(), n_basis.z(), 0,
						 0,           0,           0,           1};

  Matrix<float, 4, 4>  C{1, 0, 0, -center.x(),
						 0, 1, 0, -center.y(),
						 0, 0, 1, -center.z(),
						 0, 0, 0,           1};

  return MT * C;
  
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

template<typename T> void draw_triangles(TGAImage &img, const OBJObject<T> &obj, std::vector<float> &zbuffer, Matrix<float, 4, 4> &mdl_mat, Matrix<float, 4, 4> &persp_mat, Matrix<float, 4, 4> &vprt_mat)
{

  std::random_device rand_colors_distribution("default");
  
  for (const auto &faces : obj.faces) {
    auto tri1_index = static_cast<size_t>(faces.face_vertices.at(0) - 1);
    auto tri2_index = static_cast<size_t>(faces.face_vertices.at(1) - 1);
    auto tri3_index = static_cast<size_t>(faces.face_vertices.at(2) - 1);

	Vec3<float> tri1(obj.vertices.at(tri1_index).get_x(),
				   obj.vertices.at(tri1_index).get_y(),
				   obj.vertices.at(tri1_index).get_z());

	Vec3<float> tri2(obj.vertices.at(tri2_index).get_x(),
				   obj.vertices.at(tri2_index).get_y(),
				   obj.vertices.at(tri2_index).get_z());

	Vec3<float> tri3(obj.vertices.at(tri3_index).get_x(),
				   obj.vertices.at(tri3_index).get_y(),
				   obj.vertices.at(tri3_index).get_z());

	///TODO:
	// took this step by step, need to clean it up

	Vec4<float> tri1_m = persp_mat * mdl_mat * Vec4<float>(tri1.x(), tri1.y(), tri1.z(), 1.0F);
	Vec4<float> tri2_m = persp_mat * mdl_mat * Vec4<float>(tri2.x(), tri2.y(), tri2.z(), 1.0F);
	Vec4<float> tri3_m = persp_mat * mdl_mat * Vec4<float>(tri3.x(), tri3.y(), tri3.z(), 1.0F);

	tri1_m = tri1_m * (1.0F/tri1_m.w());
	tri2_m = tri2_m * (1.0F/tri2_m.w());
	tri3_m = tri3_m * (1.0F/tri3_m.w());

	auto tri1_screen = vprt_mat * tri1_m;
	auto tri2_screen = vprt_mat * tri2_m;
	auto tri3_screen = vprt_mat * tri3_m;

    fill_triangle_zbuffer(std::round(tri1_screen.x()),
						  std::round(tri1_screen.y()),
						  std::round(tri1_screen.z()),
						  std::round(tri2_screen.x()),
						  std::round(tri2_screen.y()),
						  std::round(tri2_screen.z()),
						  std::round(tri3_screen.x()),
						  std::round(tri3_screen.y()),
						  std::round(tri3_screen.z()),						  
						  img,
						  zbuffer,
						  rand_colors_distribution
						  );
		
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
						   std::vector<float> &zbuffer,
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
	  auto z_val = static_cast<float>((lam1 * az) + (lam2 * bz) + (lam3 * cz));
	  size_t z_index = (i*800) + j;        /// TODO MAGIC NUM
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F && sarea_total>1.0F) {
		if(zbuffer.at(z_index) < z_val){
		  zbuffer.at(z_index) =  z_val;
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

  Vec3<float>    eye{ 300.0F , 300.0F, 300.0F};  // camera position
  Vec3<float> center{ 0.0F , 0.0F ,0.0F};  // camera direction
  Vec3<float>     up{ 0.0F , 1.0F, 0.0F};  // camera up vector
 
  constexpr int width  = 800;
  constexpr int height = 800;
  TGAImage diablo_fb(width, height, TGAImage::RGB);
  std::vector<float> diablo_zbfr (width*height, -1000.0F);

  OBJObject<float> face {};
  read_obj("assets/diablo3_pose.obj", face);

  bool is_running = true;
  std::string command_str;
  
  std::cout << "enter command:" << std::endl;
  while (std::getline(std::cin, command_str)) {


	
    if ("step" == command_str) {
      std::cout << "stepping forward...\n";
	  diablo_fb.reset(TGAColor(177, 195, 209, 255));
      auto MV_matrix = modelview_matrix(eye, center, up);
      auto P_matrix = perspective_matrix((eye - center).mag());
      auto VP_matrix = viewport_matrix(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

	  //reset z_buffer
	  std::fill(diablo_zbfr.begin(), diablo_zbfr.end(), -1000.0F);
      draw_triangles(diablo_fb, face, diablo_zbfr, MV_matrix, P_matrix, VP_matrix);

      diablo_fb.write_tga_file("diablo_img_ztest.tga");
	  std::cout << "enter command:" << std::endl;
    }

	else if ("ex+" == command_str) {
	  eye.increment(0);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("ex-" == command_str) {
	  eye.decrement(0);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("ey+" == command_str) {
	  eye.increment(1);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("ey-" == command_str) {
	  eye.decrement(1);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("ez+" == command_str) {
	  eye.increment(2);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("ez-" == command_str) {
	  eye.decrement(2);
	  std::cout << "eye: ";
	  eye.print();
	}

	else if ("cx+" == command_str) {
	  center.tiny_increment(0);
	  std::cout << "center:";
	  center.print();
	}

	else if ("cx-" == command_str) {
	  center.tiny_decrement(0);
	  std::cout << "center:";
	  center.print();
	}

	else if ("cy+" == command_str) {
	  center.tiny_increment(1);
	  std::cout << "center:";
	  center.print();
	}

	else if ("cy-" == command_str) {
	  center.tiny_decrement(1);
	  std::cout << "center:";
	  center.print();
	}

	else if ("cz+" == command_str) {
	  center.tiny_increment(2);
	  std::cout << "center:";
	  center.print();
	}

	else if ("cz-" == command_str) {
	  center.tiny_decrement(2);
	  std::cout << "center:";
	  center.print();
	}

    else if ("exit" == command_str) {
      break;
    } else {
      std::cout << "unknown command" << std::endl;
	  std::cout << "enter command:" << std::endl;
    }
  }

  return 0;
}
