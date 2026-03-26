#include "Gui.h"
#include "type.h"
#include "Scene/SceneObject.h"
#include "Chunk.h"      
#include "Explosion.h"  
#include "voxelizer.h"
#include <imgui.h>
#include <cstring>
#include <vector>

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

    SceneObject* selected = app.GetSelectedObject();

    if (selected) {

        char nameBuf[64];
        strncpy(nameBuf, selected->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = 0;
        if(ImGui::InputText("Name", nameBuf, 64)) selected->name = nameBuf;

        ImGui::Separator();

        switch (selected->type) {

            case ObjectType::CHUNK: {
                Chunk* chunk = dynamic_cast<Chunk*>(selected);

                ImGui::TextColored(ImVec4(1,1,0,1), "Transform");
                float speed = ImGui::GetIO().KeyShift ? 0.0001f : 0.1f;
                ImGui::DragFloat3("Pos", &chunk->position[0], speed);
                ImGui::DragFloat3("Rot", &chunk->rotation[0], 1.0f);
                ImGui::DragFloat3("Scale", &chunk->scale[0], speed);

                ImGui::Separator();

                ImGui::TextColored(ImVec4(1,1,0,1), "Properties");
                ImGui::Checkbox("Static", &chunk->isStatic);
                ImGui::SameLine();
                ImGui::Checkbox("Gravity", &chunk->useGravity);
                ImGui::DragFloat("Resistance", &chunk->resistance, 0.5f);
                ImGui::ColorEdit3("Color", (float*)&chunk->color);

                ImGui::Separator();

                if (ImGui::Button("Rebuild Mesh", ImVec2(-1, 0))) chunk->UpdateMesh();

                static char modelPath[128] = "../models/teapot.obj";
                ImGui::InputText("##ModelPath", modelPath, 128);
                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                   if (Voxelizer::LoadAndVoxelize(modelPath, *chunk)) chunk->name = modelPath;
                }
                break;
            }

            case ObjectType::EXPLOSION: {
                PointExplosion* exp = dynamic_cast<PointExplosion*>(selected);

                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Transform");
                float speed = ImGui::GetIO().KeyShift ? 0.0001f : 0.1f;
                ImGui::DragFloat3("Pos", &exp->position[0], speed);
                ImGui::DragFloat3("Rot", &exp->rotation[0], 1.0f);
                ImGui::DragFloat3("Scale", &exp->scale[0], speed);

                ImGui::Separator();

                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Explosion Properties");
                ImGui::Checkbox("Visible", &exp->visible);
                ImGui::DragFloat("Radius", &exp->data.radius, 0.1f, 0.0f, 100.0f);
                ImGui::DragFloat("Force", &exp->data.force, 1.0f, 0.0f, 5000.0f);

                ImGui::Checkbox("Use Falloff", &exp->data.falloff);
                if (exp->data.falloff) {
                    ImGui::SliderFloat("Falloff Factor", &exp->data.falloffFactor, 0.1f, 5.0f);
                }

                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("TRIGGER NOW", ImVec2(-1, 40))) {

                    exp->data.center = exp->position;

                    std::vector<Chunk*> debris;
                    std::vector<Chunk*> targets;

                    for(auto* obj : app.GetSceneObjects()) {
                        if(obj->type == ObjectType::CHUNK) {
                            targets.push_back(dynamic_cast<Chunk*>(obj));
                        }
                    }

                    exp->Detonate(targets, debris);

                    for(Chunk* d : debris) app.AddObject(d);
                }
                ImGui::PopStyleColor();
                break;
            }
        }

        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Delete Object", ImVec2(-1, 0))) app.DeleteSelectedObject();
        ImGui::PopStyleColor();

    } else {
        ImGui::TextDisabled("No object selected");
    }
    ImGui::End();
}
