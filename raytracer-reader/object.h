#pragma once

#include <triangle.h>
#include <material.h>
#include <sphere.h>

struct Object {
    Object(const Material* material, Triangle triangle) : material{material}, polygon{triangle} {
    }

    const Material* material = nullptr;
    Triangle polygon;

    std::vector<std::optional<Vector>> normals;

    void AddNormal(Vector normal) {
        normals.push_back(normal);
    }

    const Vector* GetNormal(size_t index) const {
        if (normals[index]) {
            return &normals[index].value();
        } else {
            return nullptr;
        }
    }

    // ~Object() {
    //     for (size_t i = 0; i < normals.size(); ++i) {
    //         delete normals[i];
    //     }
    // }
};

struct SphereObject {
    SphereObject(const Material* material, Vector center, double r)
        : material{material}, sphere{center, r} {
        // sphere = Sphere(center, r);
    }
    const Material* material = nullptr;
    Sphere sphere;
};
