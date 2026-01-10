#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "ray.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"

struct camera_params{
    int image_width; 
    double aspect_ratio, viewport_height, focal_length;
};

double hit_sphere(const point3& center, double radius, const ray& r) {
    vec3 oc = center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(),oc); // b = -2h
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = h*h - a*c;
    // auto b = -2.0 * dot(r.direction(), oc);
    // auto discriminant = b*b - 4*a*c;

    if (discriminant < 0) {
        return -1.0;
    }
    return (h - std::sqrt(discriminant) ) / a;
}

color ray_color(const ray& r, const hittable& world, int max_bounces, double reflectance_coeff){
    hit_record rec;
    if (max_bounces <= 0) return color(0, 0, 0);
    if (world.hit(r, interval(0.001, Infinity), rec)){  // 0.001 tolerance for floating point rounding error
        // vec3 direction = random_uniform_on_hemisphere(rec.normal);
        vec3 direction = rec.normal + random_unit_vector();         // lambertian diffuse

        ray scattered;
        color attenuation;

        if (rec.material->scatter(r, rec, attenuation, scattered)){
            return attenuation * ray_color(scattered, world, max_bounces-1, reflectance_coeff);
        }
        // return reflectance_coeff * ray_color(ray(rec.point_incident, direction), world, max_bounces - 1, reflectance_coeff);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}

class Camera {
  public:
    bool enableAA = true;   // Enable Anti-aliasing, default : true
    int sample_per_pixel = 4;
    int max_bounces = 10;
    double reflectance_coeff = 0.5;
    double verticalFOV = 90;

    point3 look_from = point3(0, 0, 0);
    point3 look_at   = point3(0, 0, -1);
    vec3   world_up  = vec3(0, 1, 0);

    double dof_angle = 0;
    double focus_dist = 10;


    Camera(int imWidth, double aspectRatio, double viewportHeight, double focalLength, hittable_list& world_)
        : world(world_)
    {
        image_width = imWidth;
        aspect_ratio = aspectRatio;
        viewport_height = viewportHeight;
        // focal_length = focalLength;
    }
    
    int ImageWidth(){ return image_width; }
    int ImageHeight(){ return image_height; }

    vec3 getCenter(){ return camera_center; }
    void setCenter(vec3 newPosition){ camera_center = newPosition; }

    double getAspectRatio(){ return aspect_ratio; }

    vec3 ViewportU(){ return viewport_u; }
    vec3 ViewportV(){ return viewport_v; }
    
    vec3 Pixel_DeltaU(){ return pixel_delta_u; }
    vec3 Pixel_DeltaV(){ return pixel_delta_v; }

    vec3 Pixel00_Loc(){ return pixel00_loc; }
    vec3 VP_Upper_Left(){ return viewport_upper_left; }


    void Render(){
        initialize();

        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                if (enableAA){
                    color pixel_color = color(0, 0, 0);
                    process_ray_samples(i, j, pixel_color);
                    write_color(std::cout,  pixel_samples_scale * pixel_color);
                }
                else{
                    vec3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    vec3 ray_direction = pixel_center - camera_center;
                    ray r(camera_center, ray_direction);
                    
                    color pixel_color = ray_color(r, world, max_bounces, reflectance_coeff);
                    write_color(std::cout, pixel_color);
                }
            }
        }

        std::clog << "\rDone.                 \n";
    }

    void process_ray_samples(int i, int j, color &pixel_color){
        for (int sample = 0; sample < sample_per_pixel; sample++){
            ray r = get_ray(i, j);
            pixel_color += ray_color(r, world, max_bounces, reflectance_coeff);
        }
    }

    
    private:

    int image_width, image_height;
    vec3 camera_center;

    double aspect_ratio, focal_length;
    double viewport_width, viewport_height;

    vec3 viewport_u, viewport_v;
    vec3 pixel_delta_u, pixel_delta_v;
 
    vec3 viewport_upper_left, pixel00_loc;
    hittable_list& world;

    double pixel_samples_scale;         // Color scale factor for a sum of pixel samples

    vec3 cam_u, cam_v, cam_w;
    vec3 defocus_disk_u, defocus_disk_v;

    /// @brief Get integer image height based on the aspect ratio of the image
    int get_imageHeight (int imWidth, double aspectRatio){
        int image_height = int (imWidth/ aspectRatio);
        return image_height = (image_height < 1) ? 1 : image_height;
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
        // focal_length = (look_at - look_from).length();
        double theta = degrees_to_radians(verticalFOV);
        double h = std::tan(theta/2);
        double viewport_height = 2 * h * focus_dist;
        viewport_width = viewport_height * (double(image_width)/image_height);
        
        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        // viewport_u = vec3(viewport_width, 0, 0);
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


    /// @brief Create a ray from camera origin to randomly sampled location around pixel i, j 
    ray get_ray(int i, int j) const{
        vec3 offset = sample_square();
        vec3 pixel_sample = pixel00_loc
                    + ((i + offset.x()) * pixel_delta_u)
                    + ((j + offset.y()) * pixel_delta_v);

        vec3 ray_origin = (dof_angle <= 0) ? camera_center: defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;

        double ray_time = random_double();

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
