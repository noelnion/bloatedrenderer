#ifndef OPER_TR_HPP
#define OPEN_TR_HPP

#include "math.hpp"
#include "tgaimage.hpp"

#include <utility>

float s_triangle_area(const int ax, const int ay, const int bx, const int by, const int cx, const int cy)
{
  // equation for triangle area with vertex coordinates
  return 0.5F * static_cast<float>(((ax - cx)*(by - ay)) - ((ax - bx)*(cy - ay)));
}

class VertexOut{
public:
  Vec4<float> screen_pos;
  VertexOut(Vec4<float> t_scrn_pos) : screen_pos(t_scrn_pos) {}
};

using Primitive = std::array<VertexOut, 3>;
using FaceTriangles = std::array<Vec3<float>, 3>;

class FacesStream{
private:
  OBJObject<float> &obj;
  std::size_t index {};

public:

  FacesStream(OBJObject<float> &t_obj) : obj(t_obj) {}
  
  bool getFace(FaceTriangles& out){
	if(index >= obj.faces.size()){
	  return false;
	}

	std::size_t tri1_idx = obj.faces.at(index).face_vertices.at(0) - 1;
	std::size_t tri2_idx = obj.faces.at(index).face_vertices.at(1) - 1;
	std::size_t tri3_idx = obj.faces.at(index).face_vertices.at(2) - 1;

	out.at(0) = {obj.vertices.at(tri1_idx).get_x(),
	             obj.vertices.at(tri1_idx).get_y(),
	             obj.vertices.at(tri1_idx).get_z()};
	
	out.at(1) = {obj.vertices.at(tri2_idx).get_x(),
	             obj.vertices.at(tri2_idx).get_y(),
	             obj.vertices.at(tri2_idx).get_z()};
	
	out.at(2) = {obj.vertices.at(tri3_idx).get_x(),
	             obj.vertices.at(tri3_idx).get_y(),
	             obj.vertices.at(tri3_idx).get_z()};

	index++;
	return true;
	
	
  }

  
};

Primitive vertex_shader(FaceTriangles ftriag, Matrix<float, 4, 4> &mdl_mat, Matrix<float, 4, 4> &persp_mat, Matrix<float, 4, 4> &vprt_mat){

    auto& [tri1, tri2, tri3] = ftriag;

	Vec4<float> tri1_m = persp_mat * mdl_mat * Vec4<float>(tri1.x(), tri1.y(), tri1.z(), 1.0F);
	Vec4<float> tri2_m = persp_mat * mdl_mat * Vec4<float>(tri2.x(), tri2.y(), tri2.z(), 1.0F);
	Vec4<float> tri3_m = persp_mat * mdl_mat * Vec4<float>(tri3.x(), tri3.y(), tri3.z(), 1.0F);

	tri1_m = tri1_m * (1.0F/tri1_m.w());
	tri2_m = tri2_m * (1.0F/tri2_m.w());
	tri3_m = tri3_m * (1.0F/tri3_m.w());

	auto tri1_screen = vprt_mat * tri1_m;
	auto tri2_screen = vprt_mat * tri2_m;
	auto tri3_screen = vprt_mat * tri3_m;

	return {VertexOut(tri1_screen), VertexOut(tri2_screen), VertexOut(tri3_screen)};
  }


//template <typename FragFunc>
void rasterize(Primitive &primitive,
			   TGAImage &img,
			   std::vector<float> &zbuffer)
{

  auto& [ax, ay, az, aw] = primitive.at(0).screen_pos.vector;
  auto& [bx, by, bz, bw] = primitive.at(1).screen_pos.vector;
  auto& [cx, cy, cz, cw] = primitive.at(2).screen_pos.vector;
  
  float sarea_total = s_triangle_area(ax, ay, bx, by, cx, cy);
  
  auto[box_x_min, box_x_max] = std::minmax({ax, bx, cx});
  auto[box_y_min, box_y_max] = std::minmax({ay, by, cy});
  
  for (int i = box_x_min; i <= box_x_max; ++i) {
    for (int j = box_y_min; j <= box_y_max; ++j) {

      float sareaPBC = s_triangle_area(i, j, bx, by, cx, cy);
      float sareaAPC = s_triangle_area(ax, ay, i, j, cx, cy);
      float sareaABP = s_triangle_area(ax, ay, bx, by, i, j);

	  //interpolate
      float lam1 = sareaPBC / sarea_total;
      float lam2 = sareaAPC / sarea_total;
      float lam3 = sareaABP / sarea_total;
	  auto z_val = static_cast<float>((lam1 * az) + (lam2 * bz) + (lam3 * cz));
	  
	  size_t z_index = (i*img.width()) + j;
	  
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F && sarea_total>1.0F) {
		if(zbuffer.at(z_index) < z_val){
		  TGAColor pixel_color(150,150,150,255);
		  zbuffer.at(z_index) =  z_val;
		  img.set(i, j, pixel_color);
		}
	  }
	  
	}
  }
}

Matrix<float, 4, 4> viewport_matrix(const int x, const int y, const int w, const int h){
  return {static_cast<float>(w/2), 0, 0, x + static_cast<float>(w/2),
		  0, static_cast<float>(h/2), 0, y + static_cast<float>(h/2),
		  0, 0, 1, 0,
		  0, 0, 0, 1};
}

Matrix<float, 4, 4> perspective_matrix(const float focal_len){
  return {1, 0, 0, 0,
		  0, 1, 0, 0,
		  0, 0, 1, 0,
		  0, 0, -(1/focal_len), 1};
}

Matrix<float, 4, 4> lookat(const Vec3<float> eye, const Vec3<float> center, const Vec3<float> up){
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




class IShader {
  virtual std::pair<bool,TGAColor> fragment(const Vec3<float> bar) const = delete;
};

#endif // OPEN_TR_HPP
