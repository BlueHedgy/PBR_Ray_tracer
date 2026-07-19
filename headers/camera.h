#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "light.h"
#include "material.h"
#include "multi_thread.h"

#include <fstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>
#include "gui_image_load.h"

typedef std::vector<std::vector<color>> image;


inline float hit_sphere(const point3& center, float radius, const ray& r) {
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

    int image_width = 1024;
    int image_height;
    int render_width, render_height;
    float aspect_ratio = 16.0 / 9.0;
    float focal_length;

    bool enableAA = true;   // Enable Anti-aliasing, default : true
    int sample_per_pixel = 50;
    int max_bounces = 10;
    float reflectance_coeff = 0.5;
    float verticalFOV = 60;
    color background;

    point3 look_from = vec3(0, 0, 2.5);
    point3 look_at = vec3(0, 0, 0);
    vec3   world_up  = vec3(0, 1, 0);

    float dof_angle = 0;
    float focus_dist = 10;

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


    void Render(
      const light_list& lights,
      const hittable_list& objects,
      const std::string& filename,
      image &output_image
    );

    /// @brief Create a temporary threadpool to render the image by rows (scanlines)
    /// @param render_cancelled polled check whether a render cancel is requested from the GUI
    /// @param d_imdata display image data
    void Render_MultiThreaded(
      const light_list& lights,
      const hittable_list& objects,
      const std::string &filename,
      const std::atomic_bool &render_cancelled,
      image &output_image, display_image_data &d_imdata
    );


  void Render_ChunkLines(
    const light_list &lights,
    const hittable_list &objects,
    size_t t_idx,
    const std::atomic_bool &render_cancelled,
    image &output_image,
    display_image_data &d_imdata
  );

    /// @brief Compute pixel's colors on an image row (scanline)
    /// @param render_cancelled polled check whether a render cancel is requested from the GUI
    /// @param d_imdata display image data
    void Render_Scanline(
      const light_list& lights,
      const hittable_list& objects,
      const size_t line_index,
      image &output_image,
      const std::atomic_bool &render_cancelled,
      display_image_data &d_imdata
    );

    /// @brief Write the rendered image to a .PPM file format
    /// @param output_image
    /// @param filename
    void WriteImageToFile (const image &output_image, const std::string &filename);

    void initialize();


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

    /// @brief Recursively obtain the color of a traced ray using PBR Cook Torrance GGX reflectance model
    /// @param r current (bounced) ray
    /// @param scene
    /// @param max_bounces

    /// @param isEmissive inform the previous bounce whether this hit is emissive
    /// @param hit_point inform the previous bounce of this hit point location (for emission)
    /// @return
    color ray_color(
      const light_list& lights,
      const hittable_list& objects,
      const ray& r,
      int max_bounces,
      bool& isEmissive,
      vec3& hit_point
    );

    void process_ray_samples(
      const light_list& lights,
      const hittable_list& objects,
      int i, int j,
      color &pixel_color
    ) {
      for (int sample = 0; sample < sample_per_pixel; sample++){
        ray r = get_ray(i, j);
        bool isEmissive;
        vec3 hit_point;
        pixel_color += ray_color(lights, objects, r, max_bounces, isEmissive, hit_point);
      }
    }

    /// @brief Create a ray from camera origin to randomly sampled location around pixel i, j
    ray get_ray(int i, int j) const;

    /// @brief return a vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    vec3 sample_square() const {
      return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
      // Returns a random point in the camera defocus disk.
      auto p = random_in_unit_disk();
      return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

};


class camera_list {
  public:
    camera_list() {
      cameras.push_back(Camera());
    };
    camera_list(const std::vector<Camera> &cams) : cameras(cams) {}

    const std::vector<Camera>& get_all() const {
      return cameras;
    }

    Camera& get_cam(size_t index) {
      return cameras.at(index);
    }

    void add (const Camera &cam) {
      cameras.push_back(cam);
    }

    void remove(size_t index) {
      if (0 <= index && index < cameras.size()) {
        cameras.erase(cameras.begin() + index);
      }
    }

  private:
    std::vector<Camera> cameras;
};

#endif
