#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "aabb.h"
#include <variant>

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

    virtual bool emitted(const ray& ray_in, const hit_record& rec, color& emitted_color) const {
      emitted_color = color(0, 0, 0);
      return false;
    }

    virtual color pbr_color(const ray& ray_in, const hit_record& rec, const color& light_color, const point3& light_pos) const {
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

    vec3 mix (const vec3& x, const vec3& y, const vec3& a) const {
      return x * (vec3(1, 1, 1) - a) + y * a;
    }

    vec3 schlick_approx_dielectric(const color& albedo, const double& metalness, double &eta1, double &eta2, const vec3 &view_direction, const vec3 &half_vector) const {
      // double fzero = fresnel_0_dielectric(eta1, eta2);
      double fzero = 0.4;
      vec3 f0 = vec3(fzero, fzero, fzero);
      vec3 metal = vec3(metalness, metalness, metalness);
      f0 = mix(f0, albedo, metal);
      return f0 + (vec3(1,1,1) - f0) * pow((1.0 - dot(view_direction, half_vector)), 5);
    }

  protected:

    double GGX_distribution(const double &roughness, const vec3& normal, const vec3& half_vector) const {
      double alpha = roughness*roughness;
      double dot_n_h = dot(normal, half_vector);
      double x = dot_n_h > 0 ? 1: 0;
      return x * (alpha*alpha) / (Pi * pow( pow(dot_n_h, 2) * (alpha*alpha - 1) + 1, 2));
    }

    double Blinn_distribution(const double &roughness, const vec3& normal, const vec3& half_vector) const {
      double alpha = roughness*roughness;
      double ns = 2/(alpha*alpha) - 2;
      double dot_h_n = dot(half_vector, normal);
      return 1 / (Pi*alpha*alpha) * pow(dot_h_n, ns);
    }

    double Beckmann_distribution(double &alpha, vec3 &normal, vec3 &half_vector) {
      return 0;
    }

    double GGX_Schlick_approx_Geo(double &roughness, vec3 &normal, vec3 &view_direction) {
      double k = roughness*roughness / 2.0;
      return dot(normal, view_direction) / (dot(normal, view_direction) * (1-k) + k);
    }

    double Cook_Torrance_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector) const {
      auto G1 = 2 * dot(half_vector, normal) * dot(normal, view_direction) / dot(view_direction, half_vector);
      auto G2 = 2 * dot(half_vector, normal) * dot(normal, light_direction) / dot(view_direction, half_vector);

      return std::min(1.00, std::min(G1, G2));
    }

    auto Cook_Torrance_Microfacet_BRDF(const color& albedo, const double& metalness, const double& roughness, double refraction_index, const vec3& normal, const vec3& view_direction, vec3& light_direction) const {
      double air_index = 1;
      vec3 half_vector = unit_vector(light_direction + view_direction);
      // double D = Blinn_distribution(roughness, normal, half_vector);
      auto D = GGX_distribution(roughness, normal, half_vector);
      double G = Cook_Torrance_Geo_term(normal, view_direction, light_direction, half_vector);
      vec3 F = schlick_approx_dielectric(albedo, metalness, refraction_index, air_index, normal, half_vector);


      return D*G*F / (4 * dot(normal, light_direction) * dot(normal, view_direction));
    }
};

class pbr_material : public material {
  public:
    double diffuse_coeff;
    point3 light_pos;

    typedef std::variant<color, std::shared_ptr<texture>> texture_map;

    pbr_material (
      const texture_map& albedo,
      const texture_map& metalness,
      const texture_map& roughness,
      const double& diffuse_coeff,
      const point3& light_pos)
      :
      albedo(process_input_texture(albedo)),
      metalness(process_input_texture(metalness)),
      roughness(process_input_texture(roughness)),
      diffuse_coeff(diffuse_coeff),
      light_pos(light_pos)
    {}

    color pbr_color(const ray& ray_in, const hit_record& rec, const color& light_color, const point3& light_pos) const override {
      color  albedo_value    = albedo->value(rec.u, rec.v, rec.point_incident);
      double roughness_value = roughness->value(rec.u, rec.v, rec.point_incident)[0];
      double metal_value     = metalness->value(rec.u, rec.v, rec.point_incident)[0];
      vec3   light_dir       = unit_vector(light_pos - rec.point_incident);
      auto   specular        = Cook_Torrance_Microfacet_BRDF(albedo_value, metal_value, roughness_value, 0.58, rec.normal, -ray_in.direction(), light_dir);

      color pbr =  (diffuse_coeff * albedo_value / Pi + specular * (1.00-diffuse_coeff)) *
                    light_color * dot(rec.normal, light_dir);

      return pbr;
    }

    bool scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const{
      vec3 scatter_direction = rec.normal + random_unit_vector();
      if (scatter_direction.near_zero()){
        scatter_direction = rec.normal;   // if scatter direction is near opposite, make it the normal
      }
      ray_scattered = ray(rec.point_incident, scatter_direction, ray_in.time());
      attenuation = pbr_color(ray_in, rec, color(1, 1, 1), light_pos);
      return true;
    }

  private:
    std::shared_ptr<texture> albedo;
    std::shared_ptr<texture> metalness;
    std::shared_ptr<texture> roughness;


    std::shared_ptr<texture> process_input_texture (const texture_map& map_in) const {
        if (std::holds_alternative<color>(map_in)){
          return std::make_shared<solid_color>(std::get<color>(map_in));
        }
        else if (std::holds_alternative<std::shared_ptr<texture>>(map_in)) {
          return std::get<std::shared_ptr<texture>>(map_in);
        }
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
    diffuse_light(std::shared_ptr<texture> tex) : tex(tex) {;}
    diffuse_light(const color& emit) : tex(std::make_shared<solid_color>(emit)) {}

    bool emitted(const ray& ray_in, const hit_record& rec, color& emitted_color) const override {
      emitted_color = tex->value(rec.u, rec.v, rec.point_incident);
      return true;
    }


  private:
    std::shared_ptr<texture> tex;
};


#endif