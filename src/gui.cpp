#include "gui.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <cstring> // For strcpy
#include "voxelizer.h"

void GUI::Init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void GUI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::Render(Application& app) {
    // 1. SCENE HIERARCHY
    ImGui::Begin("Scene Manager");
    ImGui::Text("Objects: %d", (int)app.GetSceneObjects().size());

    // FPS Counter
    ImGui::Text("Avg: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Add Object Buttons
    if (ImGui::Button("Add Cube")) {
        Chunk* c = new Chunk(16, 1.0f); c->GenerateCube();
        c->name = "Cube " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Sphere")) {
        Chunk* c = new Chunk(32, 0.5f); c->GenerateSphere(14);
        c->name = "Sphere " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Cylinder")) {
        Chunk* c = new Chunk(20, 1.0f); c->GenerateCylinder(8, 16);
        c->name = "Cylinder " + std::to_string(app.GetSceneObjects().size()); c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }

    ImGui::Separator();

    // Object List
    Chunk* selectedObject = app.GetSelectedObject();
    for (int i = 0; i < app.GetSceneObjects().size(); i++) {
        Chunk* c = app.GetSceneObjects()[i];
        if (ImGui::Selectable(c->name.c_str(), selectedObject == c)) {
            app.GetSelectedObject() = c;
        }
    }
    ImGui::End();

    // 2. INSPECTOR
    // Refresh pointer in case it changed (although we use reference/getter)
    selectedObject = app.GetSelectedObject();
    if (selectedObject) {
        ImGui::Begin("Inspector");

        char nameBuf[64];
        strncpy(nameBuf, selectedObject->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = 0;
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

        // --- VOXEL DATA ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Voxel Settings");

        ImGui::ColorEdit3("Object Color", (float*)&selectedObject->color);

        static bool maintainSize = true;
        ImGui::Checkbox("Maintain World Size", &maintainSize);

        float currentSize = selectedObject->GetVoxelSize();
        // Use log2 to map size to slider step
        int currentStep = (int)(std::log2(currentSize));

        if (ImGui::SliderInt("Voxel Scale", &currentStep, -3, 2, "2^%d")) {
            float newSize = std::pow(2.0f, (float)currentStep);
            selectedObject->SetVoxelSize(newSize, maintainSize);
        }
        ImGui::Text("Voxel Size: %.4f m", currentSize);

        if (ImGui::Button("Rebuild Mesh")) {
            selectedObject->UpdateMesh();
        }

        // --- IMPORT MODEL ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1,1,0,1), "Import Model");

        static char modelPath[256] = "models/teapot.obj";
        ImGui::InputText("File Path", modelPath, 256);

        if (ImGui::Button("Import & Voxelize")) {
            Voxelizer voxelizer;
            std::string fullPath = "../" + std::string(modelPath);

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
             app.DeleteSelectedObject();
        }
        ImGui::PopStyleColor();

        ImGui::End();
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
