#include "TransformGizmo.h"
#include <algorithm>
#include <cfloat>

bool TransformGizmo::IntersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& boxMin, const glm::vec3& boxMax, float& t) {
    glm::vec3 invDir = 1.0f / rayDir;
    glm::vec3 t0 = (boxMin - rayOrigin) * invDir;
    glm::vec3 t1 = (boxMax - rayOrigin) * invDir;
    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);
    float tmin_max = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float tmax_min = std::min(std::min(tmax.x, tmax.y), tmax.z);
    if (tmax_min >= tmin_max && tmax_min > 0.0f) {
        t = tmin_max > 0.0f ? tmin_max : tmax_min;
        return true;
    }
    return false;
}

bool TransformGizmo::IntersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t) {
    float denom = glm::dot(planeNormal, rayDir);
    if (std::abs(denom) > 1e-6) {
        glm::vec3 p0l0 = planePoint - rayOrigin;
        t = glm::dot(p0l0, planeNormal) / denom;
        return t >= 0.0f;
    }
    return false;
}

bool TransformGizmo::IntersectRayRing(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& center, const glm::vec3& normal, float innerRadius, float outerRadius, float& t) {
    if (IntersectRayPlane(rayOrigin, rayDir, center, normal, t)) {
        glm::vec3 hitPoint = rayOrigin + rayDir * t;
        float dist = glm::length(hitPoint - center);
        return (dist >= innerRadius && dist <= outerRadius);
    }
    return false;
}
