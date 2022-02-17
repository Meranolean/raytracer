#pragma once

// #include <vector.h>
#include "../raytracer-geom/vector.h"
#include <string>

struct Material {
    Material() {
    }

    Material(std::string& name) : name{name} {
        specular_exponent = 0.0;
        refraction_index = 0.0;
        albedo = {{1.0, 0.0, 0.0}};
    }

    std::string name;
    Vector ambient_color;
    Vector diffuse_color;
    Vector specular_color;
    Vector intensity;
    double specular_exponent;
    double refraction_index;
    std::array<double, 3> albedo;
};
