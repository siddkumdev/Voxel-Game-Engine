#include "voxelizer.h"
#include "model_loader.h"
#include <iostream>
#include <algorithm>

bool Voxelizer::LoadAndVoxelize(const std::string& path, Chunk& targetChunk) {
    std::vector<glm::vec3> vertices;
    std::vector<int> indices;

    // Check file extension to dispatch loader
    size_t dot = path.find_last_of(".");
    if (dot == std::string::npos) return false;
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // Lowercase extension

    bool loaded = false;

    if (ext == "obj") {
        loaded = ModelLoader::LoadOBJ(path, vertices, indices);
    } else if (ext == "glb") {
        loaded = ModelLoader::LoadGLB(path, vertices, indices);
    } else if (ext == "fbx") {
        loaded = ModelLoader::LoadFBX(path, vertices, indices);
    } else {
        std::cerr << "Format not supported: " << ext << std::endl;
        return false;
    }

    if (!loaded) return false;

    // Delegate voxelization to Chunk which handles it via LoadMesh -> VoxelizeStoredMesh
    targetChunk.LoadMesh(vertices, indices);
    return true;
}
