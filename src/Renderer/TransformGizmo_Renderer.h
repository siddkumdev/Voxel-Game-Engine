#pragma once
#include "SceneObject.h"
#include "shader.h"
#include "TransformGizmo.h" // This file now provides the GizmoMode and GizmoAxis enums

class TransformGizmo_Renderer {
public:
    TransformGizmo_Renderer();
    ~TransformGizmo_Renderer();

    // Initializes the VAOs and VBOs for the 3D meshes (Cone and Cube)
    void Init(); 

    // Reads the SceneObject's position and state to draw the gizmo dynamically
    void Draw(const SceneObject* targetObject, const TransformGizmo& gizmo, Shader& gizmoShader, const glm::vec3& cameraPos);

private:
    // Buffers for the Translation (Cone) mesh
    unsigned int translateVAO, translateVBO;
    int translateVertexCount = 0; 
    
    // Buffers for the Scale (Cube) mesh
    unsigned int scaleVAO, scaleVBO;
    int scaleVertexCount = 0; 

    // Buffers for the Rotation (Torus) mesh
    unsigned int rotateVAO, rotateVBO;
    int rotateVertexCount = 0;

    // Helper function to draw the specific mesh
    void DrawAxis(Shader& shader, const glm::mat4& modelMatrix, const glm::vec3& color, GizmoMode mode);
};