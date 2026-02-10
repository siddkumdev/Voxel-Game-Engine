#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <string>
#include <glm.hpp>
#include "shader.h"
#include "type.h"
#include <cstdint>

enum class ChunkType {
    Cube,
    Sphere,
    Cylinder,
    Model  // Represents an imported OBJ
};

class Chunk {
public:
    // Constructor
    // resolution: The grid density (e.g., 32 = 32x32x32 voxels).
    // Physical size is now controlled exclusively by physicsBody.scale
    Chunk(int resolution = 32);
    ~Chunk();

    // Physics
    RigidBody physicsBody;
    void UpdatePhysics(float deltaTime);

    // Generation methods
    void GenerateTerrain();
    void GenerateCube();
    void GenerateSphere(int radius);             // radius in Voxel units
    void GenerateCylinder(int radius, int height); // radius/height in Voxel units
    void LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);

    // Mesh operations
    void UpdateMesh();
    void Render(Shader& shader, bool showGrid = false);

    // Voxel access
    bool IsActive(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, bool active);
    void Clear();

    // Property accessors
    int GetResolution() const { return m_ChunkSize; }
    
    // Returns the PHYSICAL size in world units
    glm::vec3 GetWorldSize() const { return physicsBody.scale; }
    glm::vec3 GetPosition() const { return physicsBody.position; }
    ChunkType GetType() const { return m_Type; }
    
    // Property mutators
    // Changing resolution creates more/less voxels but keeps the physical size same
    void SetResolution(int newResolution);
    void SetType(ChunkType newType) { m_Type = newType; }

    // Data members
    glm::vec3 color = glm::vec3(0.2f, 0.8f, 0.2f);
    std::string name = "Object";
    std::pair<glm::vec3, glm::vec3> GetAABB() const;
    
    // Checks collision with another chunk
    bool CheckCollision(const Chunk& other) const;
    
    // Resolves collision (Impulse based)
    void ResolveCollision(Chunk& other);

    // Applies explosion damage and returns DEBRIS chunks
    std::vector<Chunk*> Explode(const PointExplosion& explosion);
    ChunkType m_Type = ChunkType::Cube;
    int m_Radius = 0;
    int m_Height = 0;

    std::string m_ModelPath = "";
    void Rebuild();

private:
    // Helper to regenerate the current shape after resolution change

    // Core voxel data
    int m_ChunkSize; // Resolution
    
    // Optimization: uint8_t is faster than vector<bool> for random access
    std::vector<uint8_t> m_Voxels; 
    

    // Rendering data
    unsigned int VAO, VBO;
    std::vector<float> m_Vertices;  
    int m_VertexCount;

    // Generation parameters (Saved to allow Rebuild() to work)
    std::vector<glm::vec3> m_StoredVertices;
    std::vector<int> m_StoredIndices;

    // Helper methods
    void AddFace(const glm::vec3& bottomLeft, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal);
    bool IsValidCoordinate(int x, int y, int z) const;
    void VoxelizeStoredMesh();
};

#endif