#include "tgaimage.hpp"
#include "math.hpp"
#include "OBJUtils.hpp"
#include "open_tr.hpp"

#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

TGAColor gray_shader() {return {150,150,150,255};}
TGAColor red_shader()  {return {30,30,255,255};}

int main([[maybe_unused]]int argc,[[maybe_unused]] const char** argv){

  const TGAColor white  (255, 255, 255, 255); // attention, BGRA order
  const TGAColor green  (  0, 255,   0, 255);
  const TGAColor red    (  0,   0, 255, 255);
  const TGAColor blue   (255, 128,  64, 255);
  const TGAColor yellow (  0, 200, 255, 255);

  Vec3<float>    eye{-1.0F , 0.0F, 2.0F};  // camera position
  Vec3<float> center{ 0.0F , 0.0F ,0.0F};  // camera direction
  Vec3<float>     up{ 0.0F , 1.0F, 0.0F};  // camera up vector
 
  constexpr int width  = 800;
  constexpr int height = 800;
  TGAImage diablo_fb(width, height, TGAImage::RGB);
  std::vector<float> diablo_zbfr (width*height, -1000.0F);

  OBJObject<float> model {};
  read_obj("assets/diablo3_pose.obj", model);

  diablo_fb.clear(TGAColor(177, 195, 209, 255)); // optional
  
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
   while (facestream.getFace(face_triangles)){
	 Primitive primitive = vertex_shader(face_triangles, MV_matrix, P_matrix, VP_matrix);
	 rasterize(primitive, diablo_fb, diablo_zbfr, red_shader);
   }

   //OLD
   //draw_primitives(diablo_fb, model, diablo_zbfr, MV_matrix, P_matrix, VP_matrix);

  diablo_fb.write_tga_file("diablo_img_ztest.tga");

  return 0;
}
