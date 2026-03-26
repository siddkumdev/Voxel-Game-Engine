#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Application.h"

class GUI {
public:
    static void Init(GLFWwindow* window);
    static void BeginFrame();
    static void Render(Application& app);
    static void EndFrame();
    static void Shutdown();

    static void DrawMainMenu(Application& app);
    static void DrawSceneManager(Application& app);
    static void DrawInspector(Application& app);
    static void DrawPopups(Application& app);

    static void DrawAddObjectButton(Application& app);
};
