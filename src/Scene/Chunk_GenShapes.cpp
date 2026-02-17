#include "Chunk.h"
#include <cmath>
#include <iostream>

void Chunk::GenerateTerrain() {
    for (int x = 0; x < m_ChunkSize; x++) {
        for (int z = 0; z < m_ChunkSize; z++) {
            for (int y = 0; y < m_ChunkSize; y++) {
                bool active = (y < m_ChunkSize / 2);
                SetBlock(x, y, z, active);
            }
        }
    }
    UpdateMesh();
}

void Chunk::GenerateCube() {
    m_Type = ChunkType::Cube;
    for (int i=0; i < m_ChunkSize * m_ChunkSize * m_ChunkSize; i++) m_Voxels[i] = 1;
    UpdateMesh();
}

void Chunk::GenerateSphere(int radius) {
    m_Type = ChunkType::Sphere;
    m_Radius = radius;
    Clear();

    int cx = m_ChunkSize / 2, cy = m_ChunkSize / 2, cz = m_ChunkSize / 2;
    int r = (radius == 0) ? (m_ChunkSize / 2) : radius;

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                if (std::sqrt(std::pow(x - cx, 2) + std::pow(y - cy, 2) + std::pow(z - cz, 2)) <= r) {
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

    int cx = m_ChunkSize / 2, cz = m_ChunkSize / 2;
    int r = (radius == 0) ? (m_ChunkSize / 2) : radius;
    int h = (height == 0) ? m_ChunkSize : height;

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int z = 0; z < m_ChunkSize; z++) {
            if (std::sqrt(std::pow(x - cx, 2) + std::pow(z - cz, 2)) <= r) {
                for (int y = 0; y < h && y < m_ChunkSize; y++) SetBlock(x, y, z, true);
            }
        }
    }
    UpdateMesh();
}
