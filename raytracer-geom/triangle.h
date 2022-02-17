#pragma once

#include <vector.h>

class Triangle {
public:
    Triangle(std::initializer_list<Vector> list) {
        int i = 0;
        for (auto& vec : list) {
            vertices_[i] = vec;
            ++i;
        }
    }

    double Area() const {
        Vector side1 = vertices_[1] - vertices_[0];
        Vector side2 = vertices_[2] - vertices_[0];
        return Length(CrossProduct(side1, side2)) / 2;
    }

    const Vector& GetVertex(size_t ind) const {
        return vertices_[ind];
    }

private:
    std::array<Vector, 3> vertices_;
};
