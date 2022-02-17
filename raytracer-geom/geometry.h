#include <vector.h>
#include <sphere.h>
#include <intersection.h>
#include <triangle.h>

#include <optional>

#include <ray.h>

const double kEpsilon = 1e-12;

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {
    auto ray_to_center = sphere.GetCenter() - ray.GetOrigin();
    float tca = DotProduct(ray_to_center, ray.GetDirection());
    if (tca < 0.0) {
        return {};
    }
    float d2 = DotProduct(ray_to_center, ray_to_center) - tca * tca;
    float r2 = sphere.GetRadius() * sphere.GetRadius();
    if (d2 > r2) {
        return {};
    }
    float thc = sqrt(r2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 > t1) {
        std::swap(t0, t1);
    }

    if (t0 < 0) {
        t0 = t1;
        if (t0 < 0) {
            return {};
        }
    }
    auto pos = ray.GetOrigin() + ray.GetDirection() * t0;
    auto norm = pos - sphere.GetCenter();
    norm.Normalize();
    if (DotProduct(norm, ray.GetDirection()) > 0) {
        norm = norm * -1.0;
    }
    return Intersection(pos, norm, t0);
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {
    double a, f, u, v;
    Vector h, s, q;
    Vector ab = triangle.GetVertex(1) - triangle.GetVertex(0);
    Vector ac = triangle.GetVertex(2) - triangle.GetVertex(0);
    h = CrossProduct(ray.GetDirection(), ac);
    a = DotProduct(ab, h);
    if (a > -kEpsilon and a < kEpsilon) {
        return {};
    }

    f = 1.0 / a;
    s = ray.GetOrigin() - triangle.GetVertex(0);
    u = f * DotProduct(s, h);
    if (u < 0.0 or u > 1.0) {
        return {};
    }

    q = CrossProduct(s, ab);
    v = f * DotProduct(ray.GetDirection(), q);
    if (v < 0.0 or u + v > 1.0) {
        return {};
    }

    double t = f * DotProduct(ac, q);
    if (t > kEpsilon) {
        auto pos = ray.GetOrigin() + ray.GetDirection() * t;
        auto norm = CrossProduct(ab, ac);
        norm.Normalize();
        auto costh = DotProduct(ray.GetDirection(), norm);
        if (costh > 0) {
            norm = -1 * norm;
        }
        auto dist = Length(pos - ray.GetOrigin());
        return Intersection(pos, norm, dist);
    } else {
        return {};
    }
}

std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    auto cos1 = DotProduct(ray, normal);
    if (cos1 < 0) {
        cos1 = -cos1;
    }
    auto sin2 = eta * sqrt(1 - cos1 * cos1);
    if (sin2 > 1 or sin2 < -1) {
        return {};
    }
    auto cos2 = sqrt(1 - sin2 * sin2);
    auto output = eta * ray + (eta * cos1 - cos2) * normal;
    output.Normalize();
    return output;
}

Vector Reflect(const Vector& ray, const Vector& normal) {
    auto cos1 = DotProduct(ray, normal);
    if (cos1 < 0) {
        cos1 = -cos1;
    }
    auto output = ray + 2 * cos1 * normal;
    output.Normalize();
    return output;
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    double u, v, w;
    Vector ab = triangle.GetVertex(1) - triangle.GetVertex(0);
    Vector ac = triangle.GetVertex(2) - triangle.GetVertex(0);
    Vector ap = point - triangle.GetVertex(0);
    Vector cp = point - triangle.GetVertex(2);
    Vector bp = point - triangle.GetVertex(1);
    auto full_area = Length(CrossProduct(ab, ac));
    u = Length(CrossProduct(cp, bp)) / full_area;
    v = Length(CrossProduct(ap, cp)) / full_area;
    w = Length(CrossProduct(ap, bp)) / full_area;
    return Vector({u, v, w});
}
