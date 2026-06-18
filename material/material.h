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

    vec3 mix (const vec3& x, const vec3& y, const vec3& a) const {
      return x * (vec3(1, 1, 1) - a) + y * a;
    }


    protected:

    vec3 schlick_approx_dielectric(const color& albedo, const float& metalness, float eta1, float eta2, const vec3 &view_direction, const vec3 &half_vector) const {
      // float fzero = fresnel_0_dielectric(eta1, eta2);
      float fzero = 0.4;
      vec3 f0 = vec3(fzero, fzero, fzero);
      vec3 metal = vec3(metalness, metalness, metalness);
      f0 = mix(f0, albedo, metal);
      return f0 + (vec3(1,1,1) - f0) * pow((1.0 - dot(view_direction, half_vector)), 5);
    }

    float GGX_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const {
      float alpha = roughness*roughness;
      float dot_n_h = dot(normal, half_vector);
      float x = dot_n_h > 0 ? 1: 0;
      return x * (alpha*alpha) / (Pi * pow( pow(dot_n_h, 2) * (alpha*alpha - 1) + 1, 2));
    }

    float Blinn_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const {
      float alpha = roughness*roughness;
      float ns = 2/(alpha*alpha) - 2;
      float dot_h_n = dot(half_vector, normal);
      return 1 / (Pi*alpha*alpha) * pow(dot_h_n, ns);
    }

    float Beckmann_distribution(float &alpha, vec3 &normal, vec3 &half_vector) {
      return 0;
    }

    float GGX_Schlick_approx_Geo(float &roughness, vec3 &normal, vec3 &view_direction) {
      float k = roughness*roughness / 2.0;
      return dot(normal, view_direction) / (dot(normal, view_direction) * (1-k) + k);
    }

    float Cook_Torrance_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector) const {
      auto G1 = 2 * dot(half_vector, normal) * dot(normal, view_direction) / dot(view_direction, half_vector);
      auto G2 = 2 * dot(half_vector, normal) * dot(normal, light_direction) / dot(view_direction, half_vector);

      return std::min(1.0f, std::min(G1, G2));
    }

    float Cook_Torrance_GGX_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector, const float &roughness) const {

      float alpha = roughness*roughness;

      float dot_v_h_n = dot(view_direction, half_vector) / dot(view_direction, normal);
      float dot_l_h_n = dot(light_direction, half_vector) / dot(light_direction, normal);

      float G1 = 0, G2 = 0;

      if (dot_v_h_n > 0){
        float tan_sqrd_v_n = 1 / pow(dot(view_direction, normal), 2.0) - 1;
        G1 = 2 / (1 + std::sqrt(1 + alpha*alpha * tan_sqrd_v_n));
      }

      if (dot_l_h_n > 0){
        float tan_sqrd_l_n = 1 / pow(dot(light_direction, normal), 2.0) - 1;
        G2 =  2 / (1 + std::sqrt(1 + alpha*alpha * tan_sqrd_l_n));
      }

      return G1*G2;
    }

    color Cook_Torrance_Microfacet_BRDF(const float &D, const float &G, const vec3 &F, const vec3& normal, const vec3& light_direction, const vec3& view_direction) const {
      return D*G*F / (4 * dot(normal, light_direction) * dot(normal, view_direction) + 0.0001);
    }
};

class pbr_material : public material {
  public:
    float diffuse_coeff;
    point3 light_pos;

    typedef std::variant<color, std::shared_ptr<texture>, std::monostate> texture_map;

    pbr_material (
      const texture_map& albedo,
      const texture_map& normal,
      const texture_map& metalness,
      const texture_map& roughness,
      const float& diffuse_coeff,
      const point3& light_pos)
      :
      albedo(process_input_texture(albedo)),
      normal(process_input_texture(normal)),
      metalness(process_input_texture(metalness)),
      roughness(process_input_texture(roughness)),
      diffuse_coeff(diffuse_coeff),
      light_pos(light_pos)
    {}

    color pbr_color(const ray& ray_in, const hit_record& rec, const color& light_color, const point3& light_pos) const override {
      color albedo_value    = albedo->value(rec.u, rec.v, rec.point_incident);
      float roughness_value = roughness->value(rec.u, rec.v, rec.point_incident)[0];
      float metal_value     = metalness->value(rec.u, rec.v, rec.point_incident)[0];
      color normal_value    = normal->value(rec.u, rec.v, rec.point_incident);
      vec3  light_direction = unit_vector(light_pos - rec.point_incident);

      process_normal_value(rec, normal_value);

      vec3 half_vector = unit_vector(light_direction + -ray_in.direction());
      float D = GGX_distribution(roughness_value, normal_value, half_vector);
      float G = Cook_Torrance_GGX_Geo_term(normal_value, -ray_in.direction(), light_direction, half_vector, roughness_value);
      vec3  F = schlick_approx_dielectric(albedo_value, metal_value, 0.58, 1.0, normal_value, half_vector);

      color specular = Cook_Torrance_Microfacet_BRDF(D, G, F, normal_value, light_direction, -ray_in.direction());

      color pbr =  ((color(1, 1, 1) - F) * (1.0 - metal_value) * albedo_value / Pi + specular) *
                    light_color * dot(rec.normal, light_direction);

      return pbr;
    }

    bool scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const{
      // if (dot(-ray_in.direction(), rec.normal) < 0){
      //   attenuation = color(0, 0, 0);
      //   return false;
      // }

      color  normal_value    = normal->value(rec.u, rec.v, rec.point_incident);
      process_normal_value(rec, normal_value);

      vec3 scatter_direction = normal_value + random_unit_vector();
      if (scatter_direction.near_zero()){
        scatter_direction = normal_value;   // if scatter direction is near opposite, make it the normal
      }
      ray_scattered = ray(rec.point_incident, scatter_direction, ray_in.time());
      attenuation = pbr_color(ray_in, rec, color(2, 2, 2), light_pos);
      return true;
    }

  private:
    std::shared_ptr<texture> albedo;
    std::shared_ptr<texture> normal;
    std::shared_ptr<texture> metalness;
    std::shared_ptr<texture> roughness;


    std::shared_ptr<texture> process_input_texture (const texture_map& map_in) const {
      if (std::holds_alternative<color>(map_in)){
        return std::make_shared<solid_color>(std::get<color>(map_in));
      }
      else if (std::holds_alternative<std::shared_ptr<texture>>(map_in)) {
        return std::get<std::shared_ptr<texture>>(map_in);
      }
      return std::make_shared<solid_color>(color(INT_MIN, INT_MIN, INT_MIN));
    }

    void process_normal_value(const hit_record &rec, vec3 &normal_value) const {
      if ((normal_value == color(INT_MIN, INT_MIN, INT_MIN))) normal_value = rec.normal;
      else {
        vec3 tangent_space_normal = unit_vector(normal_value * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0));
        normal_value =
          rec.tangent   * tangent_space_normal.x() +
          rec.bitangent * -tangent_space_normal.y() +
          rec.normal    * tangent_space_normal.z()
        ;

        normal_value = unit_vector(normal_value);
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