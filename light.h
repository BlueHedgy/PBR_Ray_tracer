#ifndef LIGHT_H
#define LIGHT_H

#include "aabb.h"
#include "quad.h"
#include "material.h"
#include <algorithm>

enum light_type {
  POINT_LIGHT,
  AREA_LIGHT,
  DIRECTIONAL_LIGHT,
  NONE
};

class light {
  public:
    virtual ~light() = default;

    light_type type = light_type::NONE;
    virtual color get_color() const { return color(0, 0, 0); }
    virtual void set_color(color new_color) const { }
    virtual vec3 get_position() const{ return vec3(INT_MIN, INT_MIN, INT_MIN); }
    virtual void set_position(vec3 new_pos) const { }
    virtual vec3 get_direction() const{ return vec3(INT_MIN, INT_MIN, INT_MIN); }
    virtual void set_direction(vec3 new_dir) const { }

};


class point_light : public light {
  public:
    point_light (vec3 position, color light_color) : position(position), light_color(light_color) {
      type = POINT_LIGHT;
    }
    color get_color() const override { return light_color; }
    vec3 get_position() const override { return position; }
    float strength;

  private:
    vec3 position;
    color light_color;
};


class area_light : public light {
  public:
    area_light (quad area, color light_color) : area(area), light_color(light_color) {
      type = light_type::AREA_LIGHT;
    }

    vec3 getCenter() { return area.center; }
    color get_color() const override { return light_color; }


  private:
    quad area;
    color light_color;
};


class directional_light : public light {
  public:
    directional_light (vec3 direction, color light_color) :
      direction(direction),
      light_color(light_color)
    {
      type = DIRECTIONAL_LIGHT;
    }

    color get_color() const override { return light_color; }
    vec3 get_direction() const override { return direction; }

  private:
    vec3 direction;
    color light_color;
};




typedef std::vector<std::shared_ptr<light>> lightList;

class light_list {

  public:
    light_list() {}

    void add(std::shared_ptr<light> l) {
        lights.push_back(l);
    }

    void remove(std::shared_ptr<light> l) {
        lights.erase(std::remove(lights.begin(), lights.end(), l), lights.end());
    }

    const lightList& get_all() const { return lights; }


  private:
    lightList lights;

};

#endif