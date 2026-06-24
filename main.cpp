#include "utils.h"
#include "hittable.h"
#include "hittable_list.h"
#include "light.h"
#include "sphere.h"
#include "quad.h"
#include "triangle.h"
#include "camera.h"
#include "bvh.h"
#include "bvh_custom.h"
#include "gui_setup.h"
#include "scene.h"

#include <string>


hittable_list bouncing_sphere() {
  hittable_list object_list;

  auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
  object_list.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

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
          object_list.add(std::make_shared<sphere>(center, center2, 0.2, sphere_material));
          object_list.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
          auto fuzz   = random_double(0, 0.5);
          sphere_material = std::make_shared<metal>(albedo, fuzz);
          object_list.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = std::make_shared<dielectric>(1.5);
          object_list.add(std::make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }

  auto material1 = std::make_shared<dielectric>(1.5);
  object_list.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
  object_list.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  auto material3 = std::make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  object_list.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  // object_list = hittable_list(std::make_shared<bvh_node_custom>(object_list));
  object_list = hittable_list(std::make_shared<bvh_node_custom>(object_list, 0, object_list.objects.size()));

  return object_list;

}

hittable_list checkered_spheres() {
  hittable_list object_list;

  auto checker = std::make_shared<checker_texture>(0.21, color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
  object_list.add(std::make_shared<sphere>(point3(0,-10, 0), 10, std::make_shared<lambertian>(checker)));
  object_list.add(std::make_shared<sphere>(point3(0, 10, 0), 10, std::make_shared<lambertian>(checker)));

  return object_list;
}

hittable_list earth() {
  hittable_list object_list;

  auto earth_texture = std::make_shared<image_texture>("earthmap.jpg"); // import image texture
  auto earth_surface = std::make_shared<lambertian>(earth_texture);     // create earth surface MATERIAL using the earth texture
  auto globe = std::make_shared<sphere>(point3(0,0,0), 2, earth_surface);

  object_list.add(globe);

  return object_list;
}

hittable_list perlin_spheres() {
  hittable_list object_list;

  auto pertext = std::make_shared<noise_texture>(10);
  object_list.add(
    std::make_shared<sphere>(
      point3(0, -1000, 0),
      1000,
      std::make_shared<lambertian>(pertext)
    )
  );

  object_list.add(
    std::make_shared<sphere>(
      point3(0, 2, 0),
      2,
      std::make_shared<lambertian>(pertext)
    )
  );

  return object_list;
}

hittable_list quad_scene() {
  hittable_list object_list;

  auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green   = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue   = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal   = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

  // Quads
  object_list.add(std::make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
  object_list.add(std::make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
  object_list.add(std::make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
  object_list.add(std::make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
  object_list.add(std::make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

  return object_list;
}


hittable_list tris_scene() {
  hittable_list object_list;

  auto left_red     = std::make_shared<lambertian>(color(1.0, 0.2, 0.2));
  auto back_green   = std::make_shared<lambertian>(color(0.2, 1.0, 0.2));
  auto right_blue   = std::make_shared<lambertian>(color(0.2, 0.2, 1.0));
  auto upper_orange = std::make_shared<lambertian>(color(1.0, 0.5, 0.0));
  auto lower_teal   = std::make_shared<lambertian>(color(0.2, 0.8, 0.8));

  // Triangles
  object_list.add(std::make_shared<triangle>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
  object_list.add(std::make_shared<triangle>(point3(-3, 2, 1), vec3(0, 0, 4), vec3(0,-4, 0), back_green));
  object_list.add(std::make_shared<triangle>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
  object_list.add(std::make_shared<triangle>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
  object_list.add(std::make_shared<triangle>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
  object_list.add(std::make_shared<triangle>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

  return object_list;
}

hittable_list simple_light() {
  hittable_list object_list;

  auto pertext = std::make_shared<noise_texture>(4);
  object_list.add(std::make_shared<sphere>(point3(0,-1000,0), 1000, std::make_shared<lambertian>(pertext)));
  object_list.add(std::make_shared<sphere>(point3(0,2,0), 2, std::make_shared<lambertian>(pertext)));

  auto difflight = std::make_shared<diffuse_light>(color(1, 1, 1));
  auto voronoi_light = std::make_shared<diffuse_light>(std::make_shared<image_texture>("worley.png"));
  object_list.add(std::make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));
  return object_list;
}

Scene metal_sphere_pbr() {
  Scene scene = Scene(
    std::monostate{},
    std::monostate{},
    std::monostate{}
  );

  scene.add_light(std::make_shared<point_light>(vec3(-200, 200, -200), color(2, 0, 0)));
  scene.add_light(std::make_shared<point_light>(vec3(200, 200, -200), color(0, 2, 0)));
  scene.add_light(std::make_shared<point_light>(vec3(200, 200, 200), color(0, 0, 2)));
  scene.add_light(std::make_shared<point_light>(vec3(-200, 200, 200), color(2, 0, 2)));
  scene.add_light(std::make_shared<directional_light>(vec3(-1, -1, -1.05), color(4, 4, 4)));

  auto metal_color = std::make_shared<image_texture>("rustediron2_basecolor.png");
  auto metal_normal = std::make_shared<image_texture>("rustediron2_normal.png");
  auto metal_roughness = std::make_shared<image_texture>("rustediron2_roughness.png");
  auto metal_metalness = std::make_shared<image_texture>("rustediron2_metallic.png");


  auto red     = std::make_shared<pbr_material>(color(1.0, 0.2, 0.2), std::monostate {}, color(.1), color(.4), 0.2);
  auto yellow  = std::make_shared<pbr_material>(color(1.0, 1.0, 0.2), std::monostate {}, color(.1), color(.4), 0.2);
  auto blue    = std::make_shared<pbr_material>(color(0.2, 0.2, 1.0), std::monostate {}, color(.1), color(.4), 0.2);
  auto gray    = std::make_shared<pbr_material>(color(1), std::monostate {}, color(.1), color(.4), 0.2);
  auto metal_sphere = std::make_shared<pbr_material>(metal_color, std::monostate {}, metal_metalness, metal_roughness, 0.3);

  scene.add_object(std::make_shared<sphere>(point3(-1, 1, 0), 1, red));
  scene.add_object(std::make_shared<sphere>(point3(1, 0, 0), 1, metal_sphere));
  scene.add_object(std::make_shared<sphere>(point3(0, -1001, 0), 1000, gray));

  auto difflight = std::make_shared<diffuse_light>(color(0, 0, 10));
  auto difflight1 = std::make_shared<diffuse_light>(color(10, 10, 0));
  auto difflight2 = std::make_shared<diffuse_light>(color(10, 10, 10));

  // scene.add_object(std::make_shared<quad>(point3(2.5, 2, -2), vec3(0, -3, 0), vec3(0, 0, 3), yellow));
  // scene.add_object(std::make_shared<quad>(point3(-2.5, 2, -2), vec3(0, -3, 0), vec3(0, 0, 3), blue));
  scene.add_object(std::make_shared<quad>(point3(-2.5, 2.5, 2.5), vec3(0, 0, -5), vec3(5, 0, 0), difflight2));


  return scene;
}

int main(int argc, char* argv[]) {

  int render_case;
  char *filename;

  if (argc != 3) {
    std::cout << "Command: render_case filename !!" << std::endl;
    return -1;
  }

  try  {
    render_case = std::stoi(argv[1]);
  } catch (const std::exception& e) {
    std::cerr << "render_case NOT an integer.\n";
    return 1;
  }

  filename = argv[2];

  hittable_list object_list;
  Scene scene;
  // render_case = 8;
  switch (render_case) {
    case 1: object_list = bouncing_sphere(); break;
    case 2: object_list = checkered_spheres(); break;
    case 3: object_list = earth(); break;
    case 4: object_list = perlin_spheres(); break;
    case 5: object_list = quad_scene(); break;
    case 6: object_list = tris_scene(); break;
    case 7: object_list = simple_light(); break;
    case 8: scene = metal_sphere_pbr(); break;
  }

    // Render
  Camera cam = Camera();

  cam.image_width = 1000;
  cam.aspect_ratio = 16.0 / 9.0;
  cam.enableAA          = true;
  cam.reflectance_coeff = 0.5;
  cam.verticalFOV       = 60;

  cam.look_from         = point3(0, 0, 5);
  cam.look_at           = point3(0, 0, 0);
  cam.world_up          = vec3(0, 1, 0);

  cam.max_bounces       = 20;
  cam.sample_per_pixel  = 100;

  cam.dof_angle         = 0.0;
  cam.focus_dist        = 3.4;
  cam.background        = color(0);

  cam.Render(scene, filename);

  // GUI_Handler GUI(cam, object_list, filename);
  // GUI.SETUP();

}