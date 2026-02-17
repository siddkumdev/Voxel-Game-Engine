#include "Application.h"
#include "Gui.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <imgui.h>

void Application::Render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Shader->use();

    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.1f, 100.0f);
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (height > 0)
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    m_Shader->setMat4("view", view);
    m_Shader->setMat4("projection", projection);

    for (SceneObject* obj : m_SceneObjects) {
        glm::vec3 finalColor = obj->color;
        if (obj == m_SelectedObject) {
            finalColor += glm::vec3(0.2f);
        }

        int colorLoc = glGetUniformLocation(m_Shader->ID, "uColor");
        glUniform3f(colorLoc, finalColor.x, finalColor.y, finalColor.z);

        // Render Chunks specifically (they need the showGrid boolean)
        if (obj->type == ObjectType::CHUNK) {
            Chunk* chunk = static_cast<Chunk*>(obj);
            chunk->Render(*m_Shader, true); 
        } 
        else {
            // Generic render for non-chunk objects
            obj->Render(*m_Shader); 
        }
    }

    if (m_SelectedObject) {
        // Assuming GetAABB is virtual
        std::pair<glm::vec3, glm::vec3> aabb = m_SelectedObject->GetAABB();
        RenderDebugBox(aabb.first, aabb.second, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    GUI::Render(*this);
    GUI::EndFrame();
    glfwSwapBuffers(m_Window);
}

void Application::InitDebugCube() {
    float vertices[] = {
        // Line loop for bottom face
        0,0,0, 1,0,0,  1,0,0, 1,0,1,  1,0,1, 0,0,1,  0,0,1, 0,0,0,
        // Line loop for top face
        0,1,0, 1,1,0,  1,1,0, 1,1,1,  1,1,1, 0,1,1,  0,1,1, 0,1,0,
        // Vertical connectors
        0,0,0, 0,1,0,  1,0,0, 1,1,0,  1,0,1, 1,1,1,  0,0,1, 0,1,1
    };
    glGenVertexArrays(1, &m_DebugCubeVAO);
    glGenBuffers(1, &m_DebugCubeVBO);
    glBindVertexArray(m_DebugCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_DebugCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Application::RenderDebugBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color) {
    if (m_DebugCubeVAO == 0) InitDebugCube();

    glm::vec3 size = max - min;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, min);
    model = glm::scale(model, size);

    m_Shader->setMat4("model", model);
    int colorLoc = glGetUniformLocation(m_Shader->ID, "uColor");
    glUniform3f(colorLoc, color.x, color.y, color.z);

    glBindVertexArray(m_DebugCubeVAO);
    glDrawArrays(GL_LINES, 0, 24);
}