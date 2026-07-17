#ifndef SCENE_H
#define SCENE_H

#include <optional>
#include <any>

#include "light.h"
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
          const scene_component_t &camerasma
    ) {
      process_input(lights);
      process_input(objects);
      process_input(cameras);
    }

    size_t active_cam_index;

    const light_list& get_lights()     const { return lights;}
    const hittable_list& get_objects() const { return objects; }
    const camera_list& get_cameras()   const { return cameras; }

    void add_object(std::shared_ptr<hittable> new_object) {
      objects.add(new_object);
    }

    void add_light(std::shared_ptr<light> new_light) {
      lights.add(new_light);
    }

    void process_object_bvh(){
      objects = hittable_list(std::make_shared<bvh_node_custom>(objects, 0, objects.objects.size()));
    }

    Camera& get_active_cam() {
      return cameras.get_cam(active_cam_index);
    }

    void Render(
      std::string &filename,
      std::atomic_bool& render_cancelled,
      image & output_image,
      display_image_data& d_imdata
    ) {
      process_object_bvh();
      get_active_cam().Render_MultiThreaded(lights, objects, filename, render_cancelled, output_image, d_imdata);
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