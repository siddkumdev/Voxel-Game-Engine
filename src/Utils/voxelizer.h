#ifndef VOXELIZER_H
#define VOXELIZER_H

#include <string>
#include <vector>
#include <glm.hpp>
#include "Chunk.h"

class Voxelizer {
public:
    // Loads a model file and fills the provided Chunk with voxels
    static bool LoadAndVoxelize(const std::string& path, Chunk& targetChunk);
};

#endif
