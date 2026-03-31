#include "Gui.h"
#include <imgui.h>
#include <string>
#include "Application.h"
#include "Scene/SceneObject.h"
#include "type.h"

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

    GUI::DrawAddObjectButton(app);

    ImGui::Separator();

    ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);

    SceneObject* selected = app.GetSceneObjects().empty() ? nullptr : app.GetSelectedObject();

    for (int i = 0; i < app.GetSceneObjects().size(); i++) {

        SceneObject* obj = app.GetSceneObjects()[i];

        ImGui::PushID(i);

        std::string label = obj->name.empty() ? "Object " + std::to_string(i) : obj->name;

        if (obj->type == ObjectType::EXPLOSION) {
            label = "[EXP] " + label;
        }

        if (ImGui::Selectable(label.c_str(), selected == obj)) {

            app.SetSelectedObject(obj); 
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}
