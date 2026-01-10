#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
  public:
    // Stationary sphere
    sphere(const point3& static_center, double radius, std::shared_ptr<material> mat) : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {
      vec3 rvec = vec3(radius, radius, radius);   
    }

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, double radius, std::shared_ptr<material> mat) :center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {}

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

      return true;
    }

  private:
    // point3 center;
    ray center;
    double radius;
    std::shared_ptr<material> mat;
};

#endif