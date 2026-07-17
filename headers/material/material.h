#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "aabb.h"
#include <variant>
#include <cmath>

/// @brief Abstract material class
class material {
  public:
    virtual ~material() = default;
    color diffuse;
    float metallic;
    float roughness;
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

    float fresnel_reflected (float &eta1, float &eta2, float &theta1, float &theta2) const {
      float Fpara = (eta2*cos(theta1) - eta1*cos(theta2)) / (eta2*cos(theta1) + eta1*cos(theta2));
      float Fperp = (eta2*cos(theta2) - eta1*cos(theta1)) / (eta2*cos(theta2) + eta1*cos(theta1));

      return 0.5 * (powf(Fpara, 2) + powf(Fperp, 2));
    }

    float fresnel_0_dielectric(float &eta1, float &eta2) const {
      return (pow((eta2 - eta1), 2) / pow((eta2 + eta1), 2));
    }

    /// @brief Calculate fresnel term for eta1 = 1
    float fresnel_term(float &eta2, vec3 &view_direction, vec3 &half_vector) const {
      float c = dot(view_direction, half_vector);
      float g = sqrt(eta2*eta2 + c*c -1);

      return 0.5 * pow((g-c)/(g+c), 2) * (1 + pow( (c*(g+c) - 1) / (c*(g-c) +1), 2));
    }

  protected:

    void texture_normal_mapping(const hit_record &rec, vec3 &normal_value) const {
      if ((normal_value == color(INT_MIN, INT_MIN, INT_MIN))) normal_value = rec.normal;
      else {
        vec3 tangent_space_normal = unit_vector(normal_value * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0));

        normal_value =
          rec.tangent   * tangent_space_normal.x() +
          rec.bitangent * tangent_space_normal.y() +
          rec.normal    * tangent_space_normal.z()
        ;

        normal_value = unit_vector(normal_value);
      }
  }

};

class pbr_material : public material {
  public:

  typedef std::variant<color, std::shared_ptr<texture>, std::monostate> texture_map;

  pbr_material (
    const texture_map& albedo,
    const texture_map& normal,
    const texture_map& metalness,
    const texture_map& roughness
    )
    :
    albedo(process_input_texture(albedo)),
    normal(process_input_texture(normal)),
    metalness(process_input_texture(metalness)),
    roughness(process_input_texture(roughness))
    {}

    float diffuse_coeff;

    color pbr_color(const ray& ray_in, const hit_record& rec, const color& light_color, const point3& light_direction) const override;

    bool scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const override;

    vec3 microfacetNormalGGX(const ray& ray_in, const hit_record& rec) const;

  private:
    std::shared_ptr<texture> albedo, normal, metalness, roughness;

    vec3 schlick_approx_dielectric(const color& albedo, const float& metalness, float eta1, float eta2, const vec3 &view_direction, const vec3 &half_vector) const;

    float GGX_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const;

    float Blinn_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const;

    float Beckmann_distribution(float &alpha, vec3 &normal, vec3 &half_vector) {
      return 0;
    }

    float GGX_Schlick_approx_Geo(float &roughness, vec3 &normal, vec3 &view_direction);

    float Cook_Torrance_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector) const;

    float Cook_Torrance_GGX_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector, const float &roughness) const;

    color Cook_Torrance_Microfacet_BRDF(const float &D, const float &G, const vec3 &F, const vec3& normal, const vec3& light_direction, const vec3& view_direction) const;

    std::shared_ptr<texture> process_input_texture (const texture_map& map_in) const {
      if (std::holds_alternative<color>(map_in)){
        return std::make_shared<solid_color>(std::get<color>(map_in));
      }
      else if (std::holds_alternative<std::shared_ptr<texture>>(map_in)) {
        return std::get<std::shared_ptr<texture>>(map_in);
      }
      return std::make_shared<solid_color>(color(INT_MIN, INT_MIN, INT_MIN));
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
    metal (const color& albedo, float fuzz) : albedo (albedo), fuzz(fuzz < 1 ? fuzz : 1){}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const override {
      vec3 ray_reflected = reflect(ray_in.direction(), rec.normal);
      ray_reflected = unit_vector(ray_reflected) + (fuzz * random_unit_vector()); // a bit of randomness in reflecting rays, pseudo roughness (?)
      ray_scattered = ray(rec.point_incident, ray_reflected, ray_in.time());
      attenuation = albedo;
      return (dot(ray_scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo;
    float fuzz;
};


class dielectric: public material {
  public:
    dielectric(float refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& ray_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        float ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(ray_in.direction());

        float cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        float sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

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
    float refraction_index;

    static float reflectance(float cosine, float refraction_index){
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