#pragma once

#include <image.h>
#include <camera_options.h>
#include <render_options.h>
#include <string>
#include <scene.h>
#include <camera.h>
#include <geometry.h>

Image RenderDepth(const std::string& filename, const CameraOptions& camera_options) {
    const auto scene = ReadScene(filename);
    Image output(camera_options.screen_width, camera_options.screen_height);
    std::vector<std::vector<double>> prepixels{
        static_cast<size_t>(camera_options.screen_width),
        std::vector<double>(static_cast<size_t>(camera_options.screen_height))};
    Camera camera(&camera_options);
    double max_value = -1.0;
    for (int i = 0; i < output.Width(); ++i) {
        for (int j = 0; j < output.Height(); ++j) {
            Ray ray{camera_options.look_from, camera.GetDirection(i, j)};
            for (auto& obj : scene.GetObjects()) {
                auto inter = GetIntersection(ray, obj.polygon);
                if (inter) {
                    auto value = inter.value().GetDistance();
                    if (prepixels[i][j] == 0) {
                        prepixels[i][j] = value;
                    } else {
                        prepixels[i][j] = std::min(prepixels[i][j], value);
                    }
                }
            }
            for (auto& sphere : scene.GetSphereObjects()) {
                auto inter = GetIntersection(ray, sphere.sphere);
                if (inter) {
                    auto value = inter.value().GetDistance();
                    if (prepixels[i][j] == 0) {
                        prepixels[i][j] = value;
                    } else {
                        prepixels[i][j] = std::min(prepixels[i][j], value);
                    }
                }
            }
        }
    }

    for (int i = 0; i < output.Width(); ++i) {
        for (int j = 0; j < output.Height(); ++j) {
            max_value = std::max(max_value, prepixels[i][j]);
        }
    }

    for (int i = 0; i < output.Width(); ++i) {
        for (int j = 0; j < output.Height(); ++j) {
            RGB rgb;
            if (prepixels[i][j] == 0) {
                rgb = RGB{255, 255, 255};
            } else {
                rgb = RGB(round(255 * prepixels[i][j] / max_value));
            }
            output.SetPixel(rgb, j, i);
        }
    }

    return output;
}

inline Vector ComputeNormal(const Object& obj, const Vector& point, const Vector& default_normal) {
    auto coordinates = GetBarycentricCoords(obj.polygon, point);
    Vector output;
    for (int i = 0; i < 3; ++i) {
        if (obj.GetNormal(i)) {
            output = output + *obj.GetNormal(i) * coordinates[i];
        } else {
            output = output + default_normal * coordinates[i];
        }
    }
    return output;
}

Image RenderNormal(const std::string& filename, const CameraOptions& camera_options) {
    const auto scene = ReadScene(filename);
    Image output(camera_options.screen_width, camera_options.screen_height);
    Camera camera(&camera_options);
    for (int i = 0; i < output.Width(); ++i) {
        for (int j = 0; j < output.Height(); ++j) {
            Ray ray{camera_options.look_from, camera.GetDirection(i, j)};
            RGB rgb{0, 0, 0};
            double min_dist = std::numeric_limits<double>::max();
            for (auto& obj : scene.GetObjects()) {
                auto inter = GetIntersection(ray, obj.polygon);
                if (inter and inter.value().GetDistance() < min_dist) {
                    min_dist = inter.value().GetDistance();
                    auto normal =
                        ComputeNormal(obj, inter.value().GetPosition(), inter.value().GetNormal());
                    normal = normal * 0.5 + 0.5;
                    rgb.r = 255 * normal[0];
                    rgb.g = 255 * normal[1];
                    rgb.b = 255 * normal[2];
                }
            }
            for (auto& sphere : scene.GetSphereObjects()) {
                auto inter = GetIntersection(ray, sphere.sphere);
                if (inter and inter.value().GetDistance() < min_dist) {
                    min_dist = inter.value().GetDistance();
                    auto normal = inter.value().GetNormal();
                    normal = normal * 0.5 + 0.5;
                    rgb.r = 255 * normal[0];
                    rgb.g = 255 * normal[1];
                    rgb.b = 255 * normal[2];
                }
            }
            output.SetPixel(rgb, j, i);
        }
    }
    return output;
}

bool IsVisible(const Light& light, const Vector& position, const Scene& scene) {
    auto dir = light.position - position;
    auto length = Length(dir);
    dir.Normalize();
    Ray ray{position, dir};
    for (const auto& obj : scene.GetObjects()) {
        auto inter = GetIntersection(ray, obj.polygon);
        if (inter and inter.value().GetDistance() < length) {
            return false;
        }
    }
    for (const auto& sphere : scene.GetSphereObjects()) {
        auto inter = GetIntersection(ray, sphere.sphere);
        if (inter and inter.value().GetDistance() < length) {
            return false;
        }
    }
    return true;
}

Vector ComputeLightedColor(const Material* material, const Intersection& inter, const Ray& ray,
                           const Scene& scene) {
    Vector output;
    for (const auto& light : scene.GetLights()) {
        if (IsVisible(light, inter.GetPosition(), scene)) {
            auto vl = light.position - inter.GetPosition();
            vl.Normalize();
            auto normal_x_vl = DotProduct(inter.GetNormal(), vl);
            output =
                output + material->diffuse_color * light.intensity * std::max(0.0, normal_x_vl);
            auto vr = 2 * normal_x_vl * inter.GetNormal() - vl;
            auto ve = -1 * ray.GetDirection();
            output =
                output + material->specular_color * light.intensity *
                             pow(std::max(0.0, DotProduct(ve, vr)), material->specular_exponent);
        }
    }
    return output;
}

std::optional<std::pair<Intersection, const Material*>> FindIntersection(const Ray& ray,
                                                                         const Scene& scene) {
    double min_dist = std::numeric_limits<double>::max();
    std::optional<std::pair<Intersection, const Material*>> best;
    for (auto& obj : scene.GetObjects()) {
        auto inter = GetIntersection(ray, obj.polygon);
        if (inter and inter.value().GetDistance() < min_dist) {
            min_dist = inter.value().GetDistance();
            Intersection new_inter{
                inter.value().GetPosition(),
                ComputeNormal(obj, inter.value().GetPosition(), inter.value().GetNormal()),
                inter.value().GetDistance()};
            best.emplace(new_inter, obj.material);
        }
    }
    for (auto& sphere : scene.GetSphereObjects()) {
        auto inter = GetIntersection(ray, sphere.sphere);
        if (inter and inter.value().GetDistance() < min_dist) {
            min_dist = inter.value().GetDistance();
            best.emplace(inter.value(), sphere.material);
        }
    }
    return best;
}

Vector ComputeColor(const Material* material, const Ray& ray, const Intersection& inter, int depth,
                    const Scene& scene, bool inside) {
    if (depth < 1) {
        return Vector();
    }
    double eps = 10e-5;

    Vector output = material->ambient_color + material->intensity;
    output = output + material->albedo[0] * ComputeLightedColor(material, inter, ray, scene);
    if (material->albedo[1] > 0 and depth > 1 and !inside) {
        if (inside) {
            std::cout << "ERROR" << std::endl;
        }
        auto reflect = Reflect(ray.GetDirection(), inter.GetNormal());
        Ray reflect_ray{inter.GetPosition(), reflect};
        auto reflect_inter = FindIntersection(reflect_ray, scene);
        if (reflect_inter) {
            output = output + material->albedo[1] * ComputeColor(reflect_inter.value().second,
                                                                 reflect_ray,
                                                                 reflect_inter.value().first,
                                                                 depth - 1, scene, inside);
        }
    }
    if (material->albedo[2] > 0 and depth > 1) {
        auto eta = material->refraction_index;
        if (!inside) {
            eta = 1 / eta;
        }
        auto refract = Refract(ray.GetDirection(), inter.GetNormal(), eta);
        if (refract) {
            double a = -1.0;
            if (inside) {
                a = -a;
            }
            Ray refract_ray{inter.GetPosition() + a * eps * inter.GetNormal(), refract.value()};
            auto refract_inter = FindIntersection(refract_ray, scene);
            if (refract_inter) {
                auto alb = material->albedo[2];
                if (inside) {
                    alb = 1;
                }
                output = output + alb * ComputeColor(refract_inter.value().second, refract_ray,
                                                     refract_inter.value().first, depth - 1, scene,
                                                     !inside);
            }
        }
    }
    return output;
}

inline void ToneMapping(Image& image, const std::vector<std::vector<Vector>>& prepixels) {
    double max = -1;
    for (int i = 0; i < image.Width(); ++i) {
        for (int j = 0; j < image.Height(); ++j) {
            auto& pixel = prepixels[i][j];
            max = std::max(max, pixel[0]);
            max = std::max(max, pixel[1]);
            max = std::max(max, pixel[2]);
        }
    }
    for (int i = 0; i < image.Width(); ++i) {
        for (int j = 0; j < image.Height(); ++j) {
            RGB rgb;
            auto& pixel = prepixels[i][j];
            double r = pixel[0] * (1 + pixel[0] / max / max) / (1 + pixel[0]);
            double g = pixel[1] * (1 + pixel[1] / max / max) / (1 + pixel[1]);
            double b = pixel[2] * (1 + pixel[2] / max / max) / (1 + pixel[2]);
            r = pow(r, 1 / 2.2);
            g = pow(g, 1 / 2.2);
            b = pow(b, 1 / 2.2);
            rgb.r = round(255 * r);
            rgb.g = round(255 * g);
            rgb.b = round(255 * b);
            image.SetPixel(rgb, j, i);
        }
    }
}

Image RenderFull(const std::string& filename, const CameraOptions& camera_options, int depth) {
    const auto scene = ReadScene(filename);
    Image output{camera_options.screen_width, camera_options.screen_height};
    Camera camera{&camera_options};
    std::vector<std::vector<Vector>> prepixels;
    for (int i = 0; i < output.Width(); ++i) {
        prepixels.push_back(std::vector<Vector>(output.Height()));
        for (int j = 0; j < output.Height(); ++j) {
            Ray ray{camera_options.look_from, camera.GetDirection(i, j)};
            auto inter = FindIntersection(ray, scene);
            if (inter) {
                prepixels[i][j] = ComputeColor(inter.value().second, ray, inter.value().first,
                                               depth, scene, false);
            }
        }
    }
    ToneMapping(output, prepixels);
    return output;
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    switch (render_options.mode) {
        case RenderMode::kDepth:
            return RenderDepth(filename, camera_options);
        case RenderMode::kNormal:
            return RenderNormal(filename, camera_options);
        case RenderMode::kFull:
            return RenderFull(filename, camera_options, render_options.depth);
    }
}