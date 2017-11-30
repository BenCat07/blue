#pragma once

#include <cassert>
#include <cmath>
#include <float.h>

#undef max
#undef min

namespace Math {
constexpr auto PI = 3.14159265358979323846f;

inline auto to_radians(float x) {
    return x * (PI / 180.0f);
}

inline auto to_degrees(float x) {
    return x * (180.0f / PI);
}

template <typename T>
T lerp(float percent, T min, T max) {
    return min + (percent * (max - min));
}

class Vector {
public:
    float x, y, z;

    // sentinal values
    inline static auto origin() { return Vector(0); }
    inline static auto invalid() { return Vector(FLT_MAX); }

    Vector() {}
    Vector(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector(float v) : x(v), y(v), z(v) {}
    Vector(const Vector &v) : x(v.x), y(v.y), z(v.z) {}

    // equality
    inline auto operator==(const Vector &v) const {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }
    inline auto operator!=(const Vector &v) const {
        return (x != v.x) || (y != v.y) || (z != v.z);
    }

    // arithmetic operations
    inline auto operator+=(const Vector &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    inline auto operator-=(const Vector &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    inline auto operator*=(const Vector &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    inline auto operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    inline auto operator/=(const Vector &v) {
        assert(v.x != 0.0f && v.y != 0.0f && v.z != 0.0f);
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }
    inline auto operator/=(float s) {
        assert(s != 0.0f);
        float oofl = 1.0f / s;
        x *= oofl;
        y *= oofl;
        z *= oofl;
        return *this;
    }
    inline auto operator-(void) {
        return Vector(-x, -y, -z);
    }
    inline auto operator+(const Vector &v) {
        return Vector(*this) += v;
    }
    inline auto operator-(const Vector &v) const {
        return Vector(*this) -= v;
    }
    auto operator*(const Vector &v) const {
        return Vector(*this) *= v;
    }
    inline auto operator/(const Vector &v) const {
        return Vector(*this) /= v;
    }
    inline auto operator*(float s) const {
        return Vector(*this) *= s;
    }
    inline auto operator/(float s) const {
        return Vector(*this) /= s;
    }

    inline auto length_sqr() {
        return (x * x) + (y * y) + (z * z);
    }
    inline auto length() {
        return std::sqrt(length_sqr());
    }
    inline auto distance(Vector &b) {
        auto d = *this - b;
        return d.length();
    }
    inline auto dot(Vector &b) {
        return (x * b.x) + (y * b.y) + (z * b.z);
    }
    inline auto cross(Vector &v) {
        auto res = Vector::invalid();
        res.x    = y * v.z - z * v.y;
        res.y    = z * v.x - x * v.z;
        res.z    = x * v.y - y * v.x;
        return res;
    }

    inline auto to_angle() {
        float tmp, yaw, pitch;

        if (y == 0 && x == 0) {
            yaw = 0;
            if (z > 0)
                pitch = 270;
            else
                pitch = 90;
        } else {
            yaw = (atan2(y, x) * 180 / PI);
            if (yaw < 0)
                yaw += 360;

            tmp   = sqrt(x * x + y * y);
            pitch = (atan2(-z, tmp) * 180 / PI);
            if (pitch < 0)
                pitch += 360;
        }

        return Vector(pitch, yaw, 0);
    }

    inline auto to_Vector() {
        float sp, sy, cp, cy;

        sy = sinf(to_radians(y));
        cy = cosf(to_radians(y));

        sp = sinf(to_radians(x));
        cp = cosf(to_radians(x));

        return Vector(cp * cy, cp * sy, -sp);
    }
};
} // namespace Math
