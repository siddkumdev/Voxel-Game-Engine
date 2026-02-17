#include "Chunk.h"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <glm.hpp>

void Chunk::UpdateMesh() {
    m_Vertices.clear();
    m_VertexCount = 0;

    // Reserve memory (approximation: volume / 2 * 6 faces * 6 verts * 6 floats)
    // A better approx is volume/8.
    int approxFaces = (m_ChunkSize * m_ChunkSize * m_ChunkSize) / 8;
    m_Vertices.reserve(approxFaces * 36);

    for (int x = 0; x < m_ChunkSize; x++) {
        for (int y = 0; y < m_ChunkSize; y++) {
            for (int z = 0; z < m_ChunkSize; z++) {
                if (!IsActive(x, y, z)) continue;

                float wx = (float)x, wy = (float)y, wz = (float)z;

                // Directions
                glm::vec3 up(0, 1, 0), right(1, 0, 0), fwd(0, 0, 1);

                // Check neighbors and add faces
                if (!IsActive(x, y, z + 1)) AddFace(glm::vec3(wx, wy, wz + 1), up, right, fwd);       // Front
                if (!IsActive(x, y, z - 1)) AddFace(glm::vec3(wx + 1, wy, wz), up, -right, -fwd);     // Back
                if (!IsActive(x + 1, y, z)) AddFace(glm::vec3(wx + 1, wy, wz + 1), up, -fwd, right);  // Right
                if (!IsActive(x - 1, y, z)) AddFace(glm::vec3(wx, wy, wz), up, fwd, -right);          // Left
                if (!IsActive(x, y + 1, z)) AddFace(glm::vec3(wx, wy + 1, wz + 1), -fwd, right, up);  // Top
                if (!IsActive(x, y - 1, z)) AddFace(glm::vec3(wx, wy, wz), fwd, right, -up);          // Bottom
            }
        }
    }

    if (VAO == 0) glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    if (VBO == 0) glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(float), m_Vertices.data(), GL_STATIC_DRAW);

    // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    RecalculateBounds();
}

void Chunk::AddFace(const glm::vec3& p, const glm::vec3& u, const glm::vec3& r, const glm::vec3& n) {
    // Quad vertices: p, p+u, p+r, p+u+r
    glm::vec3 p1 = p;          // Bottom-Left
    glm::vec3 p2 = p + r;      // Bottom-Right
    glm::vec3 p3 = p + u;      // Top-Left
    glm::vec3 p4 = p + u + r;  // Top-Right

    auto add = [&](const glm::vec3& v) {
        m_Vertices.push_back(v.x); m_Vertices.push_back(v.y); m_Vertices.push_back(v.z);
        m_Vertices.push_back(n.x); m_Vertices.push_back(n.y); m_Vertices.push_back(n.z);
    };

    // Tri 1
    add(p1); add(p2); add(p3);
    // Tri 2
    add(p2); add(p4); add(p3);
    m_VertexCount += 6;
}

void Chunk::Render(Shader& shader, bool showGrid) {
    if (m_VertexCount == 0 && m_Vertices.empty()) return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, physicsBody.position);
    model = glm::rotate(model, glm::radians(physicsBody.rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(physicsBody.rotation.z), glm::vec3(0, 0, 1));

    // Scale local grid (0..Size) to world (Scale)
    glm::vec3 voxelScale = physicsBody.scale / (float)m_ChunkSize;
    model = glm::scale(model, voxelScale);

    shader.use();
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, m_VertexCount > 0 ? m_VertexCount : m_Vertices.size() / 6);

    if (showGrid) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        int colorLoc = glGetUniformLocation(shader.ID, "uColor");
        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f); // Black wireframe

        glDrawArrays(GL_TRIANGLES, 0, m_VertexCount > 0 ? m_VertexCount : m_Vertices.size() / 6);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}
