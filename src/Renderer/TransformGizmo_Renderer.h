#pragma once
#include "SceneObject.h"
#include "shader.h"
#include "TransformGizmo.h"

class TransformGizmo_Renderer {
public:
    TransformGizmo_Renderer();
    ~TransformGizmo_Renderer();

    void Init(); 

    void Draw(const SceneObject* targetObject, const TransformGizmo& gizmo, Shader& gizmoShader, const glm::vec3& cameraPos);

private:

    unsigned int translateVAO, translateVBO;
    int translateVertexCount = 0; 

    unsigned int scaleVAO, scaleVBO;
    int scaleVertexCount = 0; 

    unsigned int rotateVAO, rotateVBO;
    int rotateVertexCount = 0;

    void DrawAxis(Shader& shader, const glm::mat4& modelMatrix, const glm::vec3& color, GizmoMode mode);
};
