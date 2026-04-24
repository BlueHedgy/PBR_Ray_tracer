#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "aabb.h"

/// @brief Abstract material class
class material {
  public:
    virtual ~material() = default;
    color diffuse;
    double metallic;
    double roughness;
    float refraction_index = 1;

    virtual bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered) const {
      return false;
    }

    virtual color emitted(double u, double v, const point3& p) const {
      return color(0, 0, 0);
    }

  private:

    double fresnel_reflected (double &eta1, double &eta2, double &theta1, double &theta2) const {
      double Fpara = (eta2*cos(theta1) - eta1*cos(theta2)) / (eta2*cos(theta1) + eta1*cos(theta2));
      double Fperp = (eta2*cos(theta2) - eta1*cos(theta1)) / (eta2*cos(theta2) + eta1*cos(theta1));

      return 0.5 * (powf(Fpara, 2) + powf(Fperp, 2));
    }

    double fresnel_0_dielectric(double &eta1, double &eta2) const {
      return (pow((eta2 - eta1), 2) / pow((eta2 + eta1), 2));
    }

    /// @brief Calculate fresnel term for eta1 = 1
    double fresnel_term(double &eta2, vec3 &view_direction, vec3 &half_vector) const {
      double c = dot(view_direction, half_vector);
      double g = sqrt(eta2*eta2 + c*c -1);

      return 0.5 * pow((g-c)/(g+c), 2) * (1 + pow( (c*(g+c) - 1) / (c*(g-c) +1), 2));
    }

    double schlick_approx_dielectric(double &eta1, double &eta2, vec3 &normal, vec3 &half_vector) const {
      double f0 = fresnel_0_dielectric(eta1, eta2);
      return f0 + (1.0 - f0) * pow((1.0 - dot(normal, half_vector)), 5);
    }

  protected:

    double GGX_distribution(double &roughness, vec3 &normal, vec3 &half_vector) const {
      double alpha = roughness*roughness;
      return (alpha*alpha) / (Pi * pow( pow(dot(normal, half_vector), 2) * (alpha*alpha - 1) + 1, 2));
    }

    double Blinn_distribution(double &roughness, vec3 &normal, vec3 &half_vector) const {
      double alpha = roughness*roughness;
      double ns = 2*pow(alpha, -2) - 2;
      return 1 / (Pi*alpha*alpha) * dot(normal, half_vector);
    }

    double Beckmann_distribution(double &alpha, vec3 &normal, vec3 &half_vector) {
      return 0;
    }

    double GGX_Schlick_approx_Geo(double &roughness, vec3 &normal, vec3 &view_direction) {
      double k = roughness*roughness / 2.0;
      return dot(normal, view_direction) / (dot(normal, view_direction) * (1-k) + k);
    }

    auto Cook_Torrance_Microfacet_BRDF() {
      return 1;
    }
};



/// @brief  Simple lambertian material, always scattered
class lambertian : public material {
  public:
    // lambertian (const color& albedo) : albedo (albedo){}
    lambertian (const color& albedo) : texture(std::make_shared<solid_color>(albedo)) {}
    lambertian (std::shared_ptr<texture>  texture) : texture(texture) {}

    bool scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const{
      vec3 scatter_direction = rec.normal + random_unit_vector();
      if (scatter_direction.near_zero()){
        scatter_direction = rec.normal;   // if scatter direction is near opposite, make it the normal
      }
      ray_scattered = ray(rec.point_incident, scatter_direction, ray_in.time());
      attenuation = texture->value(rec.u, rec.v, rec.point_incident);
      return true;
    }

  private:
    std::shared_ptr<texture> texture;
};


class metal : public material {
  public:
    metal (const color& albedo, double fuzz) : albedo (albedo), fuzz(fuzz < 1 ? fuzz : 1){}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const override {
      vec3 ray_reflected = reflect(ray_in.direction(), rec.normal);
      ray_reflected = unit_vector(ray_reflected) + (fuzz * random_unit_vector()); // a bit of randomness in reflecting rays, pseudo roughness (?)
      ray_scattered = ray(rec.point_incident, ray_reflected, ray_in.time());
      attenuation = albedo;
      return (dot(ray_scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};


class dielectric: public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(ray_in.direction());

        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0; // if true, MUST reflect
        vec3 reflect_direction;
        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            reflect_direction = reflect(unit_direction, rec.normal);
        else
            reflect_direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.point_incident, reflect_direction, ray_in.time());
        return true;
    }

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double reflectance(double cosine, double refraction_index){
      // Shlick's approximation for reflectance
      auto r0 = (1 - refraction_index) / (1 + refraction_index);
      r0 = r0 * r0;
      return r0 + (1-r0) * std::pow((1 - cosine), 5);
    }
};


class diffuse_light : public material {
  public:
    diffuse_light(std::shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(std::make_shared<solid_color>(emit)) {}

    color emitted(double u, double v, const point3& p) const override {
      return tex->value(u, v, p);
    }

  private:
    std::shared_ptr<texture> tex;
};


class meta_material : public material {
  public:
    meta_material(){}
};

#endif