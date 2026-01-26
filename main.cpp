#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "bvh.h"
#include "bvh_custom.h"

hittable_list bouncing_sphere() {
  hittable_list world;

  auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

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

  return world;

}


hittable_list checkered_spheres() {
  hittable_list world;

  auto checker = std::make_shared<checker_texture>(0.21, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
  world.add(std::make_shared<sphere>(point3(0,-10, 0), 10, std::make_shared<lambertian>(checker)));
  world.add(std::make_shared<sphere>(point3(0, 10, 0), 10, std::make_shared<lambertian>(checker)));

  return world;
}

hittable_list earth() {
  hittable_list world;

  auto earth_texture = std::make_shared<image_texture>("earthmap.jpg"); // import image texture
  auto earth_surface = std::make_shared<lambertian>(earth_texture);     // create earth surface MATERIAL using the earth texture
  auto globe = std::make_shared<sphere>(point3(0,0,0), 2, earth_surface);

  world.add(globe);

  return world;
}

hittable_list perlin_spheres() {
  hittable_list world;

  auto pertext = std::make_shared<noise_texture>(10);
  world.add(
    std::make_shared<sphere>(
      point3(0, -1000, 0),
      1000,
      std::make_shared<lambertian>(pertext)
    )
  );

  world.add(
    std::make_shared<sphere>(
      point3(0, 2, 0),
      2,
      std::make_shared<lambertian>(pertext)
    )
  );

  return world;
}


int main(int argc, char* argv[]) {

  int render_case = 1;

  if (argc == 1) {
    render_case = (int) *argv[0];
  }

  hittable_list world;

  switch (4) {
    case 1: world = bouncing_sphere(); break;
    case 2: world = checkered_spheres(); break;
    case 3: world = earth(); break;
    case 4: world = perlin_spheres(); break;
  }

  int image_width = 400;
  double aspect_ratio = 16.0 / 9.0;
  double viewport_height = 2.0;
  double focal_length = 1.0;

    // Render
  Camera cam = Camera(image_width, aspect_ratio, viewport_height, focal_length);
  cam.enableAA = true;
  cam.reflectance_coeff = 0.5;
  cam.verticalFOV = 20;

  cam.look_from = point3(13, 2, 3);
  cam.look_at = point3(0, 0, 0);
  cam.world_up = vec3(0, 1, 0);
  cam.max_bounces = 50;
  cam.sample_per_pixel = 100;

  cam.dof_angle = 0.0;
  cam.focus_dist = 3.4;

  cam.Render(world);

}