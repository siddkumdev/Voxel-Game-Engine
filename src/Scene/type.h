// type.h
#ifndef TYPES_H
#define TYPES_H

#include <glm.hpp>
#include <string>
#include <utility> // For std::pair

// Physics Layers for interaction filtering
enum class PhysicsLayer {
    DEFAULT = 0,
    DEBRIS = 1,
    PLAYER = 2,
    TERRAIN = 3
};

// NEW: Explosion Definition (renamed from PointExplosion struct)
struct ExplosionData {
    glm::vec3 center;
    float radius;
    float force;
    bool falloff;        // Toggle linear falloff
    float falloffFactor; // Strength of falloff (1.0 = standard linear)
};

enum class ObjectType {
    CHUNK,
    EXPLOSION
};

#endif
