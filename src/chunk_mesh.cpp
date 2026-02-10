#include "chunk.h"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <glm.hpp>

void Chunk::UpdateMesh() {
    m_Vertices.clear();
    m_VertexCount = 0;

    int approxFaces = (m_ChunkSize * m_ChunkSize * m_ChunkSize) / 8;
    m_Vertices.reserve(approxFaces * 36);

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                
                if (!IsActive(x, y, z)) continue;

                // FIX: Use integer coordinates directly
                float wx = (float)x;
                float wy = (float)y;
                float wz = (float)z;

                glm::vec3 up(0, 1, 0);
                glm::vec3 right(1, 0, 0);
                glm::vec3 forward(0, 0, 1);

                if (!IsActive(x, y, z + 1)) AddFace(glm::vec3(wx, wy, wz + 1), up, right, glm::vec3(0, 0, 1));
                if (!IsActive(x, y, z - 1)) AddFace(glm::vec3(wx + 1, wy, wz), up, glm::vec3(-1, 0, 0), glm::vec3(0, 0, -1));
                if (!IsActive(x + 1, y, z)) AddFace(glm::vec3(wx + 1, wy, wz + 1), up, glm::vec3(0, 0, -1), glm::vec3(1, 0, 0));
                if (!IsActive(x - 1, y, z)) AddFace(glm::vec3(wx, wy, wz), up, forward, glm::vec3(-1, 0, 0));
                if (!IsActive(x, y + 1, z)) AddFace(glm::vec3(wx, wy + 1, wz + 1), glm::vec3(0, 0, -1), right, glm::vec3(0, 1, 0));
                if (!IsActive(x, y - 1, z)) AddFace(glm::vec3(wx, wy, wz), forward, right, glm::vec3(0, -1, 0));
            }
        }
    }
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(float), m_Vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Chunk::AddFace(const glm::vec3& start, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal) {
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

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, physicsBody.position);
    model = glm::rotate(model, glm::radians(physicsBody.rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.z), glm::vec3(0, 0, 1));

    // FIX: Scale the internal integer grid (0..Resolution) to fit the physical Scale
    // Factor = TargetScale / Resolution
    glm::vec3 voxelScale = physicsBody.scale / (float)m_ChunkSize;
    model = glm::scale(model, voxelScale);

    shader.use();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);

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