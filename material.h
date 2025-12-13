#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"

/// @brief Abstract material class
class material {
  public:
  virtual ~material() = default;

  virtual bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const {
    return false;
  }
};

/// @brief  Simple lambertian material, always scattered
class lambertian : public material {
  public:
    lambertian (const color& albedo) : albedo (albedo){}

    bool scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const{
      vec3 scatter_direction = rec.normal + random_unit_vector();
      if (scatter_direction.near_zero()){
        scatter_direction = rec.normal;   // if scatter direction is near opposite, make it the normal
      }
      ray_scattered = ray(rec.point_incident, scatter_direction);
      attenuation = albedo;
      return true;
    }

  private:
    color albedo;
};


class metal : public material {
  public:
    metal (const color& albedo, double fuzz) : albedo (albedo), fuzz(fuzz < 1 ? fuzz : 1){}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const override {
      vec3 ray_reflected = reflect(ray_in.direction(), rec.normal);
      ray_reflected = unit_vector(ray_reflected) + (fuzz * random_unit_vector()); // a bit of randomness in reflecting rays, pseudo roughness (?)
      ray_scattered = ray(rec.point_incident, ray_reflected);
      attenuation = albedo;
      return (dot(ray_scattered.direction(), rec.normal) > 0);
    }

  private: 
    color albedo;
    double fuzz;
};

#endif