#ifndef QUAD_H
#define QUAD_H

#include "hittable.h"

class quad : public hittable {
  public:
    
  private:
    point3 Q;
    vec3 u, v;
    std::shared_ptr<material> mat;
    aabb bbox;
};

#endif