#include "camera.h"

void Camera::initialize() {
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
  viewport_height = 2 * h * focus_dist;
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


void Camera::Render(
  const light_list& lights,
  const hittable_list& objects,
  const std::string& filename,
  image &output_image) {
  initialize();

  output_image = image(image_width, std::vector<color>(image_height));

  for (int j = 0; j < image_height; j++) {
    for (int i = 0; i < image_width; i++) {
      color pixel_color = color(0, 0, 0);
      if (enableAA) {
        process_ray_samples(lights, objects, i, j, pixel_color);
      }
      else {
        vec3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        vec3 ray_direction = pixel_center - camera_center;
        ray r(camera_center, ray_direction);

        bool isEmissive;
        vec3 hit_point;
        pixel_color = ray_color(lights, objects, r, max_bounces, isEmissive, hit_point);
      }
      output_image[i][j] = pixel_color;
    }
  }
}


// MULTI-THREADING CPU RENDER________________________________________________________________________

void Camera::Render_MultiThreaded(
  const light_list& lights,
  const hittable_list& objects,
  const std::string &filename,
  const std::atomic_bool &render_cancelled,
  image &output_image,
  display_image_data &d_imdata
) {

  output_image = image(render_width, std::vector<color>(render_height));
  thread_pool render_threads;


  size_t num_threads = std::thread::hardware_concurrency();
  for (size_t tid = 0; tid < num_threads; tid++){
    render_threads.enqueue([this, &lights, &objects, tid, &output_image, &render_cancelled, &d_imdata] {
      Render_ChunkLines(lights, objects, tid,  render_cancelled, output_image, d_imdata);
    });
  }
}

void Camera::Render_ChunkLines(
  const light_list &lights,
  const hittable_list &objects,
  size_t t_idx,
  const std::atomic_bool &render_cancelled,
  image &output_image,
  display_image_data &d_imdata
) {
  size_t chunk_size = std::ceil(image_height / std::thread::hardware_concurrency());
  for (
    size_t line_index = t_idx * chunk_size;
    line_index < (t_idx + 1) * chunk_size && line_index < render_height;
    line_index++
  ) {
    Render_Scanline(lights, objects, line_index, output_image, render_cancelled, d_imdata);
  }
}


void Camera::Render_Scanline(
  const light_list& lights,
  const hittable_list& objects,
  const size_t line_index,
  image &output_image,
  const std::atomic_bool &render_cancelled,
  display_image_data &d_imdata
) {
  for (int i = 0; i < render_width; i++) {
    if (render_cancelled) { return; }
    color pixel_color = color(0, 0, 0);

    if (enableAA) {
      process_ray_samples(lights, objects, i, line_index, pixel_color);
      pixel_color /= sample_per_pixel;
    }
    else {
      vec3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (line_index * pixel_delta_v);
      vec3 ray_direction = pixel_center - camera_center;
      ray r(camera_center, ray_direction);

      bool isEmissive;
      vec3 hit_point;
      pixel_color = ray_color(lights, objects, r, max_bounces, isEmissive, hit_point);
    }

    output_image[i][line_index] = pixel_color;

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

//_________________________________________________________________________________________________



color Camera::ray_color(
  const light_list& lights,
  const hittable_list& objects,
  const ray& r,
  int max_bounces,
  bool& isEmissive,
  vec3& hit_point
) {
  if (max_bounces <= 0) return color(0, 0, 0);

  hit_record rec;

  if (!objects.hit(r, interval(0.001, Infinity), rec)) {  // 0.001 tolerance for floating point rounding error
    return background;
  }

  color emission;
  if (rec.material->emitted(r, rec, emission)){
    hit_point = rec.point_incident;
    isEmissive = true;
    return emission;
  }

  color direct_lighting = color(0, 0, 0);

  for (int i = 0; i < lights.get_all().size(); i++){
    const auto& light = *lights.get_all()[i];

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

  next_ray_color = ray_color(lights, objects, scattered, max_bounces-1, nextEmissive, next_hit_point);

  return  direct_lighting + attenuation * next_ray_color;
}

ray Camera::get_ray(int i, int j) const {
  vec3 offset = sample_square();
  vec3 pixel_sample = pixel00_loc
  + ((i + offset.x()) * pixel_delta_u)
  + ((j + offset.y()) * pixel_delta_v);

  vec3 ray_origin = (dof_angle <= 0) ? camera_center: defocus_disk_sample();
  vec3 ray_direction = pixel_sample - ray_origin;

  float ray_time = random_double();

  return ray(ray_origin, ray_direction, ray_time);
}


void Camera::WriteImageToFile(const image &output_image, const std::string &filename) {
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