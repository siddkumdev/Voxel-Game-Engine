#include "voxelizer.h"
#include <iostream>
#include <algorithm>
#include <chrono> 


bool Voxelizer::LoadAndVoxelize(const std::string& path, Chunk& targetChunk) {
    // --- FAST CUSTOM OBJ PARSER ---
    std::vector<glm::vec3> rawVertices;
    std::vector<int> indices;

    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    char line[128];
    while (fgets(line, 128, file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 v;
            // Parse "v 1.0 2.0 3.0"
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            rawVertices.push_back(v);
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            int v1, v2, v3;
            // Matches "f 1 2 3" OR "f 1/1/1 2/2/2 3/3/3"
            // We only care about the first number (vertex index)
            if (sscanf(line, "f %d/%*s %d/%*s %d/%*s", &v1, &v2, &v3) == 3 ||
                sscanf(line, "f %d %d %d", &v1, &v2, &v3) == 3 || 
                sscanf(line, "f %d//%*d %d//%*d %d//%*d", &v1, &v2, &v3) == 3) {
                
                // OBJ indices are 1-based; convert to 0-based
                indices.push_back(v1 - 1);
                indices.push_back(v2 - 1);
                indices.push_back(v3 - 1);
            }
        }
    }
    fclose(file);
    // -----------------------------

    targetChunk.Clear();
    int gridSize = targetChunk.GetSize();

    // 1. Find Bounding Box
    float minX = 1e10, minY = 1e10, minZ = 1e10;
    float maxX = -1e10, maxY = -1e10, maxZ = -1e10;

    for (const auto& v : rawVertices) {
        if (v.x < minX) minX = v.x; if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y; if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z; if (v.z > maxZ) maxZ = v.z;
    }

    float maxDim = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (maxDim <= 0) maxDim = 1.0f;
    float scale = (gridSize - 2) / maxDim;

    // 2. Pre-transform Vertices
    std::vector<glm::vec3> voxelVerts(rawVertices.size());
    for (size_t i = 0; i < rawVertices.size(); i++) {
        voxelVerts[i].x = (rawVertices[i].x - minX) * scale + 1.0f;
        voxelVerts[i].y = (rawVertices[i].y - minY) * scale + 1.0f;
        voxelVerts[i].z = (rawVertices[i].z - minZ) * scale + 1.0f;
    }

    // 3. Voxelize
    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = voxelVerts[indices[i]];
        glm::vec3 v1 = voxelVerts[indices[i+1]];
        glm::vec3 v2 = voxelVerts[indices[i+2]];

        int bMinX = (int)std::floor(std::min({v0.x, v1.x, v2.x}));
        int bMaxX = (int)std::ceil(std::max({v0.x, v1.x, v2.x}));
        int bMinY = (int)std::floor(std::min({v0.y, v1.y, v2.y}));
        int bMaxY = (int)std::ceil(std::max({v0.y, v1.y, v2.y}));
        int bMinZ = (int)std::floor(std::min({v0.z, v1.z, v2.z}));
        int bMaxZ = (int)std::ceil(std::max({v0.z, v1.z, v2.z}));

        bMinX = std::max(0, bMinX); bMaxX = std::min(gridSize - 1, bMaxX);
        bMinY = std::max(0, bMinY); bMaxY = std::min(gridSize - 1, bMaxY);
        bMinZ = std::max(0, bMinZ); bMaxZ = std::min(gridSize - 1, bMaxZ);

        for (int x = bMinX; x <= bMaxX; ++x) {
            for (int y = bMinY; y <= bMaxY; ++y) {
                for (int z = bMinZ; z <= bMaxZ; ++z) {
                    // Check intersection here if you added that function, 
                    // otherwise this is the fast-fill:
                    targetChunk.SetBlock(x, y, z, true);
                }
            }
        }
    }
    return true;
}

bool Voxelizer::TriangleIntersectsVoxel(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 vMin, glm::vec3 vMax) {
    float triMinX = std::min({v0.x, v1.x, v2.x});
    float triMaxX = std::max({v0.x, v1.x, v2.x});
    if (triMaxX < vMin.x || triMinX > vMax.x) return false;
    float triMinY = std::min({v0.y, v1.y, v2.y});
    float triMaxY = std::max({v0.y, v1.y, v2.y});
    if (triMaxY < vMin.y || triMinY > vMax.y) return false;
    float triMinZ = std::min({v0.z, v1.z, v2.z});
    float triMaxZ = std::max({v0.z, v1.z, v2.z});
    if (triMaxZ < vMin.z || triMinZ > vMax.z) return false;
    return true; 
}