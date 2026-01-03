#ifndef OPER_TR_HPP
#define OPEN_TR_HPP

#include <optional>
#include <algorithm>
#include <iostream>

#include "math.hpp"
#include "tgaimage.hpp"


float s_triangle_area(const int ax, const int ay, const int bx, const int by, const int cx, const int cy)
{
  // equation for triangle area with vertex coordinates
  return 0.5F * static_cast<float>(((ax - cx)*(by - ay)) - ((ax - bx)*(cy - ay)));
}

Matrix<float, 4, 4> viewport_matrix(const int x, const int y, const int w, const int h){
  return {static_cast<float>(w/2), 0, 0, x + static_cast<float>(w/2),
		  0, static_cast<float>(h/2), 0, y + static_cast<float>(h/2),
		  0, 0, 1, 0,
		  0, 0, 0, 1};
}

class VertexIn{
public:
  Vec3<float> normal;
  Vec3<float> pos;
  Vec2<float> uv;
  
  VertexIn() = default;

  void set_normal(Vec3<float> &t_normal){
	normal.vector = t_normal.vector;
  }

  void set_pos(Vec3<float> &t_pos){
	pos.vector = t_pos.vector;
  }

  void set_uv(Vec2<float> &t_uv){
	uv.vector = t_uv.vector;
  }
  
};

class VertexOut{
public:
  Vec4<float> clip_pos;
  Vec3<float> normal;
  Vec3<float> world_pos;
  Vec2<float> uv;
  float inv_w{};
  
  VertexOut(Vec4<float> t_clip_pos) : clip_pos(t_clip_pos) {}
  VertexOut() = default;

  void set_clip_pos(Vec4<float> t_clip)
  {
	// stupid, need to sort this out
    clip_pos.set_x(t_clip.x());
	clip_pos.set_y(t_clip.y());
	clip_pos.set_z(t_clip.z());
	clip_pos.set_w(t_clip.w());
  }

  void set_normal(Vec3<float> t_normal)
  {
	// stupid, need to sort this out
    normal.set_x(t_normal.x());
	normal.set_y(t_normal.y());
	normal.set_z(t_normal.z());
  }

  void set_world_pos(Vec3<float> t_world)
  {
	// stupid, need to sort this out
    world_pos.set_x(t_world.x());
	world_pos.set_y(t_world.y());
	world_pos.set_z(t_world.z());
  }
  
};


class FragmentIn{
public:
  Vec3<float> normal;
  Vec3<float> world_pos;
  Vec2<float> uv;
  double depth{};
  
  FragmentIn(Vec3<float> t_normal, Vec3<float> t_world_pos, Vec2<float> t_uv, double t_depth) :
	normal(t_normal),
	world_pos(t_world_pos),
	uv(t_uv),
  	depth(t_depth) {}
  
  FragmentIn() = default;
  
};

using Primitive = std::array<VertexOut, 3>;
using FaceTriangles = std::array<Vec3<float>, 3>;   ///TODO VERTEXIN CLASS!!
using TriangleVertices = std::array<VertexIn, 3>;

class FacesStream{
private:
  OBJObject<float> &obj;
  std::size_t index {};

public:

  FacesStream(OBJObject<float> &t_obj) : obj(t_obj) {}
  FacesStream() = default;

  void set(OBJObject<float> &t_obj) {
	obj = t_obj;
	index = 0;
  }
  
  bool getFace(TriangleVertices& out){
	std::cout << "getting next TriangleVertives" << std::endl;
	if(index >= obj.faces.size()){
	  return false;
	}

	//----------Get-Indexes-Of-Vertex-Atributes----------//
	std::size_t tri1_idx = obj.faces.at(index).face_vertices.at(0) - 1;
	std::size_t tri2_idx = obj.faces.at(index).face_vertices.at(1) - 1;
	std::size_t tri3_idx = obj.faces.at(index).face_vertices.at(2) - 1;

	std::size_t normal1_idx = obj.faces.at(index).vert_normals.at(0) - 1;
	std::size_t normal2_idx = obj.faces.at(index).vert_normals.at(1) - 1;
	std::size_t normal3_idx = obj.faces.at(index).vert_normals.at(2) - 1;

	std::size_t tex1_idx = obj.faces.at(index).vert_texcoords.at(0) - 1;
	std::size_t tex2_idx = obj.faces.at(index).vert_texcoords.at(1) - 1;
	std::size_t tex3_idx = obj.faces.at(index).vert_texcoords.at(2) - 1;
	//-------------------------------------------------//

	std::cout << "calculated idxs" << std::endl;

	//----------Set-VertexIn----------//
	Vec3<float> v1_pos(obj.vertices.at(tri1_idx).get_vec().x(),
					   obj.vertices.at(tri1_idx).get_vec().y(),
					   obj.vertices.at(tri1_idx).get_vec().z());
	out.at(0).set_pos(v1_pos);
	
	Vec3<float> v2_pos(obj.vertices.at(tri2_idx).get_vec().x(),
					   obj.vertices.at(tri2_idx).get_vec().y(),
					   obj.vertices.at(tri2_idx).get_vec().z());
	out.at(1).set_pos(v2_pos);
	
	Vec3<float> v3_pos(obj.vertices.at(tri3_idx).get_vec().x(),
					   obj.vertices.at(tri3_idx).get_vec().y(),
					   obj.vertices.at(tri3_idx).get_vec().z());
	out.at(2).set_pos(v3_pos);

	Vec3<float> v1_nor(obj.normals.at(normal1_idx).get_vec().x(),
					   obj.normals.at(normal1_idx).get_vec().y(),
					   obj.normals.at(normal1_idx).get_vec().z());
	out.at(0).set_normal(v1_nor);
	
	Vec3<float> v2_nor(obj.normals.at(normal2_idx).get_vec().x(),
					   obj.normals.at(normal2_idx).get_vec().y(),
					   obj.normals.at(normal2_idx).get_vec().z());
	out.at(1).set_normal(v2_nor);
	
	Vec3<float> v3_nor(obj.normals.at(normal3_idx).get_vec().x(),
					   obj.normals.at(normal3_idx).get_vec().y(),
					   obj.normals.at(normal3_idx).get_vec().z());
	out.at(2).set_normal(v3_nor);

	//out.at(0).set_uv(obj.tex_coords.at(normal1_idx).get_vec());
	//out.at(1).set_uv(obj.tex_coords.at(normal2_idx).get_vec());
	//out.at(2).set_uv(obj.tex_coords.at(normal3_idx).get_vec());

	index++;
	std::cout << "index++" << std::endl;
	return true;
	
	
  }

  
};

std::optional<Primitive> vertex_shader(TriangleVertices trigverts, Matrix<float, 4, 4> &view_mat, Matrix<float, 4, 4> &persp_mat){

  std::cout << "called vertex_shader" << std::endl;
  
  VertexOut v1;
  VertexOut v2;
  VertexOut v3;

  auto &[vin1, vin2, vin3] = trigverts;

  Vec4<float> v1_pos(vin1.pos, 1.0F);
  Vec4<float> v2_pos(vin2.pos, 1.0F);
  Vec4<float> v3_pos(vin3.pos, 1.0F);

  Vec4<float> v1_normal(vin1.normal, 1.0F);
  Vec4<float> v2_normal(vin2.normal, 1.0F);
  Vec4<float> v3_normal(vin3.normal, 1.0F);

  auto tri1_m = view_mat * v1_pos;
  auto tri2_m = view_mat * v2_pos;
  auto tri3_m = view_mat * v3_pos;

  auto v1_view_normal = (view_mat * v1_normal).xyz();
  auto v2_view_normal = (view_mat * v2_normal).xyz();
  auto v3_view_normal = (view_mat * v3_normal).xyz();

  v1_view_normal = v1_view_normal * (1.0F / v1_view_normal.mag());
  v2_view_normal = v2_view_normal * (1.0F / v2_view_normal.mag());
  v3_view_normal = v3_view_normal * (1.0F / v3_view_normal.mag());

  v1.set_world_pos(tri1_m.xyz());
  v2.set_world_pos(tri2_m.xyz());
  v3.set_world_pos(tri3_m.xyz());

  v1.set_normal(v1_view_normal);
  v2.set_normal(v2_view_normal);
  v3.set_normal(v3_view_normal);

  //--- Calculate Unit normal for vertexout----------//
  //Vec3<float> A(tri1_m.x(), tri1_m.y(), tri1_m.z());
  //Vec3<float> B(tri2_m.x(), tri2_m.y(), tri2_m.z());
  //Vec3<float> C(tri3_m.x(), tri3_m.y(), tri3_m.z());
  //Vec3<float> AB = B - A;
  //Vec3<float> AC = C - A;
  //Vec3<float> normal      = (AB ^ AC);
  //Vec3<float> unit_normal = normal * (1.0F / normal.mag());
  //v1.normal = unit_normal;
  //v2.normal = unit_normal;
  //v3.normal = unit_normal;
  //-------------------------------------------------//

  //---------apply transformations-------------------//

  //--safe-projection---//
  auto safe_project = [&](const Vec4<float>& v) -> std::optional<Vec4<float>> {
	if (std::abs(v.w()) < 1e-6f)
	  return std::nullopt;
	return v;
  };
  
  auto tri1_clip = safe_project(persp_mat * tri1_m);
  auto tri2_clip = safe_project(persp_mat * tri2_m);
  auto tri3_clip = safe_project(persp_mat * tri3_m);

  // redundant?
  if (!tri1_clip || !tri2_clip || !tri3_clip){
	return std::nullopt;
  }

  v1.set_clip_pos(*tri1_clip);
  v2.set_clip_pos(*tri2_clip);
  v3.set_clip_pos(*tri3_clip);

  //-----Set inverse w for vertexout-----------------//
  v1.inv_w = 1.0F / tri1_clip->w();
  v2.inv_w = 1.0F / tri2_clip->w();
  v3.inv_w = 1.0F / tri3_clip->w();
  //-------------------------------------------------//

  return Primitive{v1, v2, v3};
}


template <typename FragFunc>
void rasterize(Primitive &primitive,
			   TGAImage &img,
			   std::vector<float> &zbuffer,
			   FragFunc fragment_shader_func)
{

    std::cout << "called rasterize_shader" << std::endl;

  //-----read-primitive-and-initialize-viewport-matrix--------//
  auto VP_matrix = viewport_matrix(img.width() / 16, img.height() / 16, img.width() * 7 / 8, img.height() * 7 / 8);
  Vec4<float> clip1_pos = primitive.at(0).clip_pos;
  Vec4<float> clip2_pos = primitive.at(1).clip_pos;
  Vec4<float> clip3_pos = primitive.at(2).clip_pos;

  Vec3<float> v1_normal = primitive.at(0).normal;
  Vec3<float> v2_normal = primitive.at(1).normal;
  Vec3<float> v3_normal = primitive.at(2).normal;  
  //----------------------------------------------------------//  

  //-----perspective-(w)-divide--------//
  clip1_pos = clip1_pos * primitive.at(0).inv_w;
  clip2_pos = clip2_pos * primitive.at(1).inv_w;
  clip3_pos = clip3_pos * primitive.at(2).inv_w;
  //-----------------------------------//  

  //----------screen space transform---------//
  auto screen1_pos = VP_matrix * clip1_pos;
  auto screen2_pos = VP_matrix * clip2_pos;
  auto screen3_pos = VP_matrix * clip3_pos;
  //-----------------------------------------//

  /// TODO WHY IS THIS NOT WORKING, INSTEAD YOU HAVE TO static_cast TO INT IN THE LOOP!!!
  //Vec4<int> int_screen1_pos = screen1_pos.round_to_int();
  //Vec4<int> int_screen2_pos = screen2_pos.round_to_int();
  //Vec4<int> int_screen3_pos = screen3_pos.round_to_int();
  
  auto& [ax, ay, az, aw] = screen1_pos.vector;
  auto& [bx, by, bz, bw] = screen2_pos.vector;
  auto& [cx, cy, cz, cw] = screen3_pos.vector;
  
  float sarea_total = s_triangle_area(ax, ay, bx, by, cx, cy);
  
  auto[box_x_min, box_x_max] = std::minmax({ax, bx, cx});
  auto[box_y_min, box_y_max] = std::minmax({ay, by, cy});

  box_x_min = std::clamp(box_x_min, 0.0F, static_cast<float>(img.width()));
  box_x_max = std::clamp(box_x_max, 0.0F, static_cast<float>(img.width()));

  box_y_min = std::clamp(box_y_min, 0.0F, static_cast<float>(img.height()));
  box_y_max = std::clamp(box_y_max, 0.0F, static_cast<float>(img.height()));

  
  for (int i = static_cast<int>(box_x_min); i <= static_cast<int>(box_x_max); ++i) {
    for (int j = static_cast<int>(box_y_min); j <= static_cast<int>(box_y_max); ++j) {

	  //----------Calculate-Interpolation-Weights----------//
	  float sareaPBC = s_triangle_area(i, j, bx, by, cx, cy);
      float sareaAPC = s_triangle_area(ax, ay, i, j, cx, cy);
      float sareaABP = s_triangle_area(ax, ay, bx, by, i, j);
	  
      float lam1 = sareaPBC / sarea_total;
      float lam2 = sareaAPC / sarea_total;
      float lam3 = sareaABP / sarea_total;
	  //---------------------------------------------------//	  
	  
	  auto z_val = static_cast<float>((lam1 * az) + (lam2 * bz) + (lam3 * cz));
	  Vec3<float> frag_normal = (v1_normal * lam1) + (v2_normal * lam2) + (v3_normal * lam3);
	  
	  size_t z_index = (i*img.width()) + j;
	  
	  if(lam1 >= 0.0F && lam2 >= 0.0F && lam3 >= 0.0F && sarea_total>1.0F) {
		if(zbuffer.at(z_index) < z_val){
		  
		  FragmentIn fragment_input(frag_normal,
									Vec3<float>(0.0F, 0.0F, 0.0F),
									Vec2<float>(0.0F, 0.0F),
									z_val);
		  
		  auto pixel_color = fragment_shader_func(fragment_input);
		  zbuffer.at(z_index) =  z_val;
		  img.set(i, j, pixel_color);
		}
	  }
	  
	}
  }
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

#endif // OPEN_TR_HPP
