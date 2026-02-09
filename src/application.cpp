#include "application.h"
#include "gui.h"
#include "voxelizer.h"
#include "model_loader.h"

#include <iostream>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <imgui.h>

// --- HELPER MATH ---
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

static bool IntersectRayAABB(const Ray& ray, glm::vec3 boxMin, glm::vec3 boxMax, float& tMin) {
    glm::vec3 invDir = 1.0f / ray.direction;
    glm::vec3 t1 = (boxMin - ray.origin) * invDir;
    glm::vec3 t2 = (boxMax - ray.origin) * invDir;

    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar  = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (tNear > tFar || tFar < 0.0f) return false;

    tMin = tNear;
    return true;
}

// --- CALLBACK IMPLEMENTATION ---
void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnMouse(xposIn, yposIn);
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnScroll(xoffset, yoffset);
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnMouseButton(button, action, mods);
}

// --- APPLICATION IMPLEMENTATION ---

Application::Application()
    : m_Camera(glm::vec3(0.0f, 10.0f, 30.0f))
{
    m_LastX = 1280 / 2.0f;
    m_LastY = 720 / 2.0f;
}

Application::~Application() {
    for (Chunk* c : m_SceneObjects) delete c;
    delete m_Shader;
    GUI::Shutdown();
    if (m_Window) glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool Application::Init() {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(1280, 720, "Voxel Engine - Editor Mode", NULL, NULL);
    if (m_Window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(m_Window, MouseCallback);
    glfwSetScrollCallback(m_Window, ScrollCallback);
    glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    GUI::Init(m_Window);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_Shader = new Shader("shaders/Basic.glsl");

    // Default Cube
    Chunk* c = new Chunk(16, 1.0f);
    c->GenerateCube();
    c->name = "Default Cube";
    c->physicsBody.isStatic = true;
    m_SceneObjects.push_back(c);

    // Default selection
    // m_SelectedObject = c;

    return true;
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_Window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        glfwPollEvents();

        // Start GUI Frame early for input handling
        GUI::BeginFrame();

        ProcessInput();

        // Handle Selection (Deferred from click)
        if (m_MouseClicked) {
             if (!ImGui::GetIO().WantCaptureMouse) {
                int width, height;
                glfwGetWindowSize(m_Window, &width, &height);
                double xpos, ypos;
                glfwGetCursorPos(m_Window, &xpos, &ypos);
                HandleSelection((float)xpos, (float)ypos, width, height);
             }
             m_MouseClicked = false;
        }

        Update(m_DeltaTime);
        Render();
    }
}

void Application::Update(float deltaTime) {
    for (Chunk* chunk : m_SceneObjects) {
         chunk->UpdatePhysics(deltaTime);
    }
}

void Application::ProcessInput() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!m_RightClickHolding) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_FirstMouse = true;
            m_RightClickHolding = true;
        }
        bool sprint = (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) m_Camera.ProcessKeyboard(FORWARD, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) m_Camera.ProcessKeyboard(BACKWARD, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) m_Camera.ProcessKeyboard(LEFT, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) m_Camera.ProcessKeyboard(RIGHT, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS) m_Camera.ProcessKeyboard(UP, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS) m_Camera.ProcessKeyboard(DOWN, m_DeltaTime, sprint);
    } else {
        if (m_RightClickHolding) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_RightClickHolding = false;
        }
    }
}

void Application::Render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Shader->use();

    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.1f, 100.0f);
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (height > 0)
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    m_Shader->setMat4("view", view);
    m_Shader->setMat4("projection", projection);

    for (Chunk* chunk : m_SceneObjects) {
        glm::vec3 finalColor = chunk->color;
        if (chunk == m_SelectedObject) {
            finalColor += glm::vec3(0.2f);
        }

        int colorLoc = glGetUniformLocation(m_Shader->ID, "uColor");
        glUniform3f(colorLoc, finalColor.x, finalColor.y, finalColor.z);

        chunk->Render(*m_Shader, true);
    }

    // Render GUI
    GUI::Render(*this);
    GUI::EndFrame(); // Finishes rendering GUI

    glfwSwapBuffers(m_Window);
}

void Application::HandleSelection(float mouseX, float mouseY, int screenW, int screenH) {
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 100.0f);
    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::vec3 ray_wor = (glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    Ray ray;
    ray.origin = m_Camera.Position;
    ray.direction = ray_wor;

    float closestDist = 10000.0f;
    Chunk* hitObject = nullptr;

    for (Chunk* chunk : m_SceneObjects) {
        glm::vec3 minB = chunk->GetPosition();
        float worldSize = chunk->GetWorldSize();
        glm::vec3 maxB = minB + glm::vec3(worldSize);

        float dist;
        if (IntersectRayAABB(ray, minB, maxB, dist)) {
            if (dist < closestDist) {
                closestDist = dist;
                hitObject = chunk;
            }
        }
    }

    m_SelectedObject = hitObject;
}

void Application::AddChunk(Chunk* chunk) {
    m_SceneObjects.push_back(chunk);
    m_SelectedObject = chunk;
}

void Application::DeleteSelectedObject() {
    if (!m_SelectedObject) return;
    auto it = std::find(m_SceneObjects.begin(), m_SceneObjects.end(), m_SelectedObject);
    if (it != m_SceneObjects.end()) {
        m_SceneObjects.erase(it);
        delete m_SelectedObject;
        m_SelectedObject = nullptr;
    }
}

void Application::OnMouse(double xposIn, double yposIn) {
    if (!m_RightClickHolding) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (m_FirstMouse) { m_LastX = xpos; m_LastY = ypos; m_FirstMouse = false; }
    float xoffset = xpos - m_LastX;
    float yoffset = m_LastY - ypos;
    m_LastX = xpos; m_LastY = ypos;
    m_Camera.ProcessMouseMovement(xoffset, yoffset);
}

void Application::OnScroll(double xoffset, double yoffset) {
    m_Camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Application::OnMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        m_MouseClicked = true;
    }
}
