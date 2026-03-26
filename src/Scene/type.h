#ifndef TYPES_H
#define TYPES_H

#include <glm.hpp>
#include <string>
#include <utility>

enum class PhysicsLayer {
    DEFAULT = 0,
    DEBRIS = 1,
    PLAYER = 2,
    TERRAIN = 3
};

struct ExplosionData {
    glm::vec3 center;
    float radius;
    float force;
    bool falloff;
    float falloffFactor;
};

enum class ObjectType {
    CHUNK,
    EXPLOSION
};

#endif
