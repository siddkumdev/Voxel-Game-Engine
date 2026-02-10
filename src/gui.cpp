#include "gui.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstring>
#include "voxelizer.h"

// Helper for file dialog state
static bool showSaveModal = false;
static bool showLoadModal = false;
static char filenameBuffer[128] = "level.dat";

void GUI::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    // Styling: Make it look a bit more like an editor
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f; // Flat corners for docked look
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void GUI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Render(Application& app) {
    // --- 1. MAIN MENU BAR ---
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Level")) {
                app.NewLevel();
            }
            if (ImGui::MenuItem("Open...")) {
                showLoadModal = true;
            }
            if (ImGui::MenuItem("Save As...")) {
                showSaveModal = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // --- 2. LAYOUT CALCULATION ---
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos; 
    ImVec2 workSize = viewport->WorkSize;

    float leftPanelWidth = 250.0f;
    float rightPanelWidth = 300.0f;

    // --- 3. SCENE MANAGER (Pinned Left) ---
    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, workSize.y));
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    ImGui::Begin("Scene Manager", nullptr, windowFlags);
    
    ImGui::Text("Objects: %d", (int)app.GetSceneObjects().size());
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Creation Buttons
    if (ImGui::Button("Cube", ImVec2(70, 0))) {
        Chunk* c = new Chunk(16); c->GenerateCube();
        c->name = "Cube " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sphere", ImVec2(70, 0))) {
        Chunk* c = new Chunk(32); c->GenerateSphere(15);
        c->name = "Sphere " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cyl", ImVec2(70, 0))) {
        Chunk* c = new Chunk(20); c->GenerateCylinder(8, 16);
        c->name = "Cylinder " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }

    ImGui::Separator();

    // Object List
    ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);
        Chunk* selectedObject = app.GetSceneObjects().size() > 0 ? app.GetSelectedObject() : nullptr;
        
        for (int i = 0; i < app.GetSceneObjects().size(); i++) {
            Chunk* c = app.GetSceneObjects()[i];
            
            // FIX: Push a unique ID based on the index loop
            ImGui::PushID(i); 
            
            // Optional: meaningful name if empty
            std::string label = c->name.empty() ? "Object " + std::to_string(i) : c->name;
            
            if (ImGui::Selectable(label.c_str(), selectedObject == c)) {
                app.GetSelectedObject() = c;
            }
            
            // FIX: Pop the ID after the item is drawn
            ImGui::PopID(); 
        }
    ImGui::EndChild();

    ImGui::End();

    // --- 4. INSPECTOR (Pinned Right) ---
    ImGui::SetNextWindowPos(ImVec2(workPos.x + workSize.x - rightPanelWidth, workPos.y));
    ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, workSize.y));

ImGui::Begin("Inspector", nullptr, windowFlags);
    
    selectedObject = app.GetSelectedObject(); // Refresh
    if (selectedObject) {
        char nameBuf[64];
        strncpy(nameBuf, selectedObject->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = 0;
        if(ImGui::InputText("Name", nameBuf, 64)) selectedObject->name = nameBuf;

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Transform");

        // Check if Shift is held down
        bool isShiftHeld = ImGui::GetIO().KeyShift;

        // Define speeds
        float linearSpeed = isShiftHeld ? 0.0001f : 0.1f;
        float rotSpeed = isShiftHeld ? 0.001f : 1.0f;

        ImGui::DragFloat3("Pos", &selectedObject->physicsBody.position[0], linearSpeed);
        ImGui::DragFloat3("Rot", &selectedObject->physicsBody.rotation[0], rotSpeed);
        ImGui::DragFloat3("Scale", &selectedObject->physicsBody.scale[0], linearSpeed);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Properties");
        ImGui::Checkbox("Static", &selectedObject->physicsBody.isStatic);
        ImGui::SameLine();
        ImGui::Checkbox("Gravity", &selectedObject->physicsBody.useGravity);
        ImGui::DragFloat("Resistance", &selectedObject->physicsBody.resistance, 0.5f, 0.0f, 100.0f);
        ImGui::ColorEdit3("Color", (float*)&selectedObject->color);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Voxel Ops");
        
        if (ImGui::Button("Rebuild Mesh", ImVec2(-1, 0))) {
            selectedObject->UpdateMesh();
        }
        
        // --- NEW: IMPORT & VOXELIZE ---
        ImGui::Spacing();
        ImGui::Text("Import Mesh (.obj/.glb)");
        
        // FIX: Default to "../models/" because the .exe is inside /build/
        static char modelPath[128] = "../models/teapot.obj"; 
        
        ImGui::InputText("##ModelPath", modelPath, 128);
        ImGui::SameLine();
        
        if (ImGui::Button("Load")) {
            if (Voxelizer::LoadAndVoxelize(modelPath, *selectedObject)) {
                // Set the name to the filename for clarity
                selectedObject->name = modelPath;
            }
        }
        // ------------------------------
        
        ImGui::Spacing();        
        int currentRes = selectedObject->GetResolution();
        if (ImGui::SliderInt("Grid Resolution", &currentRes, 2, 128)) {
            selectedObject->SetResolution(currentRes);
            }

            ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "DESTRUCTION TOOLS");

        // 1. Explosion Settings
        ImGui::DragFloat("Explosion Radius", &app.m_ExplosionRadius, 0.1f, 0.1f, 50.0f);
        ImGui::DragFloat("Explosion Force", &app.m_ExplosionForce, 1.0f, 0.0f, 1000.0f);

        ImGui::Spacing();

        // 2. Mode Toggle
        if (ImGui::Checkbox("💥 CLICK TO EXPLODE", &app.m_ExplosionMode)) {
            if (app.m_ExplosionMode) app.GetSelectedObject() = nullptr;
        }

        ImGui::Spacing();

        // 3. Manual Trigger Button
        Chunk* selected = app.GetSelectedObject();
        ImGui::BeginDisabled(selected == nullptr); 
        if (ImGui::Button("Detonate Selected Object", ImVec2(-1, 0))) {
            if (selected) {
                glm::vec3 center = selected->physicsBody.position + (selected->GetWorldSize() * 0.5f);
                app.TriggerExplosion(center);
            }
        }
        ImGui::EndDisabled();
        
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Delete Object", ImVec2(-1, 0))) {
             app.DeleteSelectedObject();
        }
        ImGui::PopStyleColor();

    } else {
        ImGui::TextDisabled("No object selected");
    }
    ImGui::End();
    // --- 5. POPUPS ---
    if (showSaveModal) {
        ImGui::OpenPopup("Save Level");
        showSaveModal = false;
    }
    if (showLoadModal) {
        ImGui::OpenPopup("Load Level");
        showLoadModal = false;
    }

    if (ImGui::BeginPopupModal("Save Level", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", filenameBuffer, 128);
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            app.SaveLevel(filenameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Load Level", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", filenameBuffer, 128);
        if (ImGui::Button("Load", ImVec2(120, 0))) {
            app.LoadLevel(filenameBuffer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

void GUI::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}