#ifndef VOXELIZER_H
#define VOXELIZER_H

#include <string>
#include <vector>
#include <glm.hpp>
#include "chunk.h"

class Voxelizer {
public:
    // Loads an OBJ file and fills the provided Chunk with voxels
    bool LoadAndVoxelize(const std::string& path, Chunk& targetChunk);

private:
    // Simple Triangle-AABB intersection check
    bool TriangleIntersectsVoxel(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, 
                                 glm::vec3 voxelMin, glm::vec3 voxelMax);
    bool planeBoxOverlap(glm::vec3 normal, glm::vec3 vert, glm::vec3 maxbox);
                                
};

#endif