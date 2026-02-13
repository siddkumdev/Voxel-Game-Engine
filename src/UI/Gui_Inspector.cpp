#include "Gui.h"
#include <imgui.h>
#include <cstring>
#include "voxelizer.h"

void GUI::DrawInspector(Application& app) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 workPos = vp->WorkPos;
    ImVec2 workSize = vp->WorkSize;
    float width = 300.0f;

    ImGui::SetNextWindowPos(ImVec2(workPos.x + workSize.x - width, workPos.y));
    ImGui::SetNextWindowSize(ImVec2(width, workSize.y));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Inspector", nullptr, flags);

    Chunk* selected = app.GetSelectedObject();
    if (selected) {
        char nameBuf[64];
        strncpy(nameBuf, selected->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = 0;
        if(ImGui::InputText("Name", nameBuf, 64)) selected->name = nameBuf;

        ImGui::Separator();

        // Transform
        ImGui::TextColored(ImVec4(1,1,0,1), "Transform");
        float speed = ImGui::GetIO().KeyShift ? 0.0001f : 0.1f;
        float rotSpeed = ImGui::GetIO().KeyShift ? 0.001f : 1.0f;

        ImGui::DragFloat3("Pos", &selected->physicsBody.position[0], speed);
        ImGui::DragFloat3("Rot", &selected->physicsBody.rotation[0], rotSpeed);
        ImGui::DragFloat3("Scale", &selected->physicsBody.scale[0], speed);

        ImGui::Separator();

        // Physics
        ImGui::TextColored(ImVec4(1,1,0,1), "Properties");
        ImGui::Checkbox("Static", &selected->physicsBody.isStatic);
        ImGui::SameLine();
        ImGui::Checkbox("Gravity", &selected->physicsBody.useGravity);
        ImGui::DragFloat("Resistance", &selected->physicsBody.resistance, 0.5f, 0.0f, 100.0f);
        ImGui::ColorEdit3("Color", (float*)&selected->color);

        ImGui::Separator();

        // Voxel Ops
        ImGui::TextColored(ImVec4(1,1,0,1), "Voxel Ops");
        if (ImGui::Button("Rebuild Mesh", ImVec2(-1, 0))) selected->UpdateMesh();

        ImGui::Spacing();
        ImGui::Text("Import Mesh (.obj/.glb)");
        static char modelPath[128] = "../models/teapot.obj";
        ImGui::InputText("##ModelPath", modelPath, 128);
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            if (Voxelizer::LoadAndVoxelize(modelPath, *selected)) selected->name = modelPath;
        }

        ImGui::Spacing();
        int res = selected->GetResolution();
        if (ImGui::SliderInt("Resolution", &res, 2, 128)) selected->SetResolution(res);

        ImGui::Separator();

        // Destruction
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "DESTRUCTION");
        ImGui::DragFloat("Radius", &app.m_ExplosionRadius, 0.1f, 0.1f, 50.0f);
        ImGui::DragFloat("Force", &app.m_ExplosionForce, 1.0f, 0.0f, 1000.0f);

        if (ImGui::Checkbox("CLICK TO EXPLODE", &app.m_ExplosionMode)) {
            if (app.m_ExplosionMode) app.GetSelectedObject() = nullptr;
        }

        ImGui::BeginDisabled(selected == nullptr);
        if (ImGui::Button("Detonate Selected", ImVec2(-1, 0))) {
            glm::vec3 center = selected->physicsBody.position + (selected->GetWorldSize() * 0.5f);
            app.TriggerExplosion(center);
        }
        ImGui::EndDisabled();

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Delete Object", ImVec2(-1, 0))) app.DeleteSelectedObject();
        ImGui::PopStyleColor();

    } else {
        ImGui::TextDisabled("No object selected");
    }
    ImGui::End();
}
