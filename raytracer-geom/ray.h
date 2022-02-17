#pragma once

#include <vector.h>

class Ray {
public:
    Ray(Vector origin, Vector direction)
        : origin_{std::move(origin)}, direction_{std::move(direction)} {
        direction_.Normalize();
    }

    const Vector& GetOrigin() const {
        return origin_;
    }

    const Vector& GetDirection() const {
        return direction_;
    }

private:
    Vector origin_;
    Vector direction_;
};
