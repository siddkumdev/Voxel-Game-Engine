#include "model_loader.h"
#include <iostream>
#include <cstdio>

bool ModelLoader::LoadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_vertices;
    char line[128];

    while (fgets(line, 128, file)) {
        // Parse Vertex Position
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 v;
            if (sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z) == 3) {
                temp_vertices.push_back(v);
            }
        } 
        // Parse Faces (Triangles)
        else if (line[0] == 'f' && line[1] == ' ') {
            int v1, v2, v3;
            // Robust parsing for: "f v1 v2 v3", "f v1/vt1/vn1 ...", "f v1//vn1 ..."
            if (sscanf(line, "f %d/%*s %d/%*s %d/%*s", &v1, &v2, &v3) == 3 ||
                sscanf(line, "f %d//%*d %d//%*d %d//%*d", &v1, &v2, &v3) == 3 ||
                sscanf(line, "f %d %d %d", &v1, &v2, &v3) == 3) 
            {
                // OBJ uses 1-based indexing; convert to 0-based
                // We "unroll" the vertices to match your project's previous layout
                out_vertices.push_back(temp_vertices[v1 - 1]);
                out_indices.push_back((int)out_indices.size());

                out_vertices.push_back(temp_vertices[v2 - 1]);
                out_indices.push_back((int)out_indices.size());

                out_vertices.push_back(temp_vertices[v3 - 1]);
                out_indices.push_back((int)out_indices.size());
            }
        }
    }

    fclose(file);
    return true;
}

bool ModelLoader::LoadGLB(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    std::cerr << "GLB loading not implemented." << std::endl;
    return false;
}

bool ModelLoader::LoadFBX(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    std::cerr << "FBX loading not implemented." << std::endl;
    return false;
}