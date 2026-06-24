#ifndef SCENE_H
#define SCENE_H

#include <optional>
#include <any>

#include "light.h"
// #include "light_list.h"
#include "hittable_list.h"
#include "hittable.h"
#include "camera.h"
#include "camera_list.h"
#include "bvh_custom.h"

typedef std::variant<std::monostate, light_list, hittable_list, camera_list> scene_component_t;

class Scene {
  public:
    Scene() = default;

    Scene(const scene_component_t &lights,
          const scene_component_t &objects,
          const scene_component_t &cameras
    ) {
      process_input(lights);
      process_input(objects);
      process_input(cameras);
    }

    lightList get_lights()     const { return lights.get_all();  }
    hittable_list get_objects() const { return objects; }
    camera_list get_cameras()   const { return cameras; }

    void add_object(std::shared_ptr<hittable> new_object) {
      objects.add(new_object);
    }

    void add_light(std::shared_ptr<light> new_light) {
      lights.add(new_light);
    }

    void process_object_bvh(){
      objects = hittable_list(std::make_shared<bvh_node_custom>(objects, 0, objects.objects.size()));
    }

  private:
    void process_input(scene_component_t const &input) {
      if (std::holds_alternative<std::monostate>(input)) { return; }
      if (std::holds_alternative<light_list>(input)) {
        lights = std::get<light_list>(input);
      }
      else if (std::holds_alternative<hittable_list>(input)) {
        objects = std::get<hittable_list>(input);
      }
      else if (std::holds_alternative<camera_list>(input)) {
        cameras = std::get<camera_list>(input);
      }
    }

    light_list lights;
    hittable_list objects;
    camera_list cameras;


};
#endif