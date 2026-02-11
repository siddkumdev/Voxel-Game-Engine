#include "core.h"
#include "gui.h"
#include "voxelizer.h"
#include "serializer.h" // Ensure this is included

#include <iostream>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <imgui.h>

// --- HELPER MATH ---


bool Application::IntersectRayAABB(const Application::Ray& ray, glm::vec3 boxMin, glm::vec3 boxMax, float& tMin) {
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



// --- APPLICATION IMPLEMENTATION ---

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

    // Default Cube - FIXED CONSTRUCTOR
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

    if (m_SelectedObject) {
        // Get the EXACT box the physics engine sees
        std::pair<glm::vec3, glm::vec3> aabb = m_SelectedObject->GetAABB();
        
        // Draw Red Box for Hitbox
        RenderDebugBox(aabb.first, aabb.second, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    GUI::Render(*this);
    GUI::EndFrame();
    glfwSwapBuffers(m_Window);
}

// application.cpp


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


void Application::TriggerExplosion(glm::vec3 target) {
    PointExplosion exp;
    exp.center = target;
    exp.radius = m_ExplosionRadius;       // Radius in world units
    exp.force = m_ExplosionForce;       // Strength
    exp.falloff = true;
    exp.falloffFactor = 1.0f;

    std::vector<Chunk*> allNewDebris;

    for (Chunk* c : m_SceneObjects) {
        // Run explode on all chunks
        std::vector<Chunk*> debris = c->Explode(exp);
        allNewDebris.insert(allNewDebris.end(), debris.begin(), debris.end());
    }

    // Add generated debris to scene
    for (Chunk* d : allNewDebris) {
        m_SceneObjects.push_back(d);
    }
}

void Application::Update(float deltaTime) {
    // 1. Physics Integration
    for (Chunk* chunk : m_SceneObjects) {
         chunk->UpdatePhysics(deltaTime);
    }

    // 2. Naive Collision Detection (O(N^2))
    // In a real engine, use a Spatial Hash or Octree
    for (size_t i = 0; i < m_SceneObjects.size(); i++) {
        for (size_t j = i + 1; j < m_SceneObjects.size(); j++) {
            Chunk* c1 = m_SceneObjects[i];
            Chunk* c2 = m_SceneObjects[j];
            
            if (c1->CheckCollision(*c2)) {
                c1->ResolveCollision(*c2);
            }
        }
    }
}

void Application::NewLevel() {
    for (Chunk* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    // FIXED CONSTRUCTOR
    Chunk* c = new Chunk(16);
    c->GenerateCube();
    c->name = "Default Cube";
    c->physicsBody.isStatic = true;
    m_SceneObjects.push_back(c);
}

void Application::SaveLevel(const std::string& path) {
    if (LevelSerializer::SaveLevel(path, m_SceneObjects)) {
        std::cout << "Level saved successfully to " << path << std::endl;
    } else {
        std::cout << "Failed to save level!" << std::endl;
    }
}

void Application::LoadLevel(const std::string& path) {
    for (Chunk* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    if (LevelSerializer::LoadLevel(path, m_SceneObjects)) {
        std::cout << "Level loaded: " << path << std::endl;
    } else {
        std::cout << "Failed to load level!" << std::endl;
        NewLevel();
    }
}

void Application::InitDebugCube() {
    float vertices[] = {
        // Line loop for bottom face
        0,0,0, 1,0,0,  1,0,0, 1,0,1,  1,0,1, 0,0,1,  0,0,1, 0,0,0,
        // Line loop for top face
        0,1,0, 1,1,0,  1,1,0, 1,1,1,  1,1,1, 0,1,1,  0,1,1, 0,1,0,
        // Vertical connectors
        0,0,0, 0,1,0,  1,0,0, 1,1,0,  1,0,1, 1,1,1,  0,0,1, 0,1,1
    };
    glGenVertexArrays(1, &m_DebugCubeVAO);
    glGenBuffers(1, &m_DebugCubeVBO);
    glBindVertexArray(m_DebugCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_DebugCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// [Insert helper function]
void Application::RenderDebugBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
    if (m_DebugCubeVAO == 0) InitDebugCube();

    glm::vec3 size = max - min;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, min);
    model = glm::scale(model, size);

    m_Shader->setMat4("model", model);
    int colorLoc = glGetUniformLocation(m_Shader->ID, "uColor");
    glUniform3f(colorLoc, color.x, color.y, color.z);

    glBindVertexArray(m_DebugCubeVAO);
    glDrawArrays(GL_LINES, 0, 24);
}