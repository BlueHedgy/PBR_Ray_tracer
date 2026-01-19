#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "bvh.h"
#include "bvh_custom.h"

int main() {

  int image_width = 400;
  double aspect_ratio = 16.0 / 9.0;
  double viewport_height = 2.0;
  double focal_length = 1.0;

  hittable_list world;

  // auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
  // world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

  auto checker = std::make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
  world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(checker)));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        std::shared_ptr<material> sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = std::make_shared<lambertian>(albedo);
          auto center2 = center + vec3(0, random_double(0,.5), 0);
          world.add(std::make_shared<sphere>(center, center2, 0.2, sphere_material));
          world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
          auto fuzz = random_double(0, 0.5);
          sphere_material = std::make_shared<metal>(albedo, fuzz);
          world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = std::make_shared<dielectric>(1.5);
          world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }

  auto material1 = std::make_shared<dielectric>(1.5);
  world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
  world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  // world = hittable_list(std::make_shared<bvh_node_custom>(world));
  world = hittable_list(std::make_shared<bvh_node_custom>(world, 0, world.objects.size()));


  // Render
  Camera mainCamera = Camera(image_width, aspect_ratio, viewport_height, focal_length, world);
  mainCamera.enableAA = true;
  mainCamera.reflectance_coeff = 0.5;
  mainCamera.verticalFOV = 90;

  mainCamera.look_from = point3(-2, 2, 1);
  mainCamera.look_at = point3(0, 0, -1);
  mainCamera.world_up = vec3(0, 1, 0);
  mainCamera.max_bounces = 50;
  mainCamera.sample_per_pixel = 100;

  mainCamera.dof_angle = 0.0;
  mainCamera.focus_dist = 3.4;

  mainCamera.Render();

}