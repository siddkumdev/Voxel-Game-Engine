#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <string>
#include <glm.hpp>
#include <cstdint>
#include "shader.h"
#include "type.h"

enum class ChunkType { Cube, Sphere, Cylinder, Model };

class Chunk : public SceneObject {
public:
    Chunk(int resolution = 32);
    ~Chunk();

    // -- Physics & Transform --
    RigidBody physicsBody;
    void UpdatePhysics(float deltaTime);

    bool CheckCollision(const Chunk& other) const;
    void ResolveCollision(Chunk& other);

    // Returns debris chunks
    std::vector<Chunk*> Explode(const PointExplosion& explosion);

    glm::vec3 GetWorldSize() const { return physicsBody.scale; }
    glm::vec3 GetPosition() const { return physicsBody.position; }
    std::pair<glm::vec3, glm::vec3> GetAABB() const;

    // -- Generation --
    void GenerateTerrain();
    void GenerateCube();
    void GenerateSphere(int radius);
    void GenerateCylinder(int radius, int height);
    void LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);
    void Rebuild();

    // -- Voxel Manipulation --
    bool IsActive(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, bool active);
    void Clear();
    void SetResolution(int newResolution);
    int GetResolution() const { return m_ChunkSize; }

    // -- Rendering --
    void UpdateMesh();
    void Render(Shader& shader, bool showGrid = false);
    void Render(Shader& shader) override;
    // -- Properties --
    std::string name = "Object";
    ChunkType m_Type = ChunkType::Cube;
    std::string m_ModelPath = "";

    // Gen Params (Public for serialization)
    int m_Radius = 0;
    int m_Height = 0;
    std::vector<glm::vec3> m_StoredVertices;
    std::vector<int> m_StoredIndices;

    // Bounds
    glm::ivec3 boundsMin = glm::ivec3(0);
    glm::ivec3 boundsMax = glm::ivec3(16);
    void RecalculateBounds();

private:
    int m_ChunkSize;
    std::vector<uint8_t> m_Voxels;

    // Rendering
    unsigned int VAO, VBO;
    std::vector<float> m_Vertices;
    int m_VertexCount;

    // Helpers
    void AddFace(const glm::vec3& p, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal);
    bool IsValidCoordinate(int x, int y, int z) const;
    void VoxelizeStoredMesh();
};

#endif
