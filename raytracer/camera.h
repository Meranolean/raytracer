#pragma once

#include <vector.h>
#include <camera_options.h>
#include <rotation_matrix.h>

class Camera {
public:
    Camera(const CameraOptions* options)
        : options_{options}, matrix_{options->look_from, options->look_to} {
        height_ = 2 * tan(options->fov / 2);
        width_ = height_ / options->screen_height * options->screen_width;
    }

    Vector GetDirection(int i, int j) {
        double x = i + 0.5, y = -j - 0.5;
        x = 2 * x / options_->screen_width - 1;
        y = 2 * y / options_->screen_height + 1;
        x = x * width_ / 2;
        y = y * height_ / 2;
        Vector direction{x, y, -1};
        auto output = matrix_ * direction;
        output.Normalize();
        return output;
    }

private:
    const CameraOptions* options_;
    RotationMatrix matrix_;
    double width_, height_;
};