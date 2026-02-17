#include "Chunk.h"
#include <cmath>
#include <algorithm>
#include <iostream>

void Chunk::LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices) {
    m_Type = ChunkType::Model;
    m_StoredVertices = vertices;
    m_StoredIndices = indices;
    VoxelizeStoredMesh();
}

void Chunk::VoxelizeStoredMesh() {
    Clear();
    if (m_StoredVertices.empty()) return;

    // 1. Calculate Bounds
    float minX = 1e10, minY = 1e10, minZ = 1e10;
    float maxX = -1e10, maxY = -1e10, maxZ = -1e10;

    for (const auto& v : m_StoredVertices) {
        if (v.x < minX) minX = v.x; if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y; if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z; if (v.z > maxZ) maxZ = v.z;
    }

    float maxDim = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (maxDim <= 0) maxDim = 1.0f;

    // Scale to fit inside the integer grid
    float scale = (m_ChunkSize - 2) / maxDim;

    std::vector<glm::vec3> localVerts(m_StoredVertices.size());
    for (size_t i = 0; i < m_StoredVertices.size(); i++) {
        localVerts[i].x = (m_StoredVertices[i].x - minX) * scale + 1.0f;
        localVerts[i].y = (m_StoredVertices[i].y - minY) * scale + 1.0f;
        localVerts[i].z = (m_StoredVertices[i].z - minZ) * scale + 1.0f;
    }

    // 2. Voxelize (Rasterize Triangles)
    for (size_t i = 0; i < m_StoredIndices.size(); i += 3) {
        glm::vec3 v0 = localVerts[m_StoredIndices[i]];
        glm::vec3 v1 = localVerts[m_StoredIndices[i+1]];
        glm::vec3 v2 = localVerts[m_StoredIndices[i+2]];

        int bMinX = std::max(0, (int)std::floor(std::min({v0.x, v1.x, v2.x})));
        int bMaxX = std::min(m_ChunkSize-1, (int)std::ceil(std::max({v0.x, v1.x, v2.x})));
        int bMinY = std::max(0, (int)std::floor(std::min({v0.y, v1.y, v2.y})));
        int bMaxY = std::min(m_ChunkSize-1, (int)std::ceil(std::max({v0.y, v1.y, v2.y})));
        int bMinZ = std::max(0, (int)std::floor(std::min({v0.z, v1.z, v2.z})));
        int bMaxZ = std::min(m_ChunkSize-1, (int)std::ceil(std::max({v0.z, v1.z, v2.z})));

        for (int x = bMinX; x <= bMaxX; ++x) {
            for (int y = bMinY; y <= bMaxY; ++y) {
                for (int z = bMinZ; z <= bMaxZ; ++z) {
                    SetBlock(x, y, z, true);
                }
            }
        }
    }
    UpdateMesh();
}
