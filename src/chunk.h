#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm.hpp>
#include "shader.h"
#include "type.h"
#include <cstdint> // for uint8_t
// A customizable chunk of voxels
// Add this Enum at the top of chunk.h
enum class ChunkType {
    Cube,
    Sphere,
    Cylinder,
    Model // Represents an imported OBJ
};

class Chunk {
public:
    // chunkSize: Number of blocks along one axis (e.g., 16 or 32)
    // voxelSize: World size of a single block (e.g., 1.0f)
    Chunk(int chunkSize, float voxelSize);
    ~Chunk();

    RigidBody physicsBody;
    void UpdatePhysics(float deltaTime);
    // Fill the chunk with data (e.g., random noise or flat terrain)
    void GenerateTerrain();
    // Inside chunk.h (Public section)
    glm::vec3 color = glm::vec3(0.2f, 0.8f, 0.2f); // Default Green
    float GetVoxelSize() const { return m_VoxelSize; }
    // Rebuilds the mesh (vertices) based on the current voxel data
    // Call this after modifying blocks
    void UpdateMesh();
    
    // Adjusts the voxel size and optionally maintains world size by changing resolution
    void SetVoxelSize(float newVoxelSize, bool maintainWorldSize);

    // Renders the chunk
    void Render(Shader& shader, bool showGrid = false);

    // Helpers
    bool IsActive(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, bool active);
    
    void Clear();
    int GetSize() const { return m_ChunkSize; } // Assuming 'size' is your 32x32x32 dimension
    void GenerateCube(); // The default logic you had
    void GenerateSphere(int radius);
    void GenerateCylinder(int radius, int height);

    // Getters for Selection Logic
    glm::vec3 GetPosition() const { return physicsBody.position; }
    float GetWorldSize() const { return m_ChunkSize * m_VoxelSize; }
    
    // We need to identify objects in the list
    std::string name = "Object";
    void LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);
private:
    int m_ChunkSize;
    float m_VoxelSize;
    
    // 1D array representing 3D data (x + y*size + z*size*size)
    std::vector<uint8_t> m_Voxels; // 8x faster access
    // OpenGL handles
    unsigned int VAO, VBO;
    std::vector<float> m_Vertices; // Stores Pos(3) + Normal(3)
    int m_VertexCount;

    // Helper to add a specific face to the mesh
    void addFace(const glm::vec3& bottomLeft, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal);
    
    // Helper to check bounds
    bool isValidCoordinate(int x, int y, int z) const;
    ChunkType m_Type = ChunkType::Cube;
    
    // Primitive Parameters (To remember settings)
    int m_Radius = 0;
    int m_Height = 0;

    // Mesh Data (To remember imported OBJs)
    std::vector<glm::vec3> m_StoredVertices;
    std::vector<int> m_StoredIndices;

    // Internal helper to re-process the mesh data
    void VoxelizeStoredMesh();
    
    
};

#endif