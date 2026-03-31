#include "TransformGizmo_Renderer.h"
#include "glad/glad.h" 
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

#include "../Scene/SceneObject.h" 
#include "TransformGizmo.h"      

TransformGizmo_Renderer::TransformGizmo_Renderer() : translateVAO(0), translateVBO(0), scaleVAO(0), scaleVBO(0), rotateVAO(0), rotateVBO(0) {
}

TransformGizmo_Renderer::~TransformGizmo_Renderer() {
    if (translateVAO != 0) {
        glDeleteVertexArrays(1, &translateVAO);
        glDeleteBuffers(1, &translateVBO);
    }
    if (scaleVAO != 0) {
        glDeleteVertexArrays(1, &scaleVAO);
        glDeleteBuffers(1, &scaleVBO);
    }
    if (rotateVAO != 0) {
        glDeleteVertexArrays(1, &rotateVAO);
        glDeleteBuffers(1, &rotateVBO);
    }
}

void TransformGizmo_Renderer::DrawAxis(Shader& shader, const glm::mat4& modelMatrix, const glm::vec3& color, GizmoMode mode) {
    shader.use();
    shader.setMat4("model", modelMatrix); 
    shader.setVec3("color", color);       

    if (mode == GizmoMode::Translate) {
        glBindVertexArray(translateVAO);
        glDrawArrays(GL_TRIANGLES, 0, translateVertexCount); 
    } else if (mode == GizmoMode::Scale) {
        glBindVertexArray(scaleVAO);
        glDrawArrays(GL_TRIANGLES, 0, scaleVertexCount); 
    } else if (mode == GizmoMode::Rotate) {
        glBindVertexArray(rotateVAO);
        glDrawArrays(GL_TRIANGLES, 0, rotateVertexCount); 
    }

    glBindVertexArray(0);
}

void TransformGizmo_Renderer::Draw(const SceneObject* targetObject, const TransformGizmo& gizmo, Shader& gizmoShader, const glm::vec3& cameraPos) {
    if (!targetObject || !targetObject->isSelected) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); 

    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.z), glm::vec3(0, 0, 1));

    glm::vec3 pivot = targetObject->position + glm::vec3(rotMat * glm::vec4(targetObject->scale * 0.5f, 1.0f));
    GizmoMode currentMode = gizmo.GetMode();

    float distanceToCamera = glm::length(cameraPos - pivot);
    float dynamicScale = distanceToCamera * 0.25f; 
    float finalGizmoScale = 0.75f * dynamicScale; 

    glm::mat4 baseModel = glm::mat4(1.0f);
    baseModel = glm::translate(baseModel, pivot);
    baseModel = baseModel * rotMat;
    baseModel = glm::scale(baseModel, glm::vec3(finalGizmoScale)); 

    glm::mat4 modelX = baseModel; 
    glm::vec3 colorX = (gizmo.GetHoveredAxis() == GizmoAxis::X || gizmo.GetDraggingAxis() == GizmoAxis::X) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    DrawAxis(gizmoShader, modelX, colorX, currentMode);

    glm::mat4 modelY = glm::rotate(baseModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
    glm::vec3 colorY = (gizmo.GetHoveredAxis() == GizmoAxis::Y || gizmo.GetDraggingAxis() == GizmoAxis::Y) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    DrawAxis(gizmoShader, modelY, colorY, currentMode);

    glm::mat4 modelZ = glm::rotate(baseModel, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
    glm::vec3 colorZ = (gizmo.GetHoveredAxis() == GizmoAxis::Z || gizmo.GetDraggingAxis() == GizmoAxis::Z) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
    DrawAxis(gizmoShader, modelZ, colorZ, currentMode);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); 
}
