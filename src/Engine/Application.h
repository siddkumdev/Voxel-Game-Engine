#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include "camera.h"
#include "Chunk.h"
#include "shader.h"

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();

    // Callbacks
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    // Getters for GUI
    std::vector<Chunk*>& GetSceneObjects() { return m_SceneObjects; }
    Chunk*& GetSelectedObject() { return m_SelectedObject; }
    Camera& GetCamera() { return m_Camera; }

    // Public method to add objects (used by GUI)
    void AddChunk(Chunk* chunk);
    void DeleteSelectedObject();

    // Level management
    void NewLevel();
    void SaveLevel(const std::string& path);
    void LoadLevel(const std::string& path);

    // Explosion trigger (for testing)
    void TriggerExplosion(glm::vec3 center); // NEW: Helper to spawn explosion

    // Public variables for GUI to access directly
    bool m_ExplosionMode = false;
    float m_ExplosionRadius = 5.0f;
    float m_ExplosionForce = 50.0f;    

private:
    struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};
    static bool IntersectRayAABB(const Ray& ray, glm::vec3 boxMin, glm::vec3 boxMax, float& tMin);
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    void HandleSelection(float xpos, float ypos, int width, int height);

    // Internal Input Handlers
    void OnMouse(double xpos, double ypos);
    void OnScroll(double xoffset, double yoffset);
    void OnMouseButton(int button, int action, int mods);

    GLFWwindow* m_Window = nullptr;
    const int SCR_WIDTH = 1280;
    const int SCR_HEIGHT = 720;

    Camera m_Camera;
    Shader* m_Shader = nullptr;

    std::vector<Chunk*> m_SceneObjects;
    Chunk* m_SelectedObject = nullptr;

    // Time
    float m_DeltaTime = 0.0f;
    float m_LastFrame = 0.0f;

    // Input State
    float m_LastX;
    float m_LastY;
    bool m_FirstMouse = true;
    bool m_RightClickHolding = false;
    bool m_MouseClicked = false;

    unsigned int m_DebugCubeVAO = 0;
    unsigned int m_DebugCubeVBO = 0;
    void InitDebugCube();
    void RenderDebugBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color);
};
