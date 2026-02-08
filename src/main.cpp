#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "shader.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


#include "camera.h"
#include "chunk.h"
#include "voxelizer.h"

// --- GLOBALS ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera camera(glm::vec3(0.0f, 10.0f, 30.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool isRightClickHolding = false;

// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- SCENE MANAGEMENT ---
std::vector<Chunk*> sceneObjects; // List of all objects
Chunk* selectedObject = nullptr;  // Pointer to the currently selected object

// --- INPUT HELPERS ---
bool mouseClicked = false; // Flag to handle single clicks

// Forward Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void RenderUI();
void SelectObject(float mouseX, float mouseY, int screenW, int screenH, const glm::mat4& view, const glm::mat4& proj);

// --- INTERSECTION MATH ---
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

// Standard AABB Intersection
bool IntersectRayAABB(const Ray& ray, glm::vec3 boxMin, glm::vec3 boxMax, float& tMin) {
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

int main()
{
    // 1. Setup Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Voxel Engine - Editor Mode", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback); // NEW: Handle clicks

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // 2. ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Shader ourShader("shaders/Basic.glsl");

    // 3. Initial Scene
    // Let's add one default cube to start
    Chunk* c = new Chunk(16, 1.0f);
    c->GenerateCube(); 
    c->name = "Default Cube";
    sceneObjects.push_back(c);

    // 4. Game Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window);

        // --- UPDATE LOGIC ---
        // Handle Object Selection
        if (mouseClicked && !ImGui::GetIO().WantCaptureMouse) {
            // Get window size
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

            SelectObject((float)xpos, (float)ypos, width, height, view, proj);
            mouseClicked = false; // Reset flag
        }

        // --- RENDER START ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        
        // Camera Matrices
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        // Render ALL Objects
        // Inside main loop...
    for (Chunk* chunk : sceneObjects) {
        
        // Check if this object is selected to add a highlight effect
        glm::vec3 finalColor = chunk->color;
        if (chunk == selectedObject) {
            // Brighten selected object slightly
            finalColor += glm::vec3(0.2f); 
        }

        // Send the specific object color to the shader
        int colorLoc = glGetUniformLocation(ourShader.ID, "uColor");
        glUniform3f(colorLoc, finalColor.x, finalColor.y, finalColor.z);
        
        chunk->UpdatePhysics(deltaTime);
        chunk->Render(ourShader, true);
}

        // --- RENDER GUI ---
        RenderUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup memory
    for (Chunk* c : sceneObjects) delete c;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// --- RAYCASTING LOGIC ---
void SelectObject(float mouseX, float mouseY, int screenW, int screenH, const glm::mat4& view, const glm::mat4& proj) {
    // 1. Normalized Device Coordinates
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // 2. Homogeneous Clip Coordinates
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    // 3. Eye Coordinates
    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    // 4. World Coordinates
    glm::vec3 ray_wor = (glm::inverse(view) * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    Ray ray;
    ray.origin = camera.Position;
    ray.direction = ray_wor;

    // 5. Check Intersections
    float closestDist = 10000.0f;
    Chunk* hitObject = nullptr;

    for (Chunk* chunk : sceneObjects) {
        // Calculate AABB for the chunk
        // Min = Position, Max = Position + (ChunkSize * VoxelSize)
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

    selectedObject = hitObject; // Update global selection (can be null if clicked empty space)
}

// --- GUI IMPLEMENTATION ---
void RenderUI() {
    // 1. SCENE HIERARCHY
    ImGui::Begin("Scene Manager");
    ImGui::Text("Objects: %d", (int)sceneObjects.size());
    
    // FPS Counter
    ImGui::Text("Avg: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Add Object Buttons
    if (ImGui::Button("Add Cube")) {
        Chunk* c = new Chunk(16, 1.0f); c->GenerateCube(); 
        c->name = "Cube " + std::to_string(sceneObjects.size()); c->physicsBody.isStatic = true; 
        sceneObjects.push_back(c); selectedObject = c;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Sphere")) {
        Chunk* c = new Chunk(32, 0.5f); c->GenerateSphere(14); 
        c->name = "Sphere " + std::to_string(sceneObjects.size()); c->physicsBody.isStatic = true;
        sceneObjects.push_back(c); selectedObject = c;
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Cylinder")) {
        Chunk* c = new Chunk(20, 1.0f); c->GenerateCylinder(8, 16); 
        c->name = "Cylinder " + std::to_string(sceneObjects.size()); c->physicsBody.isStatic = true;
        sceneObjects.push_back(c); selectedObject = c;
    }

    ImGui::Separator();
    
    // Object List
    for (int i = 0; i < sceneObjects.size(); i++) {
        Chunk* c = sceneObjects[i];
        if (ImGui::Selectable(c->name.c_str(), selectedObject == c)) {
            selectedObject = c;
        }
    }
    ImGui::End();

    // 2. INSPECTOR (The Missing Options are Here)
    if (selectedObject) {
        ImGui::Begin("Inspector");
        
        char nameBuf[64];
        strcpy(nameBuf, selectedObject->name.c_str());
        if(ImGui::InputText("Name", nameBuf, 64)) selectedObject->name = nameBuf;

        ImGui::Separator();

        // --- TRANSFORM ---
        ImGui::TextColored(ImVec4(1,1,0,1), "Transform");
        ImGui::DragFloat3("Position", &selectedObject->physicsBody.position[0], 0.1f);
        ImGui::DragFloat3("Rotation", &selectedObject->physicsBody.rotation[0], 1.0f);
        
        // --- PHYSICS ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Physics");
        ImGui::Checkbox("Static", &selectedObject->physicsBody.isStatic);
        ImGui::SameLine();
        ImGui::Checkbox("Gravity", &selectedObject->physicsBody.useGravity);

        // --- VOXEL DATA (Restored Options) ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Voxel Settings");

        // 1. Color Picker
        ImGui::ColorEdit3("Object Color", (float*)&selectedObject->color);

        // 2. Maintain World Size
        static bool maintainSize = true;
        ImGui::Checkbox("Maintain World Size", &maintainSize);

        // 3. Voxel Scale Slider (Smart Sync)
        // We calculate the current 'step' from the object's actual size
        // so the slider is always in the correct position when you select an object.
        float currentSize = selectedObject->GetVoxelSize();
        int currentStep = (int)(std::log2(currentSize)); 
        
        if (ImGui::SliderInt("Voxel Scale", &currentStep, -3, 2, "2^%d")) {
            float newSize = std::pow(2.0f, (float)currentStep);
            selectedObject->SetVoxelSize(newSize, maintainSize);
        }
        ImGui::Text("Voxel Size: %.4f m", currentSize);

        // 4. Rebuild Mesh
        if (ImGui::Button("Rebuild Mesh")) {
            selectedObject->UpdateMesh();
        }

        // --- IMPORT MODEL ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Import .OBJ");
        
        static char modelPath[256] = "models/teapot.obj";
        ImGui::InputText("File Path", modelPath, 256);

        if (ImGui::Button("Import & Voxelize")) {
            Voxelizer voxelizer;
            std::string fullPath = "../" + std::string(modelPath);
            
            // Pass the *selectedObject* to the voxelizer
            if (voxelizer.LoadAndVoxelize(fullPath, *selectedObject)) {
                selectedObject->UpdateMesh();
            } else {
                std::cout << "Failed to load model: " << fullPath << std::endl;
            }
        }
        
        // DELETE BUTTON
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Delete Object", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
             auto it = std::find(sceneObjects.begin(), sceneObjects.end(), selectedObject);
             if (it != sceneObjects.end()) {
                 sceneObjects.erase(it);
                 delete selectedObject;
                 selectedObject = nullptr;
             }
        }
        ImGui::PopStyleColor();
        
        ImGui::End();
    }
}
// --- BOILERPLATE CALLBACKS ---
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouseClicked = true;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Right-click logic for camera movement
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!isRightClickHolding) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; 
            isRightClickHolding = true;
        }
        bool sprint = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime, sprint);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime, sprint);
    } else {
        if (isRightClickHolding) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isRightClickHolding = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!isRightClickHolding) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}