#include "material.h"

vec3 mix (const vec3& x, const vec3& y, const vec3& a) {
  return x * (vec3(1, 1, 1) - a) + y * a;
}



vec3 pbr_material::schlick_approx_dielectric(const color& albedo, const float& metalness, float eta1, float eta2, const vec3 &view_direction, const vec3 &half_vector) const {
  // float fzero = fresnel_0_dielectric(eta1, eta2);
  float fzero = 0.4;
  vec3 f0 = vec3(fzero, fzero, fzero);
  vec3 metal = vec3(metalness, metalness, metalness);
  f0 = mix(f0, albedo, metal);
  return f0 + (vec3(1,1,1) - f0) * pow((1.0 - dot(view_direction, half_vector)), 5);
}

float pbr_material::GGX_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const {
  float alpha = roughness*roughness;
  float dot_n_h = dot(normal, half_vector);
  float x = dot_n_h > 0 ? 1: 0;
  return x * (alpha*alpha) / (Pi * pow( pow(dot_n_h, 2) * (alpha*alpha - 1) + 1, 2));
}
float pbr_material::Blinn_distribution(const float &roughness, const vec3& normal, const vec3& half_vector) const {
  float alpha = roughness*roughness;
  float ns = 2/(alpha*alpha) - 2;
  float dot_h_n = dot(half_vector, normal);
  return 1 / (Pi*alpha*alpha) * pow(dot_h_n, ns);
}

float pbr_material::Cook_Torrance_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector) const {
  auto G1 = 2 * dot(half_vector, normal) * dot(normal, view_direction) / dot(view_direction, half_vector);
  auto G2 = 2 * dot(half_vector, normal) * dot(normal, light_direction) / dot(view_direction, half_vector);

  return std::min(1.0f, std::min(G1, G2));
}

float pbr_material::GGX_Schlick_approx_Geo(float &roughness, vec3 &normal, vec3 &view_direction) {
  float k = roughness*roughness / 2.0;
  return dot(normal, view_direction) / (dot(normal, view_direction) * (1-k) + k);
}

float pbr_material::Cook_Torrance_GGX_Geo_term(const vec3& normal, const vec3& view_direction, const vec3& light_direction, const vec3& half_vector, const float &roughness) const {

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

color pbr_material::Cook_Torrance_Microfacet_BRDF(const float &D, const float &G, const vec3 &F, const vec3& normal, const vec3& light_direction, const vec3& view_direction) const {
      return D*G*F / (4 * dot(normal, light_direction) * dot(normal, view_direction) + 0.0001);
  }

color pbr_material::pbr_color(const ray& ray_in, const hit_record& rec, const color& light_color, const point3& light_direction) const {

  if (light_direction == point3(INT_MIN, INT_MIN, INT_MIN)) return color(0, 0, 0);

  color albedo_value    = albedo->value(rec.u, rec.v, rec.point_incident);
  float roughness_value = roughness->value(rec.u, rec.v, rec.point_incident)[0];
  float metal_value     = metalness->value(rec.u, rec.v, rec.point_incident)[0];
  color normal_value    = normal->value(rec.u, rec.v, rec.point_incident);
  texture_normal_mapping(rec, normal_value);
  // normal_value = microfacetNormalGGX(ray_in, rec);

  vec3 half_vector = unit_vector(light_direction + -ray_in.direction());
  float D = GGX_distribution(roughness_value, normal_value, half_vector);
  float G = Cook_Torrance_GGX_Geo_term(normal_value, -ray_in.direction(), light_direction, half_vector, roughness_value);
  vec3  F = schlick_approx_dielectric(albedo_value, metal_value, 0.58, 1.0, normal_value, half_vector);

  color specular = Cook_Torrance_Microfacet_BRDF(D, G, F, normal_value, light_direction, -ray_in.direction());

  color pbr =  ((color(1, 1, 1) - F) * (1.0 - metal_value) * albedo_value / Pi + specular) *
                light_color * std::max(dot(normal_value, light_direction), 0.0f);

  return pbr;
}


bool pbr_material::scatter (const ray& ray_in, const hit_record& rec, color& attenuation, ray& ray_scattered) const{
  color n = normal->value(rec.u, rec.v, rec.point_incident);  // macrofacet normal
  texture_normal_mapping(rec, n);
  float roughness_value = roughness->value(rec.u, rec.v, rec.point_incident)[0];

  vec3 m = microfacetNormalGGX(ray_in, rec);        // microfacet normal
  vec3 scatter_direction = 2 * std::abs(dot(-ray_in.direction(), m)) * m - -ray_in.direction();
  if (scatter_direction.near_zero()){
    scatter_direction = n;   // if scatter direction is near opposite, make it the normal
  }
  ray_scattered = ray(rec.point_incident, scatter_direction, ray_in.time());

  float cos_theta_m = dot(ray_scattered.direction(), m);
  float cos_theta_n = dot(ray_scattered.direction(), rec.normal);

  vec3 half_vector = unit_vector(ray_scattered.direction() + -ray_in.direction());
  float G = Cook_Torrance_GGX_Geo_term(m, -ray_in.direction(), ray_scattered.direction(), half_vector, roughness_value);

  float scatter_weight = std::abs(cos_theta_m) * G /
                        (std::abs(cos_theta_n) * std::abs(dot(m, n)));

  if(scatter_weight > 1) scatter_weight = 1; // portion of the energy scattered, shouldn't be > 1 (?)

  attenuation = albedo->value(rec.u, rec.v, rec.point_incident) * scatter_weight; //!TODO Need to fix this distribution

  return cos_theta_m > 0;
}


vec3 pbr_material::microfacetNormalGGX(const ray& ray_in, const hit_record& rec) const {
  float u1 = random_float();
  float u2 = random_float();
  float alpha = std::pow(roughness->value(rec.u, rec.v, rec.point_incident)[0], 2);
  alpha = std::max(alpha, 0.001f);

  float phi = 2 * Pi * u2;
  float theta = std::atan(alpha * std::sqrt(u1) / std::sqrt(1-u1));

  vec3 local_microfacet_normal = {
    std::sin(theta) * std::cos(phi),
    std::sin(theta) * std::sin(phi),
    std::cos(theta)
  };

  color normal_value = normal->value(rec.u, rec.v, rec.point_incident);
  texture_normal_mapping(rec, normal_value);

  // If normal points in world Y-axis direction, use world Z-axis as a fallback.
  // else use the world Y-axis (0, 1, 0) as the guide vector.
  vec3 guide = (std::abs(normal_value.y()) < 0.999f) ? vec3(0, 1, 0) : vec3(0, 0, 1);

  // Generate tangent vectors perpendicular to N
  vec3 T = unit_vector(cross(guide, normal_value));
  vec3 B = cross(normal_value, T);
  // -------------------------------------

  // Project the Z-up local vector into your Y-up world space
  // local_m.x scales Tangent, local_m.y scales Bitangent, local_m.z scales Normal
  vec3 world_m =  T * local_microfacet_normal.x() +
                  B * local_microfacet_normal.y() +
                  normal_value * local_microfacet_normal.z();
  return unit_vector(world_m);
}