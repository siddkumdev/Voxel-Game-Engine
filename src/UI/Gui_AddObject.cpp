#include "Gui.h"
#include "Application.h"
#include "Chunk.h"
#include "Explosion.h"
#include <imgui.h>
#include <string>

void GUI::DrawAddObjectButton(Application& app) {

    if (ImGui::Button("Add Object", ImVec2(-FLT_MIN, 0.0f))) {
        ImGui::OpenPopup("AddObjectMenu");
    }

    if (ImGui::BeginPopup("AddObjectMenu")) {

        if (ImGui::MenuItem("Cube")) {
            Chunk* c = new Chunk(16);
            c->GenerateCube();
            c->name = "Cube " + std::to_string(app.GetSceneObjects().size());
            c->isStatic = true;
            c->type = ObjectType::CHUNK; 
            app.AddObject(c); 
        }

        if (ImGui::MenuItem("Sphere")) {
            Chunk* c = new Chunk(32);
            c->GenerateSphere(15);
            c->name = "Sphere " + std::to_string(app.GetSceneObjects().size());
            c->isStatic = true;
            c->type = ObjectType::CHUNK;
            app.AddObject(c);
        }

        if (ImGui::MenuItem("Cylinder")) {
            Chunk* c = new Chunk(20);
            c->GenerateCylinder(8, 16);
            c->name = "Cylinder " + std::to_string(app.GetSceneObjects().size());
            c->isStatic = true;
            c->type = ObjectType::CHUNK;
            app.AddObject(c);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Point Explosion")) {
            PointExplosion* exp = new PointExplosion();
            exp->name = "Explosion " + std::to_string(app.GetSceneObjects().size());

            exp->type = ObjectType::EXPLOSION; 

            exp->position = glm::vec3(0.0f, 5.0f, 0.0f);
            exp->data.center = exp->position;

            app.AddObject(exp);
        }

        ImGui::EndPopup();
    }
}
