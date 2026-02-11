#include "chunk.h"
#include <algorithm>
#include <map>


struct ShardData {
    std::vector<glm::ivec3> voxels; // Local coordinates in the PARENT chunk
    glm::ivec3 minB = glm::ivec3(10000);
    glm::ivec3 maxB = glm::ivec3(-10000);
    glm::vec3 centerOfMassAcc = glm::vec3(0.0f); // To calculate center
};

std::vector<Chunk*> Chunk::Explode(const PointExplosion& explosion) {
    std::vector<Chunk*> debrisList;
    
    if (explosion.force <= 0.0f) return debrisList;
    if (physicsBody.layer == PhysicsLayer::DEBRIS) return debrisList;

    float parentTotalScale = physicsBody.scale.x;
    float voxelWorldSize = parentTotalScale / (float)m_ChunkSize;
    
    // Transform Explosion to Local Index Space
    glm::vec3 localCenter = (explosion.center - physicsBody.position) / voxelWorldSize;
    float localRadius = explosion.radius / voxelWorldSize;

    // 2. Dynamic Destruction Logic
    float intensity = explosion.force / (physicsBody.resistance + 0.01f);
    
    if (intensity < 1.0f) return debrisList;

    // Calculate Number of Shards (Seeds)
    float fractureDensity = std::min(intensity * 2.0f, 50.0f); 
    int numSeeds = (int)fractureDensity; 
    if (numSeeds < 2) numSeeds = 2;

    // 3. Generate Voronoi Seeds (FIXED)
    std::vector<glm::vec3> seeds;
    
    // Calculate the INTERSECTION of the explosion sphere and the chunk box (0..16)
    // This prevents seeds from spawning in empty space if the radius is huge
    int minX = std::max(0, (int)(localCenter.x - localRadius));
    int maxX = std::min(m_ChunkSize, (int)(localCenter.x + localRadius));
    int minY = std::max(0, (int)(localCenter.y - localRadius));
    int maxY = std::min(m_ChunkSize, (int)(localCenter.y + localRadius));
    int minZ = std::max(0, (int)(localCenter.z - localRadius));
    int maxZ = std::min(m_ChunkSize, (int)(localCenter.z + localRadius));

    // Safety check: If explosion doesn't touch the chunk at all
    if (minX >= maxX || minY >= maxY || minZ >= maxZ) return debrisList;

    for (int i = 0; i < numSeeds; i++) {
        glm::vec3 seed;
        int attempts = 0;
        bool valid = false;
        
        do {
            // Generate random point strictly within the intersection box
            // static_cast prevents integer division issues
            float rx = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX + 0.1f)));
            float ry = minY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxY - minY + 0.1f)));
            float rz = minZ + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxZ - minZ + 0.1f)));
            
            seed = glm::vec3(rx, ry, rz);
            
            // Refine: Ensure it's also within the circular blast radius
            if (glm::distance(seed, localCenter) <= localRadius) {
                valid = true;
            }
            attempts++;
        } while (!valid && attempts < 10);

        // Fallback: If rejection sampling fails, clamp the center to the box
        if (!valid) {
            seed.x = std::clamp(localCenter.x, (float)minX, (float)maxX);
            seed.y = std::clamp(localCenter.y, (float)minY, (float)maxY);
            seed.z = std::clamp(localCenter.z, (float)minZ, (float)maxZ);
        }
        
        seeds.push_back(seed);
    }

    // 4. Assign Voxels to Seeds (Voronoi Partitioning)
    std::map<int, ShardData> shards;
    bool meshChanged = false;

    // Optimization: Only loop through the affected area, not the whole chunk
    int cx = (int)localCenter.x; 
    int cy = (int)localCenter.y; 
    int cz = (int)localCenter.z;
    int r = (int)std::ceil(localRadius);

    // Clamp loop bounds to chunk size
    int startX = std::max(0, cx - r); int endX = std::min(m_ChunkSize - 1, cx + r);
    int startY = std::max(0, cy - r); int endY = std::min(m_ChunkSize - 1, cy + r);
    int startZ = std::max(0, cz - r); int endZ = std::min(m_ChunkSize - 1, cz + r);

    for (int x = startX; x <= endX; x++) {
        for (int y = startY; y <= endY; y++) {
            for (int z = startZ; z <= endZ; z++) {
                
                if (!IsActive(x, y, z)) continue;

                // Check Radius
                float distToBlast = glm::distance(glm::vec3(x, y, z), localCenter);
                if (distToBlast > localRadius) continue;

                // Remove from Parent
                SetBlock(x, y, z, false);
                meshChanged = true;

                // Find Closest Seed
                int bestSeed = 0;
                float closestDist = 100000.0f;
                
                for (int i = 0; i < seeds.size(); i++) {
                    float d = glm::distance(glm::vec3(x, y, z), seeds[i]);
                    if (d < closestDist) {
                        closestDist = d;
                        bestSeed = i;
                    }
                }

                // Add to Shard
                ShardData& shard = shards[bestSeed];
                shard.voxels.push_back(glm::ivec3(x, y, z));
                shard.minB = glm::min(shard.minB, glm::ivec3(x, y, z));
                shard.maxB = glm::max(shard.maxB, glm::ivec3(x, y, z));
                shard.centerOfMassAcc += glm::vec3(x, y, z);
            }
        }
    }

    // 5. Create Debris Objects
    for (auto const& [seedIdx, data] : shards) {
        if (data.voxels.empty()) continue;

        // Determine Size
        glm::ivec3 sizeVec = data.maxB - data.minB + glm::ivec3(1);
        int maxDim = std::max({sizeVec.x, sizeVec.y, sizeVec.z});
        
        // Pad slightly to avoid boundary issues
        Chunk* debris = new Chunk(maxDim + 2); 
        debris->name = "Shard";

        // Calculate offset to center the voxels in the new chunk
        glm::ivec3 offset = data.minB;
        
        for (const auto& v : data.voxels) {
            int lx = v.x - offset.x + 1;
            int ly = v.y - offset.y + 1;
            int lz = v.z - offset.z + 1;
            debris->SetBlock(lx, ly, lz, true);
        }

        debris->physicsBody.layer = PhysicsLayer::DEBRIS;
        debris->physicsBody.isStatic = false;
        debris->physicsBody.useGravity = true;
        
        // Scale is fixed to voxel size (ShardResolution * VoxelSize)
        debris->physicsBody.scale = glm::vec3(voxelWorldSize * (float)debris->m_ChunkSize);

        // Position: Place the new chunk so its voxels align with where they used to be
        // NewChunkPos = ParentPos + (Offset - Padding) * VoxelSize
        glm::vec3 worldOffset = glm::vec3(offset.x - 1, offset.y - 1, offset.z - 1) * voxelWorldSize;
        debris->physicsBody.position = physicsBody.position + worldOffset;

        // Mass based on volume
        debris->physicsBody.mass = data.voxels.size() * 0.1f; 
        debris->physicsBody.resistance = physicsBody.resistance;

        // Velocity: Average center of mass of the shard -> outward from explosion
        glm::vec3 centerIdx = data.centerOfMassAcc / (float)data.voxels.size();
        glm::vec3 worldCenter = physicsBody.position + (centerIdx * voxelWorldSize);
        
        glm::vec3 dir = glm::normalize(worldCenter - explosion.center);
        if (glm::length(dir) < 0.001f) dir = glm::vec3(0, 1, 0);

        // Randomize Rotation slightly
        debris->physicsBody.rotation = glm::vec3(rand()%360, rand()%360, rand()%360);
        
        // Velocity = Force * (1/Mass) * RandomFactor
        // Lighter shards fly faster
        float speed = (explosion.force / debris->physicsBody.mass) * 5.0f;
        debris->physicsBody.velocity = dir * std::min(speed, 50.0f); // Cap speed

        // Finalize
        debris->RecalculateBounds(); // Ensure tight hitbox
        debris->UpdateMesh();
        debrisList.push_back(debris);
    }

    if (meshChanged) {
        RecalculateBounds();
        UpdateMesh();
    }

    return debrisList;
}
