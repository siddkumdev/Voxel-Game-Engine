#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <string>
#include <glm.hpp>
#include <cstdint>
#include "shader.h"
#include "type.h"
#include "SceneObject.h"
#include "../Physics/RigidBody.h"

enum class ChunkType { Cube, Sphere, Cylinder, Model };

class Chunk : public RigidBody {
public:
    Chunk(int resolution = 32);
    ~Chunk();

    glm::vec3 GetWorldSize() const { return scale; }
    glm::vec3 GetPosition() const { return position; }

    std::pair<glm::vec3, glm::vec3> GetAABB() const override;

    void GenerateTerrain();
    void GenerateCube();
    void GenerateSphere(int radius);
    void GenerateCylinder(int radius, int height);
    void LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);
    void Rebuild();

    bool IsActive(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, bool active);
    void Clear();
    void SetResolution(int newResolution);
    int GetResolution() const { return m_ChunkSize; }

    void UpdateMesh();
    void Render(Shader& shader) override;

    ChunkType m_Type = ChunkType::Cube;
    std::string m_ModelPath = "";

    int m_Radius = 0;
    int m_Height = 0;
    std::vector<glm::vec3> m_StoredVertices;
    std::vector<int> m_StoredIndices;

    glm::ivec3 boundsMin = glm::ivec3(0);
    glm::ivec3 boundsMax = glm::ivec3(16);
    void RecalculateBounds();

    bool showGrid = false;

private:
    int m_ChunkSize;
    std::vector<uint8_t> m_Voxels;

    unsigned int VAO, VBO;
    std::vector<float> m_Vertices;
    int m_VertexCount;

    void AddFace(const glm::vec3& p, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal);
    bool IsValidCoordinate(int x, int y, int z) const;
    void VoxelizeStoredMesh();
};

#endif
