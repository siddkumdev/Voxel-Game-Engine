#include "Gui.h"
#include <imgui.h>
#include <string>
#include "Application.h"
#include "Scene/SceneObject.h"
#include "type.h"

// #include "Gui_AddObject.h" 

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

    // ---------------------------------------------------------
    // CALL THE ADD FUNCTION
    // ---------------------------------------------------------
    // Ensure this function is declared in Gui.h or its own header
    GUI::DrawAddObjectButton(app);
    // ---------------------------------------------------------

    ImGui::Separator();

    // Object List Logic
    ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);
    
    // FIX 1: Use SceneObject* (Base class), not Chunk*
    SceneObject* selected = app.GetSceneObjects().empty() ? nullptr : app.GetSelectedObject();

    for (int i = 0; i < app.GetSceneObjects().size(); i++) {
        // FIX 2: Iterate as SceneObject*
        SceneObject* obj = app.GetSceneObjects()[i];
        
        ImGui::PushID(i);
        // SceneObject has 'name', so we don't need to cast to Chunk here
        std::string label = obj->name.empty() ? "Object " + std::to_string(i) : obj->name;

        // Optional: Add a prefix for Explosions to make them distinct
        if (obj->type == ObjectType::EXPLOSION) {
            label = "[EXP] " + label;
        }

        if (ImGui::Selectable(label.c_str(), selected == obj)) {
            // FIX 3: Use a setter. You cannot assign to a return value (GetSelectedObject() = c is invalid).
            // You must add 'void SetSelectedObject(SceneObject* obj) { m_SelectedObject = obj; }' to Application.h
            app.SetSelectedObject(obj); 
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}