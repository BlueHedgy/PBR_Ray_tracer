#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"


class sphere : public hittable {
  public:
    // Stationary sphere
    sphere(const point3& static_center, double radius, std::shared_ptr<material> mat) :
      center(static_center, vec3(0,0,0)),
      radius(std::fmax(0,radius)),
      mat(mat)
    {
      vec3 rvec = vec3(radius, radius, radius);
      bbox = aabb(static_center - rvec, static_center + rvec);
    }

    aabb bounding_box() const override { return bbox;}

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, double radius, std::shared_ptr<material> mat) :
      center(center1, center2 - center1),
      radius(std::fmax(0,radius)),
      mat(mat)
    {
      auto rvec = vec3(radius, radius, radius);
      aabb box1(center.at(0) - rvec, center.at(0) + rvec);
      aabb box2(center.at(1) - rvec, center.at(1) + rvec);

      bbox = aabb(box1, box2);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
      point3 current_center = center.at(r.time());

      vec3 oc = current_center - r.origin();
      auto a = r.direction().length_squared();
      auto h = dot(r.direction(), oc);
      auto c = oc.length_squared() - radius*radius;

      auto discriminant = h*h - a*c;
      if (discriminant < 0)
        return false;

      auto sqrtd = std::sqrt(discriminant);

      // Find the nearest root that lies in the acceptable range.
      auto root = (h - sqrtd) / a;
      if (!ray_t.surrounds(root)) {
        root = (h + sqrtd) / a;
        if (!ray_t.surrounds(root))
          return false;
      }

      rec.t = root;
      rec.point_incident = r.at(rec.t);
      rec.material = mat;
      vec3 outward_normal = (rec.point_incident - current_center) / radius;

      rec.set_face_normal(r, outward_normal);
      rec.tangent = compute_tangent(outward_normal, current_center);
      rec.bitangent = compute_bitangent(outward_normal, rec.tangent);

      get_sphere_uv(outward_normal, rec.u, rec.v);

      return true;
    }

  private:
    // point3 center;
    ray center;
    double radius;
    std::shared_ptr<material> mat;
    aabb bbox;


    static void get_sphere_uv(const point3& p, double& u, double& v) {
      auto theta = std::acos(p.y());
      auto phi = std::atan2(p.z(), -p.x());

      u = phi / (2*Pi);
      v = theta / Pi;
    }

    vec3 compute_tangent(const vec3 &p, const vec3 &current_center) const {
      return unit_vector(vec3(p.z(), 0, -p.x()));
    }

    vec3 compute_bitangent(const vec3 &normal, const vec3 &tangent) const {
      return unit_vector(cross(normal, tangent));
    }
};

#endif