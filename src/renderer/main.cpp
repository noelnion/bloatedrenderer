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

int main([[maybe_unused]]int argc,[[maybe_unused]] const char** argv){

  const TGAColor white  (255, 255, 255, 255); // attention, BGRA order
  const TGAColor green  (  0, 255,   0, 255);
  const TGAColor red    (  0,   0, 255, 255);
  const TGAColor blue   (255, 128,  64, 255);
  const TGAColor yellow (  0, 200, 255, 255);

  Vec3<float>    eye{-1.0F , 0.0F, 2.0F};  // camera position
  Vec3<float> center{ 0.0F , 0.0F ,0.0F};  // camera direction
  Vec3<float>     up{ 0.0F , 1.0F, 0.0F};  // camera up vector

  Vec3<float>  light{ 1.0F , 1.0F, 1.0F};    // light source pos
  TGAColor light_color(144, 117, 255, 255);  // light source color
 
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


  //reset z_buffer
  std::fill(diablo_zbfr.begin(), diablo_zbfr.end(), -1000.0F);  //maybe in open_tr


  FacesStream facestream(model);

  TriangleVertices face_triangles {};

  auto phong_lam = [&](FragmentIn fragIn) -> TGAColor {

	std::cout << "called frag_shader" << std::endl;

	Vec4<float> light_mdl = MV_matrix * Vec4<float>(light.x(), light.y(), light.z(), 1.0F);
	Vec3<float> light_dir_vec(light_mdl.x(), light_mdl.y(), light_mdl.z());
	auto light_dir_unit = light_dir_vec * (1.0F / light_dir_vec.mag());

	uint8_t blue = light_color[0];
	uint8_t gren = light_color[1];
	uint8_t redd = light_color[2];
  
	float ambient_val = 0.3;
	
	Vec3<float> reflected = fragIn.normal * ((fragIn.normal & light_dir_unit) * 2) - light_dir_unit;
	Vec3<float> reflected_unit = reflected * (1.0F / reflected.mag());
	float spec_val = std::pow(std::max(reflected_unit.z(), 0.0F), 35);

	float difuse_val = std::max(0.0F, fragIn.normal & light_dir_unit);
	float difuse_coeff = 0.4F;

	float total_lighting = ambient_val + difuse_coeff*difuse_val + 0.9F*spec_val;

	///TODO tgacolor optimization to do per channel multiplication
	blue *= std::min(1.0F, total_lighting);
	redd *= std::min(1.0F, total_lighting);
	gren *= std::min(1.0F, total_lighting);

	return {blue,
			gren,
			redd,
			UINT8_MAX};
  };

  auto blueish_shader = [&](FragmentIn fragIn) -> TGAColor {
	return {222, 20, 100, UINT8_MAX};
  };

  auto bound_shader = phong_lam;
   
  while (facestream.getFace(face_triangles)){
	auto primitive_opt = vertex_shader(face_triangles, MV_matrix, P_matrix);
	if (!primitive_opt.has_value()){
	  continue; // safely skip triangle
	}
	rasterize(*primitive_opt, diablo_fb, diablo_zbfr, bound_shader);
  }

  OBJObject<float> eyes {};
  read_obj("assets/african_head_eye_outer.obj", eyes);
  facestream.set(eyes);


  light_color[0] = 50;
  light_color[1] = 50;
  light_color[2] = 50;  
	
  while (facestream.getFace(face_triangles)){
   auto primitive_opt = vertex_shader(face_triangles, MV_matrix, P_matrix);
	if (!primitive_opt.has_value()){
	  continue; // safely skip triangle
	}	 
   rasterize(*primitive_opt, diablo_fb, diablo_zbfr, bound_shader);
 }

  diablo_fb.write_tga_file("shading_test.tga");

  return 0;
}
