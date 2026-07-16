#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "light.h"
#include "scene.h"
#include "material.h"
#include "multi_thread.h"

#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>

#include "gui_image_load.h"

typedef std::vector<std::vector<color>> image;


float hit_sphere(const point3& center, float radius, const ray& r) {
  vec3 oc = center - r.origin();
  auto a = r.direction().length_squared();
  auto h = dot(r.direction(),oc); // b = -2h
  auto c = oc.length_squared() - radius*radius;
  auto discriminant = h*h - a*c;

  if (discriminant < 0) {
    return -1.0;
  }
  return (h - std::sqrt(discriminant) ) / a;
}


class Camera {
  public:

    int image_width, image_height;
    int render_width, render_height;
    float aspect_ratio, focal_length;

    bool enableAA = true;   // Enable Anti-aliasing, default : true
    int sample_per_pixel = 2;
    int max_bounces = 10;
    float reflectance_coeff = 0.5;
    float verticalFOV = 90;
    color background;

    point3 look_from;
    point3 look_at;
    vec3   world_up  = vec3(0, 1, 0);

    float dof_angle = 0;
    float focus_dist = 10;

    lightList lights;

    Camera() {}

    vec3 getCenter(){ return camera_center; }
    void setCenter(vec3 newPosition){ camera_center = newPosition; }

    float getAspectRatio(){ return aspect_ratio; }

    vec3 ViewportU(){ return viewport_u; }
    vec3 ViewportV(){ return viewport_v; }

    vec3 Pixel_DeltaU(){ return pixel_delta_u; }
    vec3 Pixel_DeltaV(){ return pixel_delta_v; }

    vec3 Pixel00_Loc(){ return pixel00_loc; }
    vec3 VP_Upper_Left(){ return viewport_upper_left; }


    void Render(Scene &scene, const std::string& filename, image &output_image){
      initialize();
      std::ofstream out(filename);
      if (!out) {
        std::cerr << "Error: Could not open file for writing.\n";
        return;
      }

      scene.process_object_bvh();
      output_image = image(image_width, std::vector<color>(image_height));

      out << "P3\n" << image_width << " " << image_height << "\n255\n";
      for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
          if (enableAA) {
            color pixel_color = color(0, 0, 0);
            process_ray_samples(i, j, pixel_color, scene);
            output_image[i][j] = pixel_color;
          }
          else {
            vec3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            vec3 ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);

            bool isEmissive;
            vec3 hit_point;
            color pixel_color = ray_color(r, scene, max_bounces, reflectance_coeff, isEmissive, hit_point);
            output_image[i][j] = pixel_color;
          }
        }
      }

      std::clog << "\rDone.                 \n";
      out.close();  // Explicitly close (optional, but good practice)
    }


    void Render_MultiThreaded(Scene &scene, const std::string &filename,
      const std::atomic_bool &render_cancelled, image &output_image, display_image_data &d_imdata)
    {
      // initialize();
      scene.process_object_bvh();

      output_image = image(render_width, std::vector<color>(render_height));
      thread_pool render_threads;

      for (int j = 0; j < render_height; j++) {
        render_threads.enqueue([this, j, &scene, &output_image, &render_cancelled, &d_imdata] {
          Render_Scanline(j, scene, output_image, render_cancelled, d_imdata);
        });
      }
    }

    void Render_Scanline(size_t line_index,
      Scene &scene, image &output_image,
      const std::atomic_bool &render_cancelled, display_image_data &d_imdata)
    {
      for (int i = 0; i < render_width; i++) {
        if (render_cancelled) { return; }
        color pixel_color = color(0, 0, 0);
        if (enableAA) {
          process_ray_samples(i, line_index, pixel_color, scene);
          output_image[i][line_index] = pixel_color;
        }
        else {
          vec3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (line_index * pixel_delta_v);
          vec3 ray_direction = pixel_center - camera_center;
          ray r(camera_center, ray_direction);

          bool isEmissive;
          vec3 hit_point;
          pixel_color = ray_color(r, scene, max_bounces, reflectance_coeff, isEmissive, hit_point);
          output_image[i][line_index] = pixel_color;
        }

        pixel_color /= sample_per_pixel;

        float red = linear_to_gamma2(pixel_color.x());
        float green = linear_to_gamma2(pixel_color.y());
        float blue = linear_to_gamma2(pixel_color.z());

        size_t idx = (line_index * render_width + i) * 4;

        d_imdata.output_image_data[idx + 0] = std::clamp(red, 0.0f, 0.999f);
        d_imdata.output_image_data[idx + 1] = std::clamp(green, 0.0f, 0.999f);
        d_imdata.output_image_data[idx + 2] = std::clamp(blue, 0.0f, 0.999f);
        d_imdata.output_image_data[idx + 3] = 1.0f;
      }
    }

    void WriteImageToFile (const image &output_image, const std::string &filename){
      std::ofstream out(filename);
      if (!out) {
        std::cerr << "Error: Could not open file for writing.\n";
        return;
      }

      out << "P3\n" << render_width << " " << render_height << "\n255\n";
      for (int j = 0; j < render_height; j++) {
        std::clog << "\rScanlines remaining: " << (render_height - j) << ' ' << std::flush;
        for (int i = 0; i < render_width; i++) {
          color pixel_color = output_image[i][j];
          write_color(out, pixel_samples_scale * pixel_color);
        }
      }

      std::clog << "\rDone.                 \n";
      out.close();  // Explicitly close (optional, but good practice)
    }

    void initialize(){
      image_height = get_imageHeight(image_width, aspect_ratio);
      pixel_samples_scale = 1.0 / sample_per_pixel;

      camera_center = look_from;

      // Calculate camera parameters
      cam_w = unit_vector(look_from - look_at);
      cam_u = unit_vector(cross(world_up, cam_w));
      cam_v = cross(cam_w, cam_u);

      // Calculate viewport dimensions
      float theta = degrees_to_radians(verticalFOV);
      float h = std::tan(theta/2);
      float viewport_height = 2 * h * focus_dist;
      viewport_width = viewport_height * (float(image_width)/image_height);

      // Calculate the vectors across the horizontal and down the vertical viewport edges.
      viewport_u = viewport_width * cam_u;
      viewport_v = viewport_height * -cam_v;

      // Calculate the horizontal and vertical delta vectors from pixel to pixel.
      pixel_delta_u = viewport_u / image_width;
      pixel_delta_v = viewport_v / image_height;

      // Calculate the location of the upper left pixel.
      viewport_upper_left = camera_center - (focus_dist * cam_w) - viewport_u/2 - viewport_v/2;
      pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

      // Calculate the camera defocus disk basis vectors.
      auto defocus_radius = focus_dist * std::tan(degrees_to_radians(dof_angle / 2));
      defocus_disk_u = cam_u * defocus_radius;
      defocus_disk_v = cam_v * defocus_radius;
    }


  private:
    vec3 camera_center;

    float viewport_width, viewport_height;

    vec3 viewport_u, viewport_v;
    vec3 pixel_delta_u, pixel_delta_v;

    vec3 viewport_upper_left, pixel00_loc;

    float pixel_samples_scale;         // Color scale factor for a sum of pixel samples

    vec3 cam_u, cam_v, cam_w;
    vec3 defocus_disk_u, defocus_disk_v;

    /// @brief Get integer image height based on the aspect ratio of the image
    int get_imageHeight (int imWidth, float aspectRatio){
        int image_height = int (imWidth/ aspectRatio);
        return image_height = (image_height < 1) ? 1 : image_height;
    }


    color ray_color(const ray& r, const Scene& scene, int max_bounces, float reflectance_coeff, bool& isEmissive, vec3& hit_point){
      if (max_bounces <= 0) return color(0, 0, 0);

      hit_record rec;

      if (!scene.get_objects().hit(r, interval(0.001, Infinity), rec)) {  // 0.001 tolerance for floating point rounding error
        return background;
      }

      color emission;
      if (rec.material->emitted(r, rec, emission)){
        hit_point = rec.point_incident;
        isEmissive = true;
        return emission;
      }

      color direct_lighting = color(0, 0, 0);

      for (int i = 0; i < scene.get_lights().size(); i++){
        const auto& light = *scene.get_lights()[i];

        vec3 light_direction;
        vec3 light_color;
        if (light.type == POINT_LIGHT) {
          light_direction = unit_vector(light.get_position() - rec.point_incident);
          float distance_squared = (light.get_position() - rec.point_incident).length_squared();
          light_color = light.get_color() * 1 / distance_squared;
        }
        else if (light.type == DIRECTIONAL_LIGHT) {
          light_direction = unit_vector(-light.get_direction());
          light_color = light.get_color();
        }

        direct_lighting += rec.material->pbr_color(r, rec, light_color, light_direction);
      }

      ray scattered;// or from emission
      color attenuation;
      if (!rec.material->scatter(r, rec, attenuation, scattered)){
          return direct_lighting;
      }

      // getting info whether we need to check for emission after current bound
      bool nextEmissive = false;
      vec3 next_hit_point;
      color next_ray_color;
      color color_from_emission = color(0, 0, 0);

      next_ray_color = ray_color(scattered, scene, max_bounces-1, reflectance_coeff, nextEmissive, next_hit_point);

      return  direct_lighting + attenuation * next_ray_color;
    }

    void process_ray_samples(int i, int j, color &pixel_color, const Scene& scene){
      for (int sample = 0; sample < sample_per_pixel; sample++){
        ray r = get_ray(i, j);
        bool isEmissive;
        vec3 hit_point;
        pixel_color += ray_color(r, scene, max_bounces, reflectance_coeff, isEmissive, hit_point);
      }
    }

    /// @brief Create a ray from camera origin to randomly sampled location around pixel i, j
    ray get_ray(int i, int j) const{
      vec3 offset = sample_square();
      vec3 pixel_sample = pixel00_loc
      + ((i + offset.x()) * pixel_delta_u)
      + ((j + offset.y()) * pixel_delta_v);

      vec3 ray_origin = (dof_angle <= 0) ? camera_center: defocus_disk_sample();
      vec3 ray_direction = pixel_sample - ray_origin;

      float ray_time = random_double();

      return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square() const {
      // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
      return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
      // Returns a random point in the camera defocus disk.
      auto p = random_in_unit_disk();
      return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

};

#endif
