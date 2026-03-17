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

void TransformGizmo_Renderer::Init() {
    std::vector<float> tVerts; // Translate vertices (Cone)
    std::vector<float> sVerts; // Scale vertices (Cube)
    std::vector<float> rVerts; // Rotate vertices (Torus)
    
    int segments = 12; 
    float shaftRadius = 0.004f;
    float shaftLength = 0.4f;
    float headRadius = 0.030f;
    float headLength = 0.2f;

    const float PI = 3.14159265359f;

    // 1. Generate the shared Shaft (Cylinder)
    for (int i = 0; i < segments; i++) {
        float theta1 = ((float)i / segments) * 2.0f * PI;
        float theta2 = ((float)(i + 1) / segments) * 2.0f * PI;

        float y1 = shaftRadius * std::cos(theta1), z1 = shaftRadius * std::sin(theta1);
        float y2 = shaftRadius * std::cos(theta2), z2 = shaftRadius * std::sin(theta2);

        std::vector<float> quad = {
            0.0f, y1, z1,  shaftLength, y1, z1,  shaftLength, y2, z2,
            0.0f, y1, z1,  shaftLength, y2, z2,  0.0f, y2, z2
        };
        // Add shafts to both models
        tVerts.insert(tVerts.end(), quad.begin(), quad.end());
        sVerts.insert(sVerts.end(), quad.begin(), quad.end());
    }

    // 2. Generate the Cone (For Translate)
    for (int i = 0; i < segments; i++) {
        float t1 = ((float)i / segments) * 2.0f * PI;
        float t2 = ((float)(i + 1) / segments) * 2.0f * PI;
        float y1 = headRadius * std::cos(t1), z1 = headRadius * std::sin(t1);
        float y2 = headRadius * std::cos(t2), z2 = headRadius * std::sin(t2);

        tVerts.insert(tVerts.end(), {
            shaftLength, y1, z1,  
            shaftLength + headLength, 0.0f, 0.0f,  
            shaftLength, y2, z2,
            shaftLength, 0.0f, 0.0f,  
            shaftLength, y1, z1,  
            shaftLength, y2, z2
        });
    }

    // 3. Generate the Cube (For Scale)
    float x1 = shaftLength, x2 = shaftLength + headLength, r = headRadius;
    float box[] = {
        x1,-r, r,  x2,-r, r,  x2, r, r,  x1,-r, r,  x2, r, r,  x1, r, r, // Front
        x1,-r,-r,  x2,-r,-r,  x2, r,-r,  x1,-r,-r,  x2, r,-r,  x1, r,-r, // Back
        x1,-r,-r,  x1,-r, r,  x1, r, r,  x1,-r,-r,  x1, r, r,  x1, r,-r, // Left
        x2,-r,-r,  x2,-r, r,  x2, r, r,  x2,-r,-r,  x2, r, r,  x2, r,-r, // Right
        x1, r,-r,  x2, r,-r,  x2, r, r,  x1, r,-r,  x2, r, r,  x1, r, r, // Top
        x1,-r,-r,  x2,-r,-r,  x2,-r, r,  x1,-r,-r,  x2,-r, r,  x1,-r, r  // Bottom
    };
    sVerts.insert(sVerts.end(), std::begin(box), std::end(box));

    // 4. Generate the Torus (For Rotate)
    int majorSegments = 32;
    int minorSegments = 8;
    float majorRadius = 0.75f;
    float minorRadius = 0.02f;

    for (int i = 0; i < majorSegments; i++) {
        float a0 = ((float)i / majorSegments) * 2.0f * PI;
        float a1 = ((float)(i + 1) / majorSegments) * 2.0f * PI;

        for (int j = 0; j < minorSegments; j++) {
            float b0 = ((float)j / minorSegments) * 2.0f * PI;
            float b1 = ((float)(j + 1) / minorSegments) * 2.0f * PI;

            // Generate flat torus lying on the YZ plane
            auto getTorusPoint = [&](float a, float b) -> glm::vec3 {
                float x = minorRadius * std::cos(b);
                float y = (majorRadius + minorRadius * std::sin(b)) * std::cos(a);
                float z = (majorRadius + minorRadius * std::sin(b)) * std::sin(a);
                return glm::vec3(x, y, z);
            };

            glm::vec3 p00 = getTorusPoint(a0, b0);
            glm::vec3 p10 = getTorusPoint(a1, b0);
            glm::vec3 p11 = getTorusPoint(a1, b1);
            glm::vec3 p01 = getTorusPoint(a0, b1);

            rVerts.insert(rVerts.end(), {
                p00.x, p00.y, p00.z,  p10.x, p10.y, p10.z,  p11.x, p11.y, p11.z,
                p00.x, p00.y, p00.z,  p11.x, p11.y, p11.z,  p01.x, p01.y, p01.z
            });
        }
    }

    // --- UPLOAD TO GPU ---

    // Upload Translate Buffer
    translateVertexCount = tVerts.size() / 3;
    glGenVertexArrays(1, &translateVAO); 
    glGenBuffers(1, &translateVBO);
    glBindVertexArray(translateVAO); 
    glBindBuffer(GL_ARRAY_BUFFER, translateVBO);
    glBufferData(GL_ARRAY_BUFFER, tVerts.size() * sizeof(float), tVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Upload Scale Buffer
    scaleVertexCount = sVerts.size() / 3;
    glGenVertexArrays(1, &scaleVAO); 
    glGenBuffers(1, &scaleVBO);
    glBindVertexArray(scaleVAO); 
    glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
    glBufferData(GL_ARRAY_BUFFER, sVerts.size() * sizeof(float), sVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Upload Rotate Buffer
    rotateVertexCount = rVerts.size() / 3;
    glGenVertexArrays(1, &rotateVAO); 
    glGenBuffers(1, &rotateVBO);
    glBindVertexArray(rotateVAO); 
    glBindBuffer(GL_ARRAY_BUFFER, rotateVBO);
    glBufferData(GL_ARRAY_BUFFER, rVerts.size() * sizeof(float), rVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void TransformGizmo_Renderer::DrawAxis(Shader& shader, const glm::mat4& modelMatrix, const glm::vec3& color, GizmoMode mode) {
    shader.use();
    shader.setMat4("model", modelMatrix); 
    shader.setVec3("color", color);       

    // Select the correct mesh based on the current mode
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

    // 1. Calculate the rotation matrix
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(targetObject->rotation.z), glm::vec3(0, 0, 1));

    // 2. Find the exact center of the rotated box for our pivot
    glm::vec3 pivot = targetObject->position + glm::vec3(rotMat * glm::vec4(targetObject->scale * 0.5f, 1.0f));
    GizmoMode currentMode = gizmo.GetMode();

    float distanceToCamera = glm::length(cameraPos - pivot);
    float dynamicScale = distanceToCamera * 0.25f; 
    float finalGizmoScale = 0.75f * dynamicScale; 

    // 3. Create a base model that translates to pivot, applies object rotation, and scales!
    glm::mat4 baseModel = glm::mat4(1.0f);
    baseModel = glm::translate(baseModel, pivot);
    baseModel = baseModel * rotMat; // Tilt the gizmo to match the object
    baseModel = glm::scale(baseModel, glm::vec3(finalGizmoScale)); 

    // --- X AXIS (Red) ---
    glm::mat4 modelX = baseModel; 
    glm::vec3 colorX = (gizmo.GetHoveredAxis() == GizmoAxis::X || gizmo.GetDraggingAxis() == GizmoAxis::X) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
    DrawAxis(gizmoShader, modelX, colorX, currentMode);

    // --- Y AXIS (Green) ---
    glm::mat4 modelY = glm::rotate(baseModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
    glm::vec3 colorY = (gizmo.GetHoveredAxis() == GizmoAxis::Y || gizmo.GetDraggingAxis() == GizmoAxis::Y) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    DrawAxis(gizmoShader, modelY, colorY, currentMode);

    // --- Z AXIS (Blue) ---
    glm::mat4 modelZ = glm::rotate(baseModel, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
    glm::vec3 colorZ = (gizmo.GetHoveredAxis() == GizmoAxis::Z || gizmo.GetDraggingAxis() == GizmoAxis::Z) 
                       ? glm::vec3(1.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
    DrawAxis(gizmoShader, modelZ, colorZ, currentMode);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); 
}