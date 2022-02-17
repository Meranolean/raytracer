#pragma once

#include <vector>
#include <vector.h>

class RotationMatrix {
public:
    RotationMatrix(const Vector& from, const Vector& to) {
        Vector direction = to - from;
        columns_.reserve(3);
        direction.Normalize();
        Vector forward = -1 * direction;
        Vector right{0, 1, 0};
        right = CrossProduct(right, forward);
        if (Length(right) < 1e-9) {
            right = Vector{1, 0, 0};
        } else {
            right.Normalize();
        }
        auto up = CrossProduct(forward, right);
        up.Normalize();
        columns_.push_back(Vector({right[0], up[0], forward[0]}));
        columns_.push_back(Vector({right[1], up[1], forward[1]}));
        columns_.push_back(Vector({right[2], up[2], forward[2]}));
    }

private:
    friend Vector operator*(const RotationMatrix&, const Vector&);
    std::vector<Vector> columns_;
};

Vector operator*(const RotationMatrix& matrix, const Vector& vector) {
    std::array<double, 3> data;
    data[0] = DotProduct(matrix.columns_[0], vector);
    data[1] = DotProduct(matrix.columns_[1], vector);
    data[2] = DotProduct(matrix.columns_[2], vector);
    return Vector(data);
}