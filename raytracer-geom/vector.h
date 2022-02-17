#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <algorithm>

#include <iostream>

class Vector;
inline double Length(const Vector&);

class Vector {
public:
    Vector() {
        data_ = {{0.0, 0.0, 0.0}};
    }

    Vector(std::initializer_list<double> list) {
        int i = 0;
        for (auto& elem : list) {
            data_[i] = elem;
            ++i;
        }
    }

    // Vector(const Vector& other) : Vector{other.data_} {
    // }

    Vector(std::array<double, 3> data) : data_{data} {
    }

    double& operator[](size_t ind) {
        return data_[ind];
    }
    double operator[](size_t ind) const {
        return data_[ind];
    }

    void Normalize() {
        auto length = Length(*this);
        for (int i = 0; i < 3; ++i) {
            data_[i] /= length;
        }
    }

    void Print() {
        std::cout << data_[0] << " " << data_[1] << " " << data_[2] << std::endl;
    }

private:
    std::array<double, 3> data_;
};

inline double DotProduct(const Vector& lhs, const Vector& rhs) {
    double output = 0;
    for (int i = 0; i < 3; ++i) {
        output += lhs[i] * rhs[i];
    }
    return output;
}
inline Vector CrossProduct(const Vector& a, const Vector& b) {
    std::array<double, 3> data;
    data[0] = a[1] * b[2] - a[2] * b[1];
    data[1] = a[2] * b[0] - a[0] * b[2];
    data[2] = a[0] * b[1] - a[1] * b[0];
    return Vector(data);
}
inline double Length(const Vector& vec) {
    double output = 0;
    for (int i = 0; i < 3; ++i) {
        output += vec[i] * vec[i];
    }
    return sqrt(output);
}

inline Vector operator+(const Vector& lhs, const Vector& rhs) {
    std::array<double, 3> data;
    data[0] = lhs[0] + rhs[0];
    data[1] = lhs[1] + rhs[1];
    data[2] = lhs[2] + rhs[2];
    return Vector(data);
}

inline Vector operator+(const Vector& lhs, double rhs) {
    std::array<double, 3> data;
    data[0] = lhs[0] + rhs;
    data[1] = lhs[1] + rhs;
    data[2] = lhs[2] + rhs;
    return Vector(data);
}

inline Vector operator-(const Vector& lhs, const Vector& rhs) {
    std::array<double, 3> data;
    data[0] = lhs[0] - rhs[0];
    data[1] = lhs[1] - rhs[1];
    data[2] = lhs[2] - rhs[2];
    return Vector(data);
}

inline Vector operator*(const Vector& lhs, double alpha) {
    std::array<double, 3> data;
    data[0] = alpha * lhs[0];
    data[1] = alpha * lhs[1];
    data[2] = alpha * lhs[2];
    return Vector(data);
}

inline Vector operator*(double alpha, const Vector& rhs) {
    return operator*(rhs, alpha);
}

inline Vector operator*(const Vector& lhs, const Vector& rhs) {
    std::array<double, 3> data;
    data[0] = lhs[0] * rhs[0];
    data[1] = lhs[1] * rhs[1];
    data[2] = lhs[2] * rhs[2];
    return Vector(data);
}