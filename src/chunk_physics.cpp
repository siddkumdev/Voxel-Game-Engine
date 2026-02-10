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

// --- COLLISION LOGIC ---
std::pair<glm::vec3, glm::vec3> Chunk::GetAABB() const {
    // 1. Get the center and half-size (extents) of the object
    // We assume the pivot is in the center for better physics
    glm::vec3 worldSize = GetWorldSize(); 
    glm::vec3 center = physicsBody.position + (worldSize * 0.5f);
    glm::vec3 extent = worldSize * 0.5f;

    // 2. Build a rotation matrix
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(physicsBody.rotation.z), glm::vec3(0, 0, 1));
    glm::mat3 R = glm::mat3(rotMat); // Extract rotation part

    // 3. Calculate new AABB size (Project axes to world)
    // This creates a "Loose AABB" that fits the rotated object perfectly
    glm::vec3 newExtent;
    newExtent.x = std::abs(R[0][0]) * extent.x + std::abs(R[1][0]) * extent.y + std::abs(R[2][0]) * extent.z;
    newExtent.y = std::abs(R[0][1]) * extent.x + std::abs(R[1][1]) * extent.y + std::abs(R[2][1]) * extent.z;
    newExtent.z = std::abs(R[0][2]) * extent.x + std::abs(R[1][2]) * extent.y + std::abs(R[2][2]) * extent.z;

    // 4. Return Min and Max
    return { center - newExtent, center + newExtent };
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

    // If no overlap, return
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

    // --- VELOCITY RESOLUTION ---
    glm::vec3 relativeVelocity = physicsBody.velocity - other.physicsBody.velocity;
    float velAlongNormal = glm::dot(relativeVelocity, normal);

    // Only resolve if moving towards each other
    if (velAlongNormal < 0) {
        float e = (physicsBody.restitution + other.physicsBody.restitution) / 2.0f;

        // FIX 1: RESTING CONTACT THRESHOLD
        // If the relative velocity is very low (e.g. just gravity), turn off the bounce.
        if (std::abs(velAlongNormal) < 2.0f) { 
            e = 0.0f; 
        }

        float j = -(1.0f + e) * velAlongNormal;
        j /= (physicsBody.getInverseMass() + other.physicsBody.getInverseMass());

        glm::vec3 impulse = j * normal;
        physicsBody.velocity += impulse * physicsBody.getInverseMass();
        other.physicsBody.velocity -= impulse * other.physicsBody.getInverseMass();
    // Simple Friction (Apply opposite to tangential velocity)
        glm::vec3 tangent = relativeVelocity - (velAlongNormal * normal);
        if (glm::length(tangent) > 0.001f) {
            tangent = glm::normalize(tangent);
            // Apply friction impulse (simplified)
            float friction = 0.1f; // Adjust this 0.0 to 1.0
            glm::vec3 frictionImpulse = -tangent * j * friction;
            physicsBody.velocity += frictionImpulse * physicsBody.getInverseMass();
            other.physicsBody.velocity -= frictionImpulse * other.physicsBody.getInverseMass();
    }
    }

    // --- FIX 2: POSITIONAL CORRECTION (Anti-Sinking) ---
    // Push them apart based on EXACT penetration depth
    const float percent = 0.8f; // Correct 80% of the error (prevents over-shooting)
    const float slop = 0.01f;   // Ignore tiny overlaps to prevent micro-jitters
    
    float invMassTotal = physicsBody.getInverseMass() + other.physicsBody.getInverseMass();
    if (invMassTotal > 0.0f) {
        glm::vec3 correction = normal * (std::max(penetration - slop, 0.0f) / invMassTotal * percent);
        
        if (!physicsBody.isStatic) physicsBody.position += correction * physicsBody.getInverseMass();
        if (!other.physicsBody.isStatic) other.physicsBody.position -= correction * other.physicsBody.getInverseMass();
    }
}
// --- EXPLOSION & FRACTURE LOGIC ---

std::vector<Chunk*> Chunk::Explode(const PointExplosion& explosion) {
    std::vector<Chunk*> debrisList;
    if (explosion.force <= 0.0f) return debrisList; // Zero force = No damage
    if (physicsBody.layer == PhysicsLayer::DEBRIS) return debrisList; // Debris doesn't explode again    
    // 1. Get Parent Physics Properties
    float parentTotalScale = physicsBody.scale.x; 
    // Calculate the size of ONE voxel in world units
    float voxelWorldSize = parentTotalScale / (float)m_ChunkSize; 

    // Transform Explosion center to Local Index Space
    // (WorldPos - Origin) / SizeOfOneVoxel
    glm::vec3 localCenter = (explosion.center - physicsBody.position) / voxelWorldSize;
    float localRadius = explosion.radius / voxelWorldSize;

    // Optimization: Bounding Sphere check
    glm::vec3 chunkCenter(m_ChunkSize / 2.0f);
    // Rough diagonal check
    if (glm::distance(localCenter, chunkCenter) > (localRadius + m_ChunkSize)) return debrisList;

    int r = (int)std::ceil(localRadius);
    int cx = (int)localCenter.x;
    int cy = (int)localCenter.y;
    int cz = (int)localCenter.z;

    bool meshChanged = false;

    for (int x = cx - r; x <= cx + r; x++) {
        for (int y = cy - r; y <= cy + r; y++) {
            for (int z = cz - r; z <= cz + r; z++) {
                
                if (!IsValidCoordinate(x, y, z)) continue;
                if (!IsActive(x, y, z)) continue;

                float dist = glm::distance(glm::vec3(x,y,z), localCenter);
                if (dist > localRadius) continue;

                // Force Calc
                float damageIntensity = explosion.force;
                if (explosion.falloff) {
                    float t = dist / localRadius; 
                    damageIntensity *= (1.0f - (t * explosion.falloffFactor)); 
                }

                if (damageIntensity > physicsBody.resistance) {
                    SetBlock(x, y, z, false); 
                    meshChanged = true;

                    // Debris Size Logic
                    float destructionRatio = damageIntensity / (physicsBody.resistance + 0.1f);
                    int debrisSize = 1;
                    if (destructionRatio < 2.0f) debrisSize = 4;
                    else if (destructionRatio < 5.0f) debrisSize = 2;

                    // Spawn Debris
                    if (x % debrisSize == 0 && y % debrisSize == 0 && z % debrisSize == 0) {
                        
                        Chunk* debris = new Chunk(debrisSize);
                        debris->GenerateCube(); 
                        
                        debris->name = "Debris";
                        
                        // --- FIX IS HERE ---
                        // Old Code: scale * debrisSize (Resulted in massive objects)
                        // New Code: voxelWorldSize * debrisSize (Results in correct voxel-sized objects)
                        debris->physicsBody.scale = glm::vec3(voxelWorldSize * (float)debrisSize);
                        
                        // Position based on the specific voxel's world location
                        debris->physicsBody.position = physicsBody.position + (glm::vec3(x,y,z) * voxelWorldSize);
                        
                        debris->physicsBody.mass = physicsBody.mass / (float)(m_ChunkSize * 2); // Lighter debris
                        debris->physicsBody.resistance = physicsBody.resistance; 
                        debris->physicsBody.layer = PhysicsLayer::DEBRIS; 
                        debris->physicsBody.isStatic = false;
                        debris->physicsBody.useGravity = true;
                        
                        // Fling
                        glm::vec3 flyDir = glm::normalize(debris->physicsBody.position - explosion.center);
                        if (glm::length(flyDir) == 0) flyDir = glm::vec3(0,1,0);
                        
                        // Add some randomness to flight
                        flyDir.x += ((rand() % 100) / 500.0f); 
                        flyDir.y += ((rand() % 100) / 500.0f); 
                        flyDir.z += ((rand() % 100) / 500.0f); 

                        debris->physicsBody.velocity = flyDir * (damageIntensity * 0.2f); // Reduced multiplier
                        debris->physicsBody.rotation = glm::vec3(rand()%360, rand()%360, rand()%360);

                        debrisList.push_back(debris);
                    }
                }
            }
        }
    }

    if (meshChanged) {
        UpdateMesh();
    }

    return debrisList;
}