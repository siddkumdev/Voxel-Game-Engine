#include "Chunk.h"
#include <algorithm>
#include <glad/glad.h>
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include <cmath>

Chunk::Chunk(int resolution) 
    : m_ChunkSize(resolution), m_VertexCount(0)
{

    type = ObjectType::CHUNK;
    color = glm::vec3(0.2f, 0.8f, 0.2f);
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.resize(volume, 0);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool Chunk::IsActive(int x, int y, int z) const {
    if (!IsValidCoordinate(x, y, z)) return false;
    return m_Voxels[x + y * m_ChunkSize + z * m_ChunkSize * m_ChunkSize];
}

void Chunk::SetBlock(int x, int y, int z, bool active) {
    if (IsValidCoordinate(x, y, z)) {
        m_Voxels[x + y * m_ChunkSize + z * m_ChunkSize * m_ChunkSize] = active ? 1 : 0;
    }
}

bool Chunk::IsValidCoordinate(int x, int y, int z) const {
    return x >= 0 && x < m_ChunkSize &&
           y >= 0 && y < m_ChunkSize &&
           z >= 0 && z < m_ChunkSize;
}

void Chunk::Clear() {
    std::fill(m_Voxels.begin(), m_Voxels.end(), 0);
}

void Chunk::SetResolution(int newResolution) {
    if (newResolution < 2) newResolution = 2;
    if (newResolution > 256) newResolution = 256;

    m_ChunkSize = newResolution;
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.resize(volume);
    Clear();

    Rebuild();
}

void Chunk::Rebuild() {
    switch (m_Type) {
        case ChunkType::Cube:     GenerateCube(); break;
        case ChunkType::Sphere:   GenerateSphere(m_Radius); break;
        case ChunkType::Cylinder: GenerateCylinder(m_Radius, m_Height); break;
        case ChunkType::Model:    VoxelizeStoredMesh(); break;
    }
}

void Chunk::RecalculateBounds() {
    boundsMin = glm::ivec3(m_ChunkSize);
    boundsMax = glm::ivec3(0);
    bool empty = true;

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                if (IsActive(x, y, z)) {
                    empty = false;
                    boundsMin.x = std::min(boundsMin.x, x);
                    boundsMin.y = std::min(boundsMin.y, y);
                    boundsMin.z = std::min(boundsMin.z, z);
                    boundsMax.x = std::max(boundsMax.x, x);
                    boundsMax.y = std::max(boundsMax.y, y);
                    boundsMax.z = std::max(boundsMax.z, z);
                }
            }
        }
    }

    if (empty) {
        boundsMin = glm::ivec3(0);
        boundsMax = glm::ivec3(0);
    }
}

std::pair<glm::vec3, glm::vec3> Chunk::GetAABB() const {
    glm::vec3 voxelSize = scale / (float)m_ChunkSize;

    glm::vec3 minV = glm::vec3(boundsMin);
    glm::vec3 maxV = glm::vec3(boundsMax + glm::ivec3(1));

    glm::vec3 localCenter = (minV + maxV) * 0.5f * voxelSize;
    glm::vec3 localExtent = (maxV - minV) * 0.5f * voxelSize;

    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, glm::radians(rotation.x), {1,0,0});
    rot = glm::rotate(rot, glm::radians(rotation.y), {0,1,0});
    rot = glm::rotate(rot, glm::radians(rotation.z), {0,0,1});
    glm::mat3 R = glm::mat3(rot);

    glm::vec3 worldCenter = position + (R * localCenter);
    glm::vec3 newExtent(
        std::abs(R[0][0]) * localExtent.x + std::abs(R[1][0]) * localExtent.y + std::abs(R[2][0]) * localExtent.z,
        std::abs(R[0][1]) * localExtent.x + std::abs(R[1][1]) * localExtent.y + std::abs(R[2][1]) * localExtent.z,
        std::abs(R[0][2]) * localExtent.x + std::abs(R[1][2]) * localExtent.y + std::abs(R[2][2]) * localExtent.z
    );

    return { worldCenter - newExtent, worldCenter + newExtent };
}
