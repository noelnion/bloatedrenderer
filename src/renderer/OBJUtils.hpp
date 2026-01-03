#ifndef OBJUTILS_HPP
#define OBJUTILS_HPP

#include <iostream>
#include <string>
#include <sstream>
#include "math.hpp"

template <typename T>
class OBJVertex
{
public:
  std::array<T, 3> vertex_coords;
  OBJVertex() = default;
  OBJVertex(const T x_, const T y_, const T z_) : vertex_coords({ x_, y_, z_ }) {};

  [[nodiscard]] Vec3<T> get_vec() const {return {get_x(), get_y(), get_z()};}
  [[nodiscard]] T get_x() const { return vertex_coords.at(0); }
  [[nodiscard]] T get_y() const { return vertex_coords.at(1); }
  [[nodiscard]] T get_z() const { return vertex_coords.at(2); }

  void print() const {std::print("x = {0}, y = {1}, z = {2}\n", vertex_coords.at(0), vertex_coords.at(1), vertex_coords.at(2));}
};

class OBJNormal{
public:
  std::array<float, 3> normal_comps {};
public:  
  OBJNormal() = default;
  OBJNormal(const float x_, const float y_, const float z_) : normal_comps({ x_, y_, z_ }) {};
  [[nodiscard]] Vec3<float> get_vec() const {return {get_x(), get_y(), get_z()};}
  [[nodiscard]] float get_x() const { return normal_comps.at(0); }
  [[nodiscard]] float get_y() const { return normal_comps.at(1); }
  [[nodiscard]] float get_z() const { return normal_comps.at(2); }
};

class OBJTexCords{
public:
  std::array<float, 2> texture_coords {};
public:  
  OBJTexCords() = default;
  OBJTexCords(const float x_, const float y_) : texture_coords({ x_, y_}) {};
  [[nodiscard]] Vec2<float> get_vec() const {return {get_x(), get_y()};}  
  [[nodiscard]] float get_x() const { return texture_coords.at(0); }
  [[nodiscard]] float get_y() const { return texture_coords.at(1); }
};

class OBJFaceElements
{
public:
  std::array<int,3> face_vertices {};
  std::array<int, 3> vert_normals {};
  std::array<int, 3> vert_texcoords {};
  
  OBJFaceElements() = default;
  void set_vertices(const int t_v1, const int t_v2, const int t_v3) {face_vertices = {t_v1, t_v2, t_v3};}
  void set_normals(const int t_n1, const int t_n2, const int t_n3) {vert_normals = {t_n1, t_n2, t_n3};}
  void set_tc(const int t_tc1, const int t_tc2, const int t_tc3) {vert_texcoords = {t_tc1, t_tc2, t_tc3};}

  void print() const {std::print("tri1 = {0}, tri2 = {1}, tri3 = {2}\n", face_vertices.at(0), face_vertices.at(1), face_vertices.at(2));}

};

template <typename T>
class OBJObject
{
public:
  std::vector<OBJVertex<T>> vertices;
  std::vector<OBJFaceElements> faces;
  std::vector<OBJNormal> normals;
  std::vector<OBJTexCords> tex_coords;

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
  std::cout << "reading obj" << std::endl;
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

	  int vt_1 {};
	  int vt_2 {};
	  int vt_3 {};

	  int vn_1 {};
	  int vn_2 {};
	  int vn_3 {};
	  
	  const std::string trig_data = line.substr(symbol_end_index);
	  std::stringstream trig_data_stream(trig_data);
	  trig_data_stream >> tri1;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  
	  trig_data_stream >> vt_1;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  
	  trig_data_stream >> vn_1;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
	  trig_data_stream >> tri2;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  	  
	  trig_data_stream >> vt_2;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  	  
	  trig_data_stream >> vn_2;	  
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
	  trig_data_stream >> tri3;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  	  
	  trig_data_stream >> vt_3;
	  trig_data_stream.ignore(std::numeric_limits<std::streamsize>::max(), '/');	  	  
	  trig_data_stream >> vn_3;	  
	  trig_data_stream.clear();
      //std::print("tri1 = {0}, tri2 = {1}, tri3 = {2}\n", tri1, tri2, tri3);
	  OBJFaceElements face_in {};
	  face_in.set_vertices(tri1, tri2, tri3);
	  face_in.set_tc(vt_1, vt_2, vt_3);
	  face_in.set_normals(vn_1, vn_2, vn_3);
	  obj_object.faces.push_back(face_in);
	}
	else if("vn" == symbol){
	  float x_comp {};
	  float y_comp {};
	  float z_comp {};
	  line_stream >> x_comp >> y_comp >> z_comp;
	  obj_object.normals.emplace_back(x_comp, y_comp, z_comp);
	}
	else if("vt" == symbol){
	  float x_tex {};
	  float y_tex {};
	  line_stream >> x_tex >> y_tex;
	  obj_object.tex_coords.emplace_back(x_tex, y_tex);
	}
  }    
  obj_file_stream.close();
  return true;
}

#endif // OBJUTILS_HPP
