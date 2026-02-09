#include "chunk.h"

void Chunk::UpdatePhysics(float deltaTime) {
    if (physicsBody.isStatic) return;

    // Simple Euler Integration (Standard in basic physics engines)
    if (physicsBody.useGravity) {
        physicsBody.acceleration += glm::vec3(0.0f, -9.81f, 0.0f);
    }

    physicsBody.velocity += physicsBody.acceleration * deltaTime;
    physicsBody.position += physicsBody.velocity * deltaTime;

    // Reset acceleration for next frame
    physicsBody.acceleration = glm::vec3(0.0f);
}
