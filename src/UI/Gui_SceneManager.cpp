#include "Gui.h"
#include <imgui.h>

void GUI::DrawSceneManager(Application& app) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    float panelWidth = 250.0f;

    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, workSize.y));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Scene Manager", nullptr, flags);

    ImGui::Text("Objects: %d", (int)app.GetSceneObjects().size());
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    // Create Objects
    if (ImGui::Button("Cube", ImVec2(70, 0))) {
        Chunk* c = new Chunk(16);
        c->GenerateCube();
        c->name = "Cube " + std::to_string(app.GetSceneObjects().size());
        c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sphere", ImVec2(70, 0))) {
        Chunk* c = new Chunk(32);
        c->GenerateSphere(15);
        c->name = "Sphere " + std::to_string(app.GetSceneObjects().size());
        c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cyl", ImVec2(70, 0))) {
        Chunk* c = new Chunk(20);
        c->GenerateCylinder(8, 16);
        c->name = "Cylinder " + std::to_string(app.GetSceneObjects().size());
        c->physicsBody.isStatic = true;
        app.AddChunk(c);
    }

    ImGui::Separator();

    // List
    ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);
    Chunk* selected = app.GetSceneObjects().empty() ? nullptr : app.GetSelectedObject();

    for (int i = 0; i < app.GetSceneObjects().size(); i++) {
        Chunk* c = app.GetSceneObjects()[i];
        ImGui::PushID(i);
        std::string label = c->name.empty() ? "Object " + std::to_string(i) : c->name;

        if (ImGui::Selectable(label.c_str(), selected == c)) {
            app.GetSelectedObject() = c;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}
