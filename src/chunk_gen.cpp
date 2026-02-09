#include "chunk.h"
#include <cmath>
#include <algorithm>
#include <iostream>

void Chunk::GenerateTerrain() {
    // Fill based on current m_ChunkSize
    for (int x = 0; x < m_ChunkSize; x++) {
        for (int z = 0; z < m_ChunkSize; z++) {
            for (int y = 0; y < m_ChunkSize; y++) {

                // Logic: Fill bottom half
                // This scales perfectly.
                // If Res=16, fills up to 8. If Res=32, fills up to 16.
                // Since VoxelSize adjusts inversely, the physical height matches!
                bool active = (y < m_ChunkSize / 2);

                SetBlock(x, y, z, active);
            }
        }
    }

    UpdateMesh();
}

void Chunk::GenerateCube() {
    m_Type = ChunkType::Cube; // Remember this is a cube
    // ... (Keep existing Cube logic, but remove the Clear() if it's redundant)
    for (int i=0; i < m_ChunkSize * m_ChunkSize * m_ChunkSize; i++) m_Voxels[i] = true;
    UpdateMesh();
}

void Chunk::GenerateSphere(int radius) {
    m_Type = ChunkType::Sphere;
    m_Radius = radius; // Save for later

    // Scale radius if resolution changed (optional, keeps relative size)
    // For now, let's keep it simple:
    int actualRadius = (radius * m_ChunkSize) / 32; // Normalize based on default 32?
    // Actually, simpler logic:
    actualRadius = m_ChunkSize / 2;

    Clear();
    int cx = m_ChunkSize / 2;
    int cy = m_ChunkSize / 2;
    int cz = m_ChunkSize / 2;

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                if (std::sqrt(std::pow(x - cx, 2) + std::pow(y - cy, 2) + std::pow(z - cz, 2)) <= actualRadius) {
                    SetBlock(x, y, z, true);
                }
            }
        }
    }
    UpdateMesh();
}

void Chunk::GenerateCylinder(int radius, int height) {
    m_Type = ChunkType::Cylinder;
    m_Radius = radius;
    m_Height = height;

    Clear();
    int cx = m_ChunkSize / 2;
    int cz = m_ChunkSize / 2;

    // Heuristic: Scale height relative to chunk size
    int actualHeight = m_ChunkSize;
    int actualRad = m_ChunkSize / 2;

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int z = 0; z < m_ChunkSize; z++) {
            if (std::sqrt(std::pow(x - cx, 2) + std::pow(z - cz, 2)) <= actualRad) {
                for (int y = 0; y < actualHeight; y++) SetBlock(x, y, z, true);
            }
        }
    }
    UpdateMesh();
}

void Chunk::SetVoxelSize(float newVoxelSize, bool maintainWorldSize) {
    if (newVoxelSize <= 0.001f) return;

    if (maintainWorldSize) {
        float currentWorldSize = m_ChunkSize * m_VoxelSize;
        int newResolution = static_cast<int>((currentWorldSize / newVoxelSize) + 0.5f);
        if (newResolution < 1) newResolution = 1;

        m_ChunkSize = newResolution;
        m_VoxelSize = newVoxelSize;
    } else {
        m_VoxelSize = newVoxelSize;
    }

    // Resize Memory
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.clear();
    m_Voxels.resize(volume, false);

    // --- THE FIX: REBUILD BASED ON TYPE ---
    switch (m_Type) {
        case ChunkType::Cube:
            GenerateCube();
            break;
        case ChunkType::Sphere:
            GenerateSphere(m_Radius); // Use stored radius
            break;
        case ChunkType::Cylinder:
            GenerateCylinder(m_Radius, m_Height); // Use stored params
            break;
        case ChunkType::Model:
            VoxelizeStoredMesh(); // Re-voxelize the saved OBJ data
            break;
    }
    // UpdateMesh is called inside the Generate functions
}

void Chunk::LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices) {
    m_Type = ChunkType::Model;
    m_StoredVertices = vertices;
    m_StoredIndices = indices;

    // Reset voxels
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.assign(volume, false);

    VoxelizeStoredMesh();
}

void Chunk::VoxelizeStoredMesh() {
    if (m_StoredVertices.empty()) return;

    // A. Calculate Bounding Box of the MESH
    float minX = 1e10, minY = 1e10, minZ = 1e10;
    float maxX = -1e10, maxY = -1e10, maxZ = -1e10;

    for (const auto& v : m_StoredVertices) {
        if (v.x < minX) minX = v.x; if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y; if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z; if (v.z > maxZ) maxZ = v.z;
    }

    // B. Calculate Scale to fit inside CURRENT m_ChunkSize
    float maxDim = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (maxDim <= 0) maxDim = 1.0f;

    // Leave a 1-voxel padding
    float scale = (m_ChunkSize - 2) / maxDim;

    // C. Pre-transform Vertices to Voxel Space
    std::vector<glm::vec3> localVerts(m_StoredVertices.size());
    for (size_t i = 0; i < m_StoredVertices.size(); i++) {
        localVerts[i].x = (m_StoredVertices[i].x - minX) * scale + 1.0f;
        localVerts[i].y = (m_StoredVertices[i].y - minY) * scale + 1.0f;
        localVerts[i].z = (m_StoredVertices[i].z - minZ) * scale + 1.0f;
    }

    // D. Fill Voxels
    for (size_t i = 0; i < m_StoredIndices.size(); i += 3) {
        glm::vec3 v0 = localVerts[m_StoredIndices[i]];
        glm::vec3 v1 = localVerts[m_StoredIndices[i+1]];
        glm::vec3 v2 = localVerts[m_StoredIndices[i+2]];

        int bMinX = (int)std::floor(std::min(v0.x, std::min(v1.x, v2.x)));
        int bMaxX = (int)std::ceil(std::max(v0.x, std::max(v1.x, v2.x)));
        int bMinY = (int)std::floor(std::min(v0.y, std::min(v1.y, v2.y)));
        int bMaxY = (int)std::ceil(std::max(v0.y, std::max(v1.y, v2.y)));
        int bMinZ = (int)std::floor(std::min(v0.z, std::min(v1.z, v2.z)));
        int bMaxZ = (int)std::ceil(std::max(v0.z, std::max(v1.z, v2.z)));

        // Clamp to chunk bounds
        bMinX = std::max(0, bMinX); bMaxX = std::min(m_ChunkSize - 1, bMaxX);
        bMinY = std::max(0, bMinY); bMaxY = std::min(m_ChunkSize - 1, bMaxY);
        bMinZ = std::max(0, bMinZ); bMaxZ = std::min(m_ChunkSize - 1, bMaxZ);

        // Simple Bounding Box Fill (Fastest)
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
