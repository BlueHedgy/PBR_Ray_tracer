#ifndef BVH_CUSTOM_H
#define BVH_CUSTOM_H

#include "hittable.h"
#include "hittable_list.h"
#include <algorithm>

class bvh_node_custom : public hittable {
  public:
    bvh_node_custom(hittable_list& world, size_t start, size_t end) {

    std::vector<std::shared_ptr<hittable>> objects = world.objects;

    bbox = aabb::empty;
      for (size_t object_index = start; object_index < end; object_index++){
        bbox = aabb(bbox, objects[object_index]->bounding_box());
      }

      int axis = bbox.longest_axis();

      auto comparator = (axis == 0) ? box_x_compare :
                        (axis == 1) ? box_y_compare :
                                      box_z_compare;

      size_t objects_count = end - start;
      if (objects_count == 1) left = right = objects[start];
      else if (objects_count == 2) {
        left = objects[start];
        right = objects[start+1];
      }
      else{  // sort the pointer to the object list within the aabb
        std::sort(
          std::begin(objects) + start,
          std::begin(objects) + end,
          comparator);
        left;
        right;

        int mid = start + objects_count/2;

        left = std::make_shared<bvh_node_custom>(world, start, mid);
        right = std::make_shared<bvh_node_custom>(world, mid, end);
      }

      bbox = aabb(left->bounding_box(), right->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
      if (!bbox.hit(r, ray_t))
        return false;

      bool hit_left = left->hit(r, ray_t, rec);
      bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

      return hit_left || hit_right;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    // hittable_list& world;
    aabb bbox;
    std::shared_ptr<hittable> left, right;

    static bool box_compare(std::shared_ptr<hittable> a, std::shared_ptr<hittable> b, int axis) {
      // compare the bouding box minimum based on axis index here
      int min_axis_interval_a = a->bounding_box().axis_interval(axis).min;
      int min_axis_interval_b = b->bounding_box().axis_interval(axis).min;

      return min_axis_interval_a < min_axis_interval_b;
    }

    static bool box_x_compare(std::shared_ptr<hittable> a, std::shared_ptr<hittable> b) {
      return box_compare(a, b, 0);
    }

    static bool box_y_compare(std::shared_ptr<hittable> a, std::shared_ptr<hittable> b) {
      return box_compare(a, b, 1);
    }

    static bool box_z_compare(std::shared_ptr<hittable> a, std::shared_ptr<hittable> b) {
      return box_compare(a, b, 2);
    }

};

#endif
