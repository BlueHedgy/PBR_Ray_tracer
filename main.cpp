#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "quad.h"
#include "triangle.h"
#include "camera.h"
#include "bvh.h"
#include "bvh_custom.h"

#include <string>

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
          auto fuzz   = random_double(0, 0.5);
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

hittable_list quad_scene() {
  hittable_list world;

  auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green   = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue   = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal   = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

  // Quads
  world.add(std::make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
  world.add(std::make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
  world.add(std::make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
  world.add(std::make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
  world.add(std::make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

  return world;
}

hittable_list sphere_pbr() {
  hittable_list world;
  vec3 light_pos = vec3(2000, 2000, -2000);
  auto earth_texture = std::make_shared<image_texture>("earthmap.jpg"); // import image texture

  auto red    = std::make_shared<pbr_material>(color(1.0, 0.2, 0.2), 0.5, light_pos);
  auto green  = std::make_shared<pbr_material>(color(0.2, 1.0, 0.2), 0.5, light_pos);
  auto earth_surface = std::make_shared<pbr_material>(earth_texture, 0.1, light_pos);

  world.add(std::make_shared<sphere>(point3(-1, 0, 0), 1, earth_surface));
  world.add(std::make_shared<sphere>(point3(3, 1, 0), 0.25, green));

  auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  // world.add(std::make_shared<sphere>(point3(-1,0,0), 1, left_red));

  auto difflight = std::make_shared<diffuse_light>(color(4, 4, 4));
  world.add(std::make_shared<quad>(point3(-3,3,0), vec3(-3,-2,0), vec3(-3,-2,-3), difflight));



  return world;
}


hittable_list tris_scene() {
  hittable_list world;

  auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green   = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue   = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal   = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

  // Triangles
  world.add(std::make_shared<triangle>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
  world.add(std::make_shared<triangle>(point3(-3, 2, 1), vec3(0, 0, 4), vec3(0,-4, 0), back_green));
  world.add(std::make_shared<triangle>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
  world.add(std::make_shared<triangle>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
  world.add(std::make_shared<triangle>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
  world.add(std::make_shared<triangle>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

  return world;
}

hittable_list simple_light() {
  hittable_list world;

  auto pertext = std::make_shared<noise_texture>(4);
  world.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(pertext)));
  world.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(pertext)));

  auto difflight = std::make_shared<diffuse_light>(color(4, 4, 4));
  auto voronoi_light = std::make_shared<diffuse_light>(std::make_shared<image_texture>("worley.png"));
  // world.add(std::make_shared<sphere>(point3(0,7,0), 2, difflight));
  world.add(std::make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));
  return world;
}

int main(int argc, char* argv[]) {

  int render_case;

  // if (argc != 2) {
  //   std::cout << "Wrong input, enter an integer only !!" << std::endl;
  //   return -1;
  // }

  // try  {
  //   render_case = std::stoi(argv[1]);
  // } catch (const std::exception& e) {
  //   std::cerr << "Not an integer.\n";
  //   return 1;
  // }

  hittable_list world;
  render_case = 8;
  switch (render_case) {
    case 1: world = bouncing_sphere(); break;
    case 2: world = checkered_spheres(); break;
    case 3: world = earth(); break;
    case 4: world = perlin_spheres(); break;
    case 5: world = quad_scene(); break;
    case 6: world = tris_scene(); break;
    case 7: world = simple_light(); break;
    case 8: world = sphere_pbr(); break;
  }

  int image_width = 400;
  double aspect_ratio = 16.0 / 9.0;
  double viewport_height = 2.0;
  double focal_length = 1.0;

    // Render
  Camera cam = Camera(image_width, aspect_ratio, viewport_height, focal_length);
  cam.enableAA          = true;
  cam.reflectance_coeff = 0.5;
  cam.verticalFOV       = 60;

  cam.look_from         = point3(0, 0, -5);
  cam.look_at           = point3(0, 0, 0);
  cam.world_up          = vec3(0, 1, 0);

  cam.max_bounces       = 20;
  cam.sample_per_pixel  = 100;

  cam.dof_angle         = 0.0;
  cam.focus_dist        = 3.4;
  cam.background        = color(0, 0, 0);

  cam.Render(world);

}