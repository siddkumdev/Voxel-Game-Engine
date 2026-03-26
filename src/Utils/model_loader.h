#pragma once
#include <vector>
#include <string>
#include <glm.hpp>

class ModelLoader {
public:
    static bool LoadOBJ(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices);
    static bool LoadGLB(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices);
    static bool LoadFBX(const std::string& path, std::vector<glm::vec3>& out_vertices, std::vector<int>& out_indices);
};
