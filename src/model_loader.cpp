#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "model_loader.h"
#include <iostream>

bool ModelLoader::LoadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str())) {
        std::cerr << "Failed to load OBJ: " << err << std::endl;
        return false;
    }

    // Combine all shapes into one mesh
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 vertex;
            vertex.x = attrib.vertices[3 * index.vertex_index + 0];
            vertex.y = attrib.vertices[3 * index.vertex_index + 1];
            vertex.z = attrib.vertices[3 * index.vertex_index + 2];

            out_vertices.push_back(vertex);
            out_indices.push_back((int)out_indices.size());
        }
    }
    return true;
}

bool ModelLoader::LoadGLB(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    std::cerr << "GLB loading not implemented yet." << std::endl;
    return false;
}

bool ModelLoader::LoadFBX(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices) {
    std::cerr << "FBX loading not implemented yet." << std::endl;
    return false;
}
