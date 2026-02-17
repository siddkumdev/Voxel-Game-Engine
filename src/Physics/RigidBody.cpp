#define GLM_ENABLE_EXPERIMENTAL
#include "RigidBody.h"
#include <gtx/norm.hpp>
#include <algorithm>
#include <cmath>

void RigidBody::UpdatePhysics(float deltaTime) {
    if (isStatic) return;

    if (useGravity) {
        velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime;
    }

    position += velocity * deltaTime;
    velocity *= (1.0f - 0.5f * deltaTime); // Drag
}

bool RigidBody::CheckCollision(const RigidBody& other) const {
    if (layer != other.layer) return false;
    auto a = GetAABB();
    auto b = other.GetAABB();

    // Check overlap
    return (a.first.x <= b.second.x && a.second.x >= b.first.x) &&
           (a.first.y <= b.second.y && a.second.y >= b.first.y) &&
           (a.first.z <= b.second.z && a.second.z >= b.first.z);
}

void RigidBody::ResolveCollision(RigidBody& other) {
    if (isStatic && other.isStatic) return;

    auto a = GetAABB();
    auto b = other.GetAABB();

    // Calculate overlap
    glm::vec3 overlap;
    overlap.x = std::min(a.second.x, b.second.x) - std::max(a.first.x, b.first.x);
    overlap.y = std::min(a.second.y, b.second.y) - std::max(a.first.y, b.first.y);
    overlap.z = std::min(a.second.z, b.second.z) - std::max(a.first.z, b.first.z);

    if (overlap.x <= 0 || overlap.y <= 0 || overlap.z <= 0) return;

    // Find min penetration axis
    glm::vec3 normal(0.0f);
    float penetration = 0.0f;

    if (overlap.x < overlap.y && overlap.x < overlap.z) { normal = {1,0,0}; penetration = overlap.x; }
    else if (overlap.y < overlap.z) { normal = {0,1,0}; penetration = overlap.y; }
    else { normal = {0,0,1}; penetration = overlap.z; }

    // Ensure normal points from Other -> This
    if (glm::dot(position - other.position, normal) < 0) normal = -normal;

    // Relative Velocity
    glm::vec3 relVel = velocity - other.velocity;
    float velAlongNormal = glm::dot(relVel, normal);

    if (velAlongNormal < 0) {
        float e = (std::abs(velAlongNormal) < 2.0f) ? 0.0f : 0.5f; // Restitution
        float j = -(1.0f + e) * velAlongNormal / (getInverseMass() + other.getInverseMass());

        glm::vec3 impulse = j * normal;
        velocity += impulse * getInverseMass();
        other.velocity -= impulse * other.getInverseMass();

        // Friction
        glm::vec3 tangent = relVel - (velAlongNormal * normal);
        if (glm::length(tangent) > 0.001f) {
            tangent = glm::normalize(tangent);
            glm::vec3 frictionForce = -tangent * j * 0.1f;
            velocity += frictionForce * getInverseMass();
            other.velocity -= frictionForce * other.getInverseMass();
        }
    }

    // Positional Correction
    float percent = 0.8f, slop = 0.01f;
    float invMassTotal = getInverseMass() + other.getInverseMass();
    if (invMassTotal > 0.0f) {
        glm::vec3 correction = normal * (std::max(penetration - slop, 0.0f) / invMassTotal * percent);
        if (!isStatic) position += correction * getInverseMass();
        if (!other.isStatic) other.position -= correction * other.getInverseMass();
    }
}
