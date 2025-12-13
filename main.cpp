#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
int main() {

  int image_width = 600;
  double aspect_ratio = 16.0 / 9.0;
  double viewport_height = 2.0;
  double focal_length = 1.0;

  hittable_list world;
  auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
  auto material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
  // auto material_left   = std::make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
  auto material_left   = std::make_shared<dielectric>(1.50);
  auto material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.2);

  world.add(std::make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(std::make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
  world.add(std::make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
  world.add(std::make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));
  // Render
  Camera mainCamera = Camera(image_width, aspect_ratio, viewport_height, focal_length, world);
  mainCamera.enableAA = true;
  mainCamera.reflectance_coeff = 0.5;
  mainCamera.Render();

}