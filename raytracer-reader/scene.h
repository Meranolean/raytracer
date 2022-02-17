#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <vector>
#include <map>
#include <string>

#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <filesystem>

class Scene {
public:
    const std::vector<Object>& GetObjects() const {
        return objects_;
    }

    const std::vector<SphereObject>& GetSphereObjects() const {
        return spheres_;
    }

    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    const std::map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

    void AddObject(Object& obj) {
        objects_.push_back(obj);
    }

    void AddSphere(SphereObject sphere) {
        spheres_.push_back(sphere);
    }

    void AddLight(Light light) {
        lights_.push_back(light);
    }

    void SetMaterials(const std::map<std::string, Material>& materials) {
        materials_ = materials;
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> spheres_;
    std::vector<Light> lights_;
    std::map<std::string, Material> materials_;
};

Vector ParseVector(const std::vector<std::string>& splitted_line) {
    double a, b, c;
    a = std::stod(splitted_line[1]);
    b = std::stod(splitted_line[2]);
    c = std::stod(splitted_line[3]);
    return Vector{a, b, c};
}

std::pair<int, int> ParseThreeIndexes(const std::string& string) {
    size_t position = 0;
    std::pair<int, int> output;
    position = string.find("/", position);
    if (position == std::string::npos) {
        output.first = std::stoi(string);
        output.second = 0;
        return output;
    }
    output.first = std::stoi(string.substr(0, position));
    position = string.find("/", position + 1);
    ++position;
    auto buf = string.substr(position, string.size() - position);
    if (!buf.empty()) {
        output.second = std::stoi(buf);
    } else {
        output.second = 0;
    }
    return output;
}

std::vector<std::pair<int, int>> ParseIndexes(const std::vector<std::string>& splitted_line) {
    std::vector<std::pair<int, int>> output;
    for (size_t i = 1; i < splitted_line.size(); ++i) {
        output.push_back(ParseThreeIndexes(splitted_line[i]));
    }
    return output;
}

template <class T>
inline size_t GetIndex(int ind, const std::vector<T>& vector) {
    if (ind < 0) {
        return vector.size() + ind;
    } else {
        return ind - 1;
    }
}

inline std::map<std::string, Material> ReadMaterials(std::string_view filename) {
    std::fstream file;
    file.open(filename.data(), std::ios::in);
    std::string buffer;
    std::map<std::string, Material> output;
    std::string current_material;

    while (getline(file, buffer)) {
        if (buffer.empty() or buffer[0] == '#') {
            continue;
        }

        std::istringstream iss(buffer);
        std::vector<std::string> splitted_line(std::istream_iterator<std::string>{iss},
                                               std::istream_iterator<std::string>());

        if (splitted_line.empty()) {
            continue;
        }

        if (splitted_line[0][0] == '#') {
            continue;
        }
        if (splitted_line[0] == "newmtl") {
            current_material = splitted_line[1];
            output[current_material] = Material(current_material);
        } else if (splitted_line[0] == "Ka") {
            output[current_material].ambient_color = ParseVector(splitted_line);
        } else if (splitted_line[0] == "Kd") {
            output[current_material].diffuse_color = ParseVector(splitted_line);
        } else if (splitted_line[0] == "Ks") {
            output[current_material].specular_color = ParseVector(splitted_line);
        } else if (splitted_line[0] == "Ke") {
            output[current_material].intensity = ParseVector(splitted_line);
        } else if (splitted_line[0] == "Ns") {
            output[current_material].specular_exponent = std::stod(splitted_line[1]);
        } else if (splitted_line[0] == "Ni") {
            output[current_material].refraction_index = std::stod(splitted_line[1]);
        } else if (splitted_line[0] == "al") {
            auto values = ParseVector(splitted_line);
            output[current_material].albedo =
                std::array<double, 3>({{values[0], values[1], values[2]}});
        }
    }
    return output;
}

inline Scene ReadScene(std::string_view filename) {
    std::fstream file;
    file.open(filename.data(), std::ios::in);
    std::string buffer;
    std::map<std::string, Material> materials;
    const Material* cur_material;
    std::vector<Vector> vertexes, normals;
    Scene scene;

    while (getline(file, buffer)) {
        if (buffer.empty() or buffer[0] == '#') {
            continue;
        }

        std::istringstream iss(buffer);
        std::vector<std::string> splitted_line(std::istream_iterator<std::string>{iss},
                                               std::istream_iterator<std::string>());

        if (splitted_line.empty()) {
            continue;
        }

        if (splitted_line[0] == "mtllib") {
            std::filesystem::path path_to_obj(filename);
            auto path_to_mtl = path_to_obj.parent_path() / std::filesystem::path(splitted_line[1]);
            materials = ReadMaterials(path_to_mtl.string());
            scene.SetMaterials(materials);
        } else if (splitted_line[0] == "usemtl") {
            cur_material = &(scene.GetMaterials().at(splitted_line[1]));
        } else if (splitted_line[0] == "S") {
            auto center = ParseVector(splitted_line);
            auto r = std::stod(splitted_line[4]);
            scene.AddSphere(SphereObject(cur_material, center, r));
        } else if (splitted_line[0] == "P") {
            auto position = ParseVector(splitted_line);
            Vector intensity{std::stod(splitted_line[4]), std::stod(splitted_line[5]),
                             std::stod(splitted_line[6])};
            scene.AddLight(Light(position, intensity));
        } else if (splitted_line[0] == "v") {
            vertexes.push_back(ParseVector(splitted_line));
        } else if (splitted_line[0] == "vn") {
            normals.push_back(ParseVector(splitted_line));
        } else if (splitted_line[0] == "f") {
            auto indexes = ParseIndexes(splitted_line);
            Vector v0{vertexes[GetIndex(indexes[0].first, vertexes)]};
            for (size_t i = 1; i < indexes.size() - 1; ++i) {
                Vector v1{vertexes[GetIndex(indexes[i].first, vertexes)]};
                Vector v2{vertexes[GetIndex(indexes[i + 1].first, vertexes)]};
                Object obj{cur_material, Triangle{v0, v1, v2}};
                if (indexes[0].second != 0) {
                    obj.AddNormal(normals[GetIndex(indexes[0].second, normals)]);
                } else {
                    obj.normals.push_back({});
                }
                if (indexes[i].second != 0) {
                    obj.AddNormal(normals[GetIndex(indexes[i].second, normals)]);
                } else {
                    obj.normals.push_back({});
                }
                if (indexes[i + 1].second != 0) {
                    obj.AddNormal(normals[GetIndex(indexes[i + 1].second, normals)]);
                } else {
                    obj.normals.push_back({});
                }
                scene.AddObject(obj);
            }
        }
    }
    return scene;
}
