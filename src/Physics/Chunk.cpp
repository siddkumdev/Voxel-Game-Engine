#include "Chunk.h"
#include <algorithm>
#include <glad/glad.h>
#include <iostream>

Chunk::Chunk(int resolution) 
    : m_ChunkSize(resolution), m_VertexCount(0)
{
    // Resize voxel storage
    type = ObjectType::CHUNK; // Ensure type is set for SceneObject base
    color = glm::vec3(0.2f, 0.8f, 0.2f); // Default color for chunks
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.resize(volume, 0);

    // Init OpenGL buffers
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

void Chunk::Render(Shader& shader) {
    // Call your specific render with grid enabled/disabled as you prefer
    Render(shader, false); 
}