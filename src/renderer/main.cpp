#include "tgaimage.hpp"
#include "math.hpp"
#include "OBJUtils.hpp"
#include "open_tr.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <cstdlib>
#include <algorithm>

TGAColor gray_shader() {return {150,150,150,255};}
TGAColor red_shader()  {return {30,30,255,255};}




TGAColor phong_shader(Vec3<float> &light_pos, const FaceTriangles &ftriag, Matrix<float, 4, 4> &mdl_mat){

  auto& [tri1, tri2, tri3] = ftriag;

  Vec4<float> tri1_m =  mdl_mat * Vec4<float>(tri1.x(), tri1.y(), tri1.z(), 1.0F);
  Vec4<float> tri2_m =  mdl_mat * Vec4<float>(tri2.x(), tri2.y(), tri2.z(), 1.0F);
  Vec4<float> tri3_m =  mdl_mat * Vec4<float>(tri3.x(), tri3.y(), tri3.z(), 1.0F);

  Vec3<float> A(tri1_m.x(), tri1_m.y(), tri1_m.z());
  Vec3<float> B(tri2_m.x(), tri2_m.y(), tri2_m.z());
  Vec3<float> C(tri3_m.x(), tri3_m.y(), tri3_m.z());

  Vec4<float> light_mdl = mdl_mat * Vec4<float>(light_pos.x(), light_pos.y(), light_pos.z(), 1.0F);
  Vec3<float> light_dir_vec(light_mdl.x(), light_mdl.y(), light_mdl.z());
  Vec3<float> light_dir_unit = light_dir_vec * (1.0F / light_dir_vec.mag());

  uint8_t light_value = UINT8_MAX;
  
   float ambient_val = 0.3F;
   Vec3<float> AB = B - A;
   Vec3<float> AC = B - C;

  Vec3<float> normal      = (AB ^ AC);
  Vec3<float> unit_normal = normal * (1.0F / normal.mag());

  float difuse_val = std::max(0.0F, unit_normal & light_dir_unit);

  light_value *= std::min(1.0F, ambient_val + 0.4F * difuse_val);

  return {light_value, light_value, light_value, UINT8_MAX};
  
}

int main([[maybe_unused]]int argc,[[maybe_unused]] const char** argv){

  const TGAColor white  (255, 255, 255, 255); // attention, BGRA order
  const TGAColor green  (  0, 255,   0, 255);
  const TGAColor red    (  0,   0, 255, 255);
  const TGAColor blue   (255, 128,  64, 255);
  const TGAColor yellow (  0, 200, 255, 255);

  Vec3<float>    eye{-1.0F , 0.0F, 2.0F};  // camera position
  Vec3<float> center{ 0.0F , 0.0F ,0.0F};  // camera direction
  Vec3<float>     up{ 0.0F , 1.0F, 0.0F};  // camera up vector

  Vec3<float>  light{ 1.0F , 1.0F, 1.0F};  // camera up vector
 
  constexpr int width  = 800;
  constexpr int height = 800;
  TGAImage diablo_fb(width, height, TGAImage::RGB);
  std::vector<float> diablo_zbfr (width*height, -1000.0F);

  OBJObject<float> model {};
  //read_obj("assets/diablo3_pose.obj", model);
  read_obj("assets/african_head.obj", model);

  diablo_fb.clear(TGAColor(0, 0, 0, 255)); // optional
  
  auto MV_matrix = lookat(eye, center, up);
  auto P_matrix = perspective_matrix((eye - center).mag());
  auto VP_matrix = viewport_matrix(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

  //reset z_buffer
  std::fill(diablo_zbfr.begin(), diablo_zbfr.end(), -1000.0F);  //maybe in open_tr

  //desired usage

   FacesStream facestream(model);
 //
   FaceTriangles face_triangles {};
 //
auto phong_lam = [&]() -> TGAColor {
  auto& [tri1, tri2, tri3] = face_triangles;

  Vec4<float> tri1_m =  MV_matrix * Vec4<float>(tri1.x(), tri1.y(), tri1.z(), 1.0F);
  Vec4<float> tri2_m =  MV_matrix * Vec4<float>(tri2.x(), tri2.y(), tri2.z(), 1.0F);
  Vec4<float> tri3_m =  MV_matrix * Vec4<float>(tri3.x(), tri3.y(), tri3.z(), 1.0F);

  Vec3<float> A(tri1_m.x(), tri1_m.y(), tri1_m.z());
  Vec3<float> B(tri2_m.x(), tri2_m.y(), tri2_m.z());
  Vec3<float> C(tri3_m.x(), tri3_m.y(), tri3_m.z());

  Vec4<float> light_mdl = MV_matrix * Vec4<float>(light.x(), light.y(), light.z(), 1.0F);
  Vec3<float> light_dir_vec(light_mdl.x(), light_mdl.y(), light_mdl.z());
  auto light_dir_unit = light_dir_vec * (1.0F / light_dir_vec.mag());

  uint8_t light_value = UINT8_MAX;
  
  float ambient_val = 0.3;
  Vec3<float> AB = B - A;
  Vec3<float> AC = C - A;

  Vec3<float> normal      = (AB ^ AC);
  Vec3<float> unit_normal = normal * (1.0F / normal.mag());

  Vec3<float> reflected = unit_normal * ((unit_normal & light_dir_unit) * 2) - light_dir_unit;
  Vec3<float> reflected_unit = reflected * (1.0F / reflected.mag());
  float spec_val = std::pow(std::max(reflected_unit.z(), 0.0F), 35);

  float difuse_val = std::max(0.0F, unit_normal & light_dir_unit);
  float difuse_coeff = 0.4F;

  light_value *= std::min(1.0F, ambient_val + difuse_coeff*difuse_val + 0.9F*spec_val);

  return {light_value, light_value, light_value, UINT8_MAX};
 };
   
   while (facestream.getFace(face_triangles)){
	 Primitive primitive = vertex_shader(face_triangles, MV_matrix, P_matrix, VP_matrix);
	 
	 rasterize(primitive, diablo_fb, diablo_zbfr, phong_lam);
   }

   //OLD
   //draw_primitives(diablo_fb, model, diablo_zbfr, MV_matrix, P_matrix, VP_matrix);

  diablo_fb.write_tga_file("diablo_img_ztest.tga");

  return 0;
}
