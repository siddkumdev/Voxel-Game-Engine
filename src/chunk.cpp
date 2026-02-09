#include "chunk.h"
#include <algorithm>
#include <glad/glad.h>

Chunk::Chunk(int chunkSize, float voxelSize) 
    : m_ChunkSize(chunkSize), m_VoxelSize(voxelSize), m_VertexCount(0)
{
    // Reserve memory immediately
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.resize(volume, false);

    // Init OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

Chunk::~Chunk() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool Chunk::IsActive(int x, int y, int z) const {
    if (!isValidCoordinate(x, y, z)) return false;
    return m_Voxels[x + y * m_ChunkSize + z * m_ChunkSize * m_ChunkSize];
}

void Chunk::SetBlock(int x, int y, int z, bool active) {
    if (isValidCoordinate(x, y, z)) {
        m_Voxels[x + y * m_ChunkSize + z * m_ChunkSize * m_ChunkSize] = active;
    }
}

bool Chunk::isValidCoordinate(int x, int y, int z) const {
    return x >= 0 && x < m_ChunkSize &&
           y >= 0 && y < m_ChunkSize &&
           z >= 0 && z < m_ChunkSize;
}

void Chunk::Clear() {
    std::fill(m_Voxels.begin(), m_Voxels.end(), false);
}
