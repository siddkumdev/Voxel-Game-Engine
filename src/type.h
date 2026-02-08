#ifndef TYPES_H
#define TYPES_H

#include <glm.hpp>

struct RigidBody {
    // Spatial State
    glm::vec3 position     = glm::vec3(0.0f);
    glm::vec3 velocity     = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 rotation     = glm::vec3(0.0f); // Euler angles for simplicity

    // Physical Properties
    float mass             = 1.0f;
    float friction         = 0.5f;
    float restitution      = 0.2f; // Bounciness
    
    // Flags (Inspired by Godot's 'Freeze' or Unreal's 'Simulate Physics')
    bool isStatic          = false; // If true, gravity and forces are ignored
    bool useGravity        = false;
    
    // Derived value: Inverse mass is often more useful for physics calculations
    float getInverseMass() const { return (isStatic || mass <= 0.0f) ? 0.0f : 1.0f / mass; }
};

#endif