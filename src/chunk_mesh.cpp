#include "chunk.h"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <glm.hpp>
#include <iostream>

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
