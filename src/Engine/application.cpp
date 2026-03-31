#include "Application.h"
#include "Gui.h"
#include "Scene/SceneObject.h" 
#include "type.h"
#include <iostream>
#include <algorithm>
#include <imgui.h>

Application::Application()
    : m_Camera(glm::vec3(0.0f, 0.0f, 2.0f))
{
    m_LastX = 1280 / 2.0f;
    m_LastY = 720 / 2.0f;
    m_SelectedObject = nullptr;
}

Application::~Application() {
    for (SceneObject* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();

    delete m_Shader;
    delete m_GizmoShader; 

    GUI::Shutdown();
    if (m_Window) glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool Application::Init() {
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    glEnable(GL_MULTISAMPLE);

    GUI::Init(m_Window);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_Shader = new Shader("shaders/Basic.glsl");

    m_GizmoShader = new Shader("shaders/Gizmo.glsl"); 

    m_TransformGizmo_Renderer.Init();

    Chunk* c = new Chunk(16);
    c->GenerateCube();
    c->name = "Default Cube";
    c->isStatic = true;
    c->type = ObjectType::CHUNK;
    AddObject(c);

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

        if (!ImGui::GetIO().WantCaptureMouse) {
            int width, height;
            glfwGetWindowSize(m_Window, &width, &height);
            double xpos, ypos;
            glfwGetCursorPos(m_Window, &xpos, &ypos);

            float x_ndc = (2.0f * (float)xpos) / width - 1.0f;
            float y_ndc = 1.0f - (2.0f * (float)ypos) / height;
            glm::vec4 ray_clip(x_ndc, y_ndc, -1.0f, 1.0f);

            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
            glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f); 

            glm::mat4 view = m_Camera.GetViewMatrix();
            glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
            glm::vec3 ray_origin = m_Camera.Position;

            m_TransformGizmo.HandleInteraction(ray_origin, ray_world, m_LeftClickHolding, m_SelectedObject);

            if (m_MouseClicked) {

                if (m_SelectedObject == nullptr || m_TransformGizmo.GetHoveredAxis() == GizmoAxis::None) {
                    HandleSelection((float)xpos, (float)ypos, width, height);
                }
                m_MouseClicked = false;
            }
        } else {
            m_MouseClicked = false; 
        }

        Update(m_DeltaTime);
        Render(); 
    }
}

void Application::AddObject(SceneObject* obj) {
    m_SceneObjects.push_back(obj);
    SetSelectedObject(obj); 
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
