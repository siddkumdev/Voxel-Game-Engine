#include "chunk.h"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath> // Needed for rounding
#include <algorithm>

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

void Chunk::UpdateMesh() {
    m_Vertices.clear();
    m_VertexCount = 0;

    // 1. RESERVE MEMORY (Heuristic: Assume 10% of voxels are surface)
    // This prevents multiple re-allocations
    int approxFaces = (m_ChunkSize * m_ChunkSize * m_ChunkSize) / 4; 
    m_Vertices.reserve(approxFaces * 6 * 6); // 6 vertices per face, 6 floats per vertex

    // 2. RAW ACCESS OPTIMIZATION
    // If you switched to vector<uint8_t>, direct access is faster than function calls.
    // We iterate 0 to Size, but we can optimize checks.
    
    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                
                // Using the public getter is fine, but for raw speed 
                // in C++, accessing the array directly is better if you are inside the class.
                if (!IsActive(x, y, z)) continue;

                float wx = x * m_VoxelSize;
                float wy = y * m_VoxelSize;
                float wz = z * m_VoxelSize;

                // Pass voxel size to addFace to avoid recalculating it inside
                glm::vec3 up(0, m_VoxelSize, 0);
                glm::vec3 right(m_VoxelSize, 0, 0);
                glm::vec3 forward(0, 0, m_VoxelSize); // z is forward/backward

                // Check neighbors (Bounds checks are inside IsActive, which is good for safety)
                if (!IsActive(x, y, z + 1)) 
                    addFace(glm::vec3(wx, wy, wz + m_VoxelSize), up, right, glm::vec3(0, 0, 1));

                if (!IsActive(x, y, z - 1)) 
                    addFace(glm::vec3(wx + m_VoxelSize, wy, wz), up, glm::vec3(-m_VoxelSize, 0, 0), glm::vec3(0, 0, -1));

                if (!IsActive(x + 1, y, z)) 
                    addFace(glm::vec3(wx + m_VoxelSize, wy, wz + m_VoxelSize), up, glm::vec3(0, 0, -m_VoxelSize), glm::vec3(1, 0, 0));

                if (!IsActive(x - 1, y, z)) 
                    addFace(glm::vec3(wx, wy, wz), up, forward, glm::vec3(-1, 0, 0));

                if (!IsActive(x, y + 1, z)) 
                    addFace(glm::vec3(wx, wy + m_VoxelSize, wz + m_VoxelSize), glm::vec3(0, 0, -m_VoxelSize), right, glm::vec3(0, 1, 0));

                if (!IsActive(x, y - 1, z)) 
                    addFace(glm::vec3(wx, wy, wz), forward, right, glm::vec3(0, -1, 0));
            }
        }
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(float), m_Vertices.data(), GL_STATIC_DRAW);

    // Attribute 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Chunk::addFace(const glm::vec3& start, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal) {
    glm::vec3 v_bl = start;
    glm::vec3 v_tl = start + up;
    glm::vec3 v_br = start + right;
    glm::vec3 v_tr = start + up + right;

    // Triangle 1
    m_Vertices.push_back(v_bl.x); m_Vertices.push_back(v_bl.y); m_Vertices.push_back(v_bl.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    m_Vertices.push_back(v_br.x); m_Vertices.push_back(v_br.y); m_Vertices.push_back(v_br.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    m_Vertices.push_back(v_tl.x); m_Vertices.push_back(v_tl.y); m_Vertices.push_back(v_tl.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    // Triangle 2
    m_Vertices.push_back(v_br.x); m_Vertices.push_back(v_br.y); m_Vertices.push_back(v_br.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    m_Vertices.push_back(v_tr.x); m_Vertices.push_back(v_tr.y); m_Vertices.push_back(v_tr.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    m_Vertices.push_back(v_tl.x); m_Vertices.push_back(v_tl.y); m_Vertices.push_back(v_tl.z);
    m_Vertices.push_back(normal.x); m_Vertices.push_back(normal.y); m_Vertices.push_back(normal.z);

    m_VertexCount += 6;
}

void Chunk::UpdatePhysics(float deltaTime) {
    if (physicsBody.isStatic) return;

    // Simple Euler Integration (Standard in basic physics engines)
    if (physicsBody.useGravity) {
        physicsBody.acceleration += glm::vec3(0.0f, -9.81f, 0.0f);
    }

    physicsBody.velocity += physicsBody.acceleration * deltaTime;
    physicsBody.position += physicsBody.velocity * deltaTime;

    // Reset acceleration for next frame
    physicsBody.acceleration = glm::vec3(0.0f);
}

void Chunk::Render(Shader& shader, bool showGrid) {
    if (m_VertexCount == 0) return;

    // Logic: Instead of passing worldPosition as an argument, 
    // we now use the internal RigidBody position.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, physicsBody.position);
    
    // Add rotation support based on physics body
    model = glm::rotate(model, glm::radians(physicsBody.rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.z), glm::vec3(0, 0, 1));

    shader.use();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);

    // Solid Pass
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);

    // Wireframe Pass
    if (showGrid) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        int colorLoc = glGetUniformLocation(shader.ID, "uColor");
        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f); 

        glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}
void Chunk::Clear() {
    // Correctly use the member variable m_Voxels from your image
    std::fill(m_Voxels.begin(), m_Voxels.end(), false);
}


// Add to chunk.cpp
// 1. Update SetVoxelSize to be smart
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

// 2. Update Primitive Generators to save state
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

    // ... (Your existing Sphere Logic) ...
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
    
    // ... (Your existing Cylinder Logic) ...
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

// 3. NEW: Receive Data from Voxelizer
void Chunk::LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices) {
    m_Type = ChunkType::Model;
    m_StoredVertices = vertices;
    m_StoredIndices = indices;
    
    // Reset voxels
    int volume = m_ChunkSize * m_ChunkSize * m_ChunkSize;
    m_Voxels.assign(volume, false);

    VoxelizeStoredMesh();
}

// 4. NEW: The Actual Logic (Moved from Voxelizer.cpp)
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