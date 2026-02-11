#include "chunk.h"
#include <algorithm>
#include <glad/glad.h>
#include <iostream>

Chunk::Chunk(int resolution) 
    : m_ChunkSize(resolution), m_VertexCount(0)
{
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.resize(volume, 0); // 0 = empty

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
        m_Voxels[x + y * m_ChunkSize + z * m_ChunkSize * m_ChunkSize] = active;
    }
}

bool Chunk::IsValidCoordinate(int x, int y, int z) const {
    return x >= 0 && x < m_ChunkSize &&
           y >= 0 && y < m_ChunkSize &&
           z >= 0 && z < m_ChunkSize;
}

void Chunk::Clear() {
    std::fill(m_Voxels.begin(), m_Voxels.end(), false);
}

void Chunk::SetResolution(int newResolution) {
    if (newResolution < 1 || newResolution > 256) return; // Hard cap to prevent hangs

    m_ChunkSize = newResolution;

    // Resize Memory
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.clear();
    m_Voxels.resize(volume, 0);

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

