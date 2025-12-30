#ifndef OBJUTILS_HPP
#define OBJUTILS_HPP

#include <string>
#include <sstream>

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

#endif // OBJUTILS_HPP
