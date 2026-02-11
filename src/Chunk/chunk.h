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
    Chunk(int resolution = 32);
    ~Chunk();

    // Physics
    RigidBody physicsBody;
    // Call this every frame to update position/velocity based on physics
    void UpdatePhysics(float deltaTime);

    // Generation methods
    void GenerateTerrain();
    void GenerateCube();
    void GenerateSphere(int radius);             // radius in Voxel units
    void GenerateCylinder(int radius, int height); // radius/height in Voxel units
    void LoadMesh(const std::vector<glm::vec3>& vertices, const std::vector<int>& indices);

    // Mesh operations
    void UpdateMesh();
    // Call this to render the chunk. Pass 'true' to show a wireframe grid overlay.
    void Render(Shader& shader, bool showGrid = false);

    // Voxel access
    bool IsActive(int x, int y, int z) const;
    // Sets the voxel at (x, y, z) to active (true) or inactive (false)
    void SetBlock(int x, int y, int z, bool active);
    void Clear();

    // Property accessors
    int GetResolution() const { return m_ChunkSize; }
    
    // Returns the PHYSICAL size in world units
    glm::vec3 GetWorldSize() const { return physicsBody.scale; }
    // Returns the center position in world space
    glm::vec3 GetPosition() const { return physicsBody.position; }

    // Returns the chunk type (Cube, Sphere, etc.)
    ChunkType GetType() const { return m_Type; }
    
    // Changes the resolution of the chunk and rebuilds the mesh
    void SetResolution(int newResolution);

    // Changes the chunk type and rebuilds the mesh
    void SetType(ChunkType newType) { m_Type = newType; }

    // Data members
    glm::vec3 color = glm::vec3(0.2f, 0.8f, 0.2f);
    std::string name = "Object";
    std::pair<glm::vec3, glm::vec3> GetAABB() const;
    
    // Checks collision with another chunk
    bool CheckCollision(const Chunk& other) const;
    
    // Resolves collision with another chunk by applying an impulse to both chunks based on their mass and velocity
    void ResolveCollision(Chunk& other);

    // Explodes the chunk into smaller debris chunks based on the explosion parameters. Returns a list of new debris chunks.
    std::vector<Chunk*> Explode(const PointExplosion& explosion);
    ChunkType m_Type = ChunkType::Cube;
    int m_Radius = 0;
    int m_Height = 0;

    std::string m_ModelPath = "";
    // Call this after modifying generation parameters to regenerate the mesh
    void Rebuild();

    glm::ivec3 boundsMin = glm::ivec3(0); 
    glm::ivec3 boundsMax = glm::ivec3(16); 
    
    // Call this after generating/modifying voxels
    void RecalculateBounds();
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

    // Adds a face to the vertex buffer given the bottom-left corner, up/right vectors, and normal
    void AddFace(const glm::vec3& bottomLeft, const glm::vec3& up, const glm::vec3& right, const glm::vec3& normal);
    // Generates a mesh from the stored vertices/indices (for imported models)
    bool IsValidCoordinate(int x, int y, int z) const;
    // Voxelizes the stored mesh vertices into the voxel grid (for imported models)
    void VoxelizeStoredMesh();
};

#endif