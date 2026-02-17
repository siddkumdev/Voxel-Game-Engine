#define GLM_ENABLE_EXPERIMENTAL
#include "Chunk.h"
#include <gtx/norm.hpp>
#include <gtc/matrix_transform.hpp>
#include <algorithm>

void Chunk::UpdatePhysics(float deltaTime) {
    if (physicsBody.isStatic) return;

    if (physicsBody.useGravity) {
        physicsBody.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime;
    }

    physicsBody.position += physicsBody.velocity * deltaTime;
    physicsBody.velocity *= (1.0f - 0.5f * deltaTime); // Drag
}

bool Chunk::CheckCollision(const Chunk& other) const {
    if (physicsBody.layer != other.physicsBody.layer) return false;
    auto a = GetAABB();
    auto b = other.GetAABB();

    // Check overlap
    return (a.first.x <= b.second.x && a.second.x >= b.first.x) &&
           (a.first.y <= b.second.y && a.second.y >= b.first.y) &&
           (a.first.z <= b.second.z && a.second.z >= b.first.z);
}

void Chunk::ResolveCollision(Chunk& other) {
    if (physicsBody.isStatic && other.physicsBody.isStatic) return;

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
    if (glm::dot(physicsBody.position - other.physicsBody.position, normal) < 0) normal = -normal;

    // Relative Velocity
    glm::vec3 relVel = physicsBody.velocity - other.physicsBody.velocity;
    float velAlongNormal = glm::dot(relVel, normal);

    if (velAlongNormal < 0) {
        float e = (std::abs(velAlongNormal) < 2.0f) ? 0.0f : 0.5f; // Restitution
        float j = -(1.0f + e) * velAlongNormal / (physicsBody.getInverseMass() + other.physicsBody.getInverseMass());

        glm::vec3 impulse = j * normal;
        physicsBody.velocity += impulse * physicsBody.getInverseMass();
        other.physicsBody.velocity -= impulse * other.physicsBody.getInverseMass();

        // Friction
        glm::vec3 tangent = relVel - (velAlongNormal * normal);
        if (glm::length(tangent) > 0.001f) {
            tangent = glm::normalize(tangent);
            glm::vec3 friction = -tangent * j * 0.1f;
            physicsBody.velocity += friction * physicsBody.getInverseMass();
            other.physicsBody.velocity -= friction * other.physicsBody.getInverseMass();
        }
    }

    // Positional Correction
    float percent = 0.8f, slop = 0.01f;
    float invMassTotal = physicsBody.getInverseMass() + other.physicsBody.getInverseMass();
    if (invMassTotal > 0.0f) {
        glm::vec3 correction = normal * (std::max(penetration - slop, 0.0f) / invMassTotal * percent);
        if (!physicsBody.isStatic) physicsBody.position += correction * physicsBody.getInverseMass();
        if (!other.physicsBody.isStatic) other.physicsBody.position -= correction * other.physicsBody.getInverseMass();
    }
}

std::pair<glm::vec3, glm::vec3> Chunk::GetAABB() const {
    glm::vec3 voxelSize = physicsBody.scale / (float)m_ChunkSize;

    glm::vec3 minV = glm::vec3(boundsMin);
    glm::vec3 maxV = glm::vec3(boundsMax + glm::ivec3(1));

    glm::vec3 localCenter = (minV + maxV) * 0.5f * voxelSize;
    glm::vec3 localExtent = (maxV - minV) * 0.5f * voxelSize;

    // Apply Rotation
    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, glm::radians(physicsBody.rotation.x), {1,0,0});
    rot = glm::rotate(rot, glm::radians(physicsBody.rotation.y), {0,1,0});
    rot = glm::rotate(rot, glm::radians(physicsBody.rotation.z), {0,0,1});
    glm::mat3 R = glm::mat3(rot);

    // OBB Center in World
    // Note: The mesh is centered at physicsBody.position?
    // Wait, original code used: physicsBody.position + (R * localCenter)
    // Is mesh strictly centered? "Default Cube" -> 0..16. Center at 8.
    // If position is at corner, center is offset.
    // Original code seemed to assume position is at (0,0,0) of the local space?
    // No, `model = translate(pos) * rotate * scale`.
    // So local (0,0,0) maps to pos.
    // Chunk grid is (0,0,0) to (Size, Size, Size).
    // So Center is (Size/2) * Scale/Size.
    // So yes, we need to transform localCenter.

    glm::vec3 worldCenter = physicsBody.position + (R * localCenter);
    glm::vec3 newExtent(
        std::abs(R[0][0]) * localExtent.x + std::abs(R[1][0]) * localExtent.y + std::abs(R[2][0]) * localExtent.z,
        std::abs(R[0][1]) * localExtent.x + std::abs(R[1][1]) * localExtent.y + std::abs(R[2][1]) * localExtent.z,
        std::abs(R[0][2]) * localExtent.x + std::abs(R[1][2]) * localExtent.y + std::abs(R[2][2]) * localExtent.z
    );

    return { worldCenter - newExtent, worldCenter + newExtent };
}
