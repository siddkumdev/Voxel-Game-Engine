// chunk_physics.cpp
// !!! FIXED: Enable experimental GLM extensions !!!
#define GLM_ENABLE_EXPERIMENTAL 
#include <gtx/norm.hpp> 
#include <gtx/component_wise.hpp>
#include <gtc/matrix_transform.hpp>
#include "chunk.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>

// --- HELPER: AABB Intersection ---
static bool AABBIntersect(const glm::vec3& minA, const glm::vec3& maxA, 
                          const glm::vec3& minB, const glm::vec3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
           (minA.y <= maxB.y && maxA.y >= minB.y) &&
           (minA.z <= maxB.z && maxA.z >= minB.z);
}


void Chunk::UpdatePhysics(float deltaTime) {
    if (physicsBody.isStatic) return;

    // 1. Gravity
    if (physicsBody.useGravity) {
        physicsBody.acceleration += glm::vec3(0.0f, -9.81f, 0.0f);
    }

    // 2. Integration (Euler)
    physicsBody.velocity += physicsBody.acceleration * deltaTime;
    physicsBody.position += physicsBody.velocity * deltaTime;
    
    // 3. Damping (Air Resistance)
    physicsBody.velocity *= (1.0f - 0.5f * deltaTime); 
    physicsBody.acceleration = glm::vec3(0.0f); // Reset accumulator
    

}

bool Chunk::CheckCollision(const Chunk& other) const {
    // 1. Layer Check (Must be on same layer to interact)
    if (physicsBody.layer != other.physicsBody.layer) return false;

    // 2. AABB Check
    auto boxA = GetAABB();
    auto boxB = other.GetAABB();
    return AABBIntersect(boxA.first, boxA.second, boxB.first, boxB.second);
}

void Chunk::ResolveCollision(Chunk& other) {
    if (physicsBody.isStatic && other.physicsBody.isStatic) return;

    // 1. Get AABBs
    auto boxA = GetAABB();
    auto boxB = other.GetAABB();

    // 2. Calculate Overlap on all axes
    float overlapX = std::min(boxA.second.x, boxB.second.x) - std::max(boxA.first.x, boxB.first.x);
    float overlapY = std::min(boxA.second.y, boxB.second.y) - std::max(boxA.first.y, boxB.first.y);
    float overlapZ = std::min(boxA.second.z, boxB.second.z) - std::max(boxA.first.z, boxB.first.z);


    if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) return;

    // 3. Find Minimum Translation Vector (The shortest path out)
    glm::vec3 normal = glm::vec3(0.0f);
    float penetration = 0.0f;

    // Find the smallest overlap axis
    if (overlapX < overlapY && overlapX < overlapZ) {
        normal = glm::vec3(1, 0, 0);
        penetration = overlapX;
    } else if (overlapY < overlapZ) {
        normal = glm::vec3(0, 1, 0);
        penetration = overlapY;
    } else {
        normal = glm::vec3(0, 0, 1);
        penetration = overlapZ;
    }

    // Ensure normal points from Other -> This
    glm::vec3 dir = physicsBody.position - other.physicsBody.position;
    if (glm::dot(dir, normal) < 0) {
        normal = -normal;
    }

    glm::vec3 relativeVelocity = physicsBody.velocity - other.physicsBody.velocity;
    float velAlongNormal = glm::dot(relativeVelocity, normal);

    // Only resolve if moving towards each other
    if (velAlongNormal < 0) {
        float e = (physicsBody.restitution + other.physicsBody.restitution) / 2.0f;

        // If the relative velocity is very low (e.g. just gravity), turn off the bounce.
        if (std::abs(velAlongNormal) < 2.0f) { 
            e = 0.0f; 
        }

        float j = -(1.0f + e) * velAlongNormal;
        j /= (physicsBody.getInverseMass() + other.physicsBody.getInverseMass());

        glm::vec3 impulse = j * normal;
        physicsBody.velocity += impulse * physicsBody.getInverseMass();
        other.physicsBody.velocity -= impulse * other.physicsBody.getInverseMass();
        glm::vec3 tangent = relativeVelocity - (velAlongNormal * normal);

        // Simple Coulomb friction model
        if (glm::length(tangent) > 0.001f) {
            tangent = glm::normalize(tangent);
            float friction = 0.1f; // Adjust this 0.0 to 1.0
            glm::vec3 frictionImpulse = -tangent * j * friction;
            physicsBody.velocity += frictionImpulse * physicsBody.getInverseMass();
            other.physicsBody.velocity -= frictionImpulse * other.physicsBody.getInverseMass();
    }
    }
    
    const float percent = 0.8f; // Correct 80% of the error (prevents over-shooting)
    const float slop = 0.01f;   // Ignore tiny overlaps to prevent micro-jitters
    
    float invMassTotal = physicsBody.getInverseMass() + other.physicsBody.getInverseMass();
    if (invMassTotal > 0.0f) {
        glm::vec3 correction = normal * (std::max(penetration - slop, 0.0f) / invMassTotal * percent);
        
        if (!physicsBody.isStatic) physicsBody.position += correction * physicsBody.getInverseMass();
        if (!other.physicsBody.isStatic) other.physicsBody.position -= correction * other.physicsBody.getInverseMass();
    }
}

std::pair<glm::vec3, glm::vec3> Chunk::GetAABB() const {
    // 1. Calculate the size of a single voxel in World Space
    glm::vec3 worldScale = physicsBody.scale;
    glm::vec3 voxelSize = worldScale / (float)m_ChunkSize;

    // 2. Get the "Tight" Local Bounds (in world units, but unrotated)
    // We add +1 to max because indices are inclusive (block at 5 extends to 6)
    glm::vec3 localMin = glm::vec3(boundsMin) * voxelSize;
    glm::vec3 localMax = glm::vec3(boundsMax + glm::ivec3(1)) * voxelSize;

    // 3. Calculate Center and Extents of this tight box
    glm::vec3 localCenter = (localMin + localMax) * 0.5f;
    glm::vec3 localExtent = (localMax - localMin) * 0.5f;

    // 4. Apply Rotation to the bounds
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.z), glm::vec3(0, 0, 1));
    glm::mat3 R = glm::mat3(rotMat);

    // 5. Transform the Center to World Space
    // The visual mesh starts drawing at physicsBody.position
    glm::vec3 worldCenter = physicsBody.position + (R * localCenter);

    // 6. Project axes to get new AABB size (AABB of OBB logic)
    glm::vec3 newExtent;
    newExtent.x = std::abs(R[0][0]) * localExtent.x + std::abs(R[1][0]) * localExtent.y + std::abs(R[2][0]) * localExtent.z;
    newExtent.y = std::abs(R[0][1]) * localExtent.x + std::abs(R[1][1]) * localExtent.y + std::abs(R[2][1]) * localExtent.z;
    newExtent.z = std::abs(R[0][2]) * localExtent.x + std::abs(R[1][2]) * localExtent.y + std::abs(R[2][2]) * localExtent.z;

    return { worldCenter - newExtent, worldCenter + newExtent };
}