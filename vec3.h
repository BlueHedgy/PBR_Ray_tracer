#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class vec3 {
  public:
    float e[3];

    vec3() : e{0,0,0} {}
    vec3(float e0, float e1, float e2) : e{e0, e1, e2} {}
    vec3(float e0) : e{e0, e0, e0} {}

    float x() const { return e[0]; }
    float y() const { return e[1]; }
    float z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    float operator[](int i) const { return e[i]; }
    float& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*=(float t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(float t) {
        return *this *= 1/t;
    }

    vec3& operator=(const vec3& v) {
        e[0] = v.e[0];
        e[1] = v.e[1];
        e[2] = v.e[2];
        return *this;
    }

    float length() const {
        return std::sqrt(length_squared());
    }

    float length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    static vec3 random(){
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(float min, float max) {
        return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
    }

    /// @brief
    bool near_zero() const {
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;


// Vector Utility Functions

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(float t, const vec3& v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3& v, float t) {
    return t * v;
}

inline vec3 operator/(const vec3& v, float t) {
    return (1/t) * v;
}

inline float dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
    + u.e[1] * v.e[1]
    + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

/// @brief randomize a vector in range [-1,1], reject if end point is outside of a unit sphere
/// @return
inline vec3 random_unit_vector() {
    while (true) {
        auto p = vec3::random(-1,1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1) // handle floating point error, avoid normalized vector being zero when values are too small
            return p / sqrt(lensq);
    }
}

inline vec3 random_uniform_on_hemisphere(const vec3 &normal){
    vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0) return on_unit_sphere;
    else return -on_unit_sphere;
}

inline vec3 reflect(const vec3& ray_in, const vec3& normal) {
    return ray_in - 2 * dot(ray_in, normal) * normal;
}

/// @param refract_index_ratio: index in over index out
inline vec3 refract(const vec3&ray_in, const vec3&normal, float refract_index_ratio ) {
    float cos_theta = std::fmin(dot(-ray_in, normal), 1.0);
    vec3 ray_out_perp = refract_index_ratio*(ray_in + cos_theta * normal);
    vec3 ray_out_para = -std::sqrt(std::fabs(1.0 - std::fabs(ray_out_perp.length_squared()))) * normal;

    return ray_out_perp + ray_out_para;
}

/// @brief Randomize a point within a unit circle
inline vec3 random_in_unit_disk() {
    while (true) {
        auto p = vec3(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline bool operator==(const vec3& u, const vec3& v) {
    return u.e[0] == v.e[0] && u.e[1] == v.e[1] && u.e[2] == v.e[2];
}


#endif