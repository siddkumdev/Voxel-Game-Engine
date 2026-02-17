#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "../Scene/type.h"

class RigidBody : public SceneObject {
public:
    // Physical State
    glm::vec3 velocity     = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);

    // Physical Properties
    float mass             = 1.0f;
    float friction         = 0.5f;
    float restitution      = 0.2f; // Bounciness

    // Destruction Properties
    float resistance       = 10.0f; // How hard it is to break
    PhysicsLayer layer     = PhysicsLayer::DEFAULT;

    // Flags
    bool isStatic          = false;
    bool useGravity        = false;

    float getInverseMass() const { return (isStatic || mass <= 0.0f) ? 0.0f : 1.0f / mass; }

    // Override UpdatePhysics from SceneObject
    void UpdatePhysics(float deltaTime) override;

    // Collision
    bool CheckCollision(const RigidBody& other) const;
    void ResolveCollision(RigidBody& other);
};

#endif
