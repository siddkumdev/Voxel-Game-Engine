// type.h
#ifndef TYPES_H
#define TYPES_H

#include <glm.hpp>

// NEW: Physics Layers for interaction filtering
enum class PhysicsLayer {
    DEFAULT = 0,
    DEBRIS = 1,
    PLAYER = 2,
    TERRAIN = 3
};

struct RigidBody {
    // Spatial State
    glm::vec3 position     = glm::vec3(0.0f);
    glm::vec3 velocity     = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 rotation     = glm::vec3(0.0f); 
    glm::vec3 scale        = glm::vec3(1.0f); 

    // Physical Properties
    float mass             = 1.0f;
    float friction         = 0.5f;
    float restitution      = 0.2f; // Bounciness
    
    // NEW: Destruction Properties
    float resistance       = 10.0f; // How hard it is to break
    PhysicsLayer layer     = PhysicsLayer::DEFAULT;

    // Flags 
    bool isStatic          = false; 
    bool useGravity        = false;
    
    float getInverseMass() const { return (isStatic || mass <= 0.0f) ? 0.0f : 1.0f / mass; }
};

// NEW: Explosion Definition
struct PointExplosion {
    glm::vec3 center;
    float radius;
    float force;
    bool falloff;        // Toggle linear falloff
    float falloffFactor; // Strength of falloff (1.0 = standard linear)
};

#endif