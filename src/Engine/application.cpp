#include "Application.h"
#include "Gui.h"
#include <iostream>
#include <imgui.h>

Application::Application()
    : m_Camera(glm::vec3(0.0f, 0.0f, 2.0f))
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
    Chunk* c = new Chunk(16);
    c->GenerateCube();
    c->name = "Default Cube";
    c->physicsBody.isStatic = true;
    m_SceneObjects.push_back(c);

    return true;
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_Window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        m_DeltaTime = currentFrame - m_LastFrame;
        m_LastFrame = currentFrame;

        glfwPollEvents();
        GUI::BeginFrame();

        ProcessInput();

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
