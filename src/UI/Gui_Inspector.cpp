#include "Gui.h"
#include "type.h"       
#include "Chunk.h"      
#include "Explosion.h"  
#include "voxelizer.h"
#include <imgui.h>
#include <cstring>
#include <vector> // Required for std::vector

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
        // --- COMMON HEADER (Name) ---
        char nameBuf[64];
        strncpy(nameBuf, selected->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = 0;
        if(ImGui::InputText("Name", nameBuf, 64)) selected->name = nameBuf;
        
        ImGui::Separator();

        switch (selected->type) {
            // =========================================================
            // CHUNK INSPECTOR
            // =========================================================
            case ObjectType::CHUNK: {
                Chunk* chunk = dynamic_cast<Chunk*>(selected);

                // Transform
                ImGui::TextColored(ImVec4(1,1,0,1), "Transform");
                float speed = ImGui::GetIO().KeyShift ? 0.0001f : 0.1f;
                ImGui::DragFloat3("Pos", &chunk->physicsBody.position[0], speed);
                ImGui::DragFloat3("Rot", &chunk->physicsBody.rotation[0], 1.0f);
                ImGui::DragFloat3("Scale", &chunk->physicsBody.scale[0], speed);

                ImGui::Separator();

                // Properties
                ImGui::TextColored(ImVec4(1,1,0,1), "Properties");
                ImGui::Checkbox("Static", &chunk->physicsBody.isStatic);
                ImGui::SameLine();
                ImGui::Checkbox("Gravity", &chunk->physicsBody.useGravity);
                ImGui::DragFloat("Resistance", &chunk->physicsBody.resistance, 0.5f);
                ImGui::ColorEdit3("Color", (float*)&chunk->color);

                ImGui::Separator();
                
                // Tools
                if (ImGui::Button("Rebuild Mesh", ImVec2(-1, 0))) chunk->UpdateMesh();
                
                static char modelPath[128] = "../models/teapot.obj";
                ImGui::InputText("##ModelPath", modelPath, 128);
                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                   if (Voxelizer::LoadAndVoxelize(modelPath, *chunk)) chunk->name = modelPath;
                }
                break;
            }

            // =========================================================
            // EXPLOSION INSPECTOR
            // =========================================================
            case ObjectType::EXPLOSION: {
                Explosion* exp = dynamic_cast<Explosion*>(selected);

                // 1. Transform Information (Standard Pos/Rot/Scale)
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Transform");
                float speed = ImGui::GetIO().KeyShift ? 0.0001f : 0.1f;
                ImGui::DragFloat3("Pos", &exp->physicsBody.position[0], speed);
                ImGui::DragFloat3("Rot", &exp->physicsBody.rotation[0], 1.0f);
                ImGui::DragFloat3("Scale", &exp->physicsBody.scale[0], speed);

                ImGui::Separator();

                // 2. All Explosion Properties
                ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Explosion Properties");
                
                ImGui::DragFloat("Radius", &exp->data.radius, 0.1f, 0.0f, 100.0f);
                ImGui::DragFloat("Force", &exp->data.force, 1.0f, 0.0f, 5000.0f);
                
                ImGui::Checkbox("Use Falloff", &exp->data.falloff);
                if (exp->data.falloff) {
                    ImGui::SliderFloat("Falloff Factor", &exp->data.falloffFactor, 0.1f, 5.0f);
                }

                ImGui::Separator();
                
                // 3. Trigger Button
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                if (ImGui::Button("TRIGGER NOW", ImVec2(-1, 40))) {
                    
                    // SYNC: Ensure the explosion happens at the object's current position
                    exp->data.center = exp->physicsBody.position;

                    std::vector<Chunk*> debris;
                    std::vector<Chunk*> targets;
                    
                    // Collect valid targets (Only Chunks)
                    for(auto* obj : app.GetSceneObjects()) {
                        if(obj->type == ObjectType::CHUNK) {
                            targets.push_back(dynamic_cast<Chunk*>(obj));
                        }
                    }
                    
                    // Run logic
                    exp->Detonate(targets, debris);
                    
                    // Register new debris objects
                    for(Chunk* d : debris) app.AddObject(d);
                }
                ImGui::PopStyleColor();
                break;
            }
        }

        // --- COMMON FOOTER (Delete) ---
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("Delete Object", ImVec2(-1, 0))) app.DeleteSelectedObject();
        ImGui::PopStyleColor();

    } else {
        ImGui::TextDisabled("No object selected");
    }
    ImGui::End();
}