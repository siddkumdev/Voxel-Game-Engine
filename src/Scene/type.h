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

class Shader; // Forward declaration

struct SceneObject {
    ObjectType type;
    std::string name;

    // Transform
    glm::vec3 position     = glm::vec3(0.0f);
    glm::vec3 rotation     = glm::vec3(0.0f);
    glm::vec3 scale        = glm::vec3(1.0f);
    glm::vec3 color        = glm::vec3(1.0f);

    virtual ~SceneObject() = default;

    // Added Virtual Methods so Application can call them on any object
    virtual void UpdatePhysics(float dt) {} 
    virtual void Render(Shader& shader) {} 
    
    // Default AABB based on position and scale
    virtual std::pair<glm::vec3, glm::vec3> GetAABB() const {
        return { position, position + scale };
    }
};

#endif
