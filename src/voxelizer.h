#ifndef VOXELIZER_H
#define VOXELIZER_H

#include <string>
#include <vector>
#include <glm.hpp>
#include "chunk.h"

class Voxelizer {
public:
    // Loads a model file and fills the provided Chunk with voxels
    bool LoadAndVoxelize(const std::string& path, Chunk& targetChunk);
};

#endif
