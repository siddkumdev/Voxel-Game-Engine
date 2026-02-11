#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "core.h"

class GUI {
public:
    static void Init(GLFWwindow* window);
    static void BeginFrame();
    static void Render(Application& app);
    static void EndFrame();
    static void Shutdown();
};
