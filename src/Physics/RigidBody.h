#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "../Scene/type.h"
#include "../Scene/SceneObject.h"

class RigidBody : public SceneObject {
public:

    glm::vec3 velocity     = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);

    float mass             = 1.0f;
    float friction         = 0.5f;
    float restitution      = 0.2f;

    float resistance       = 10.0f;
    PhysicsLayer layer     = PhysicsLayer::DEFAULT;

    bool isStatic          = false;
    bool useGravity        = false;

    float getInverseMass() const { return (isStatic || mass <= 0.0f) ? 0.0f : 1.0f / mass; }

    void UpdatePhysics(float deltaTime) override;

    bool CheckCollision(const RigidBody& other) const;
    void ResolveCollision(RigidBody& other);
};

#endif
