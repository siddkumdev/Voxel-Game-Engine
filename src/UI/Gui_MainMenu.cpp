#include "Gui.h"
#include <imgui.h>

static bool showSaveModal = false;
static bool showLoadModal = false;
static char filenameBuffer[128] = "level.dat";

void GUI::DrawMainMenu(Application& app) {
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
}

void GUI::DrawPopups(Application& app) {
    if (showSaveModal) { ImGui::OpenPopup("Save Level"); showSaveModal = false; }
    if (showLoadModal) { ImGui::OpenPopup("Load Level"); showLoadModal = false; }

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
