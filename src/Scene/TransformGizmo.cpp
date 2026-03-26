#include "TransformGizmo.h"
#include "SceneObject.h"
#include <algorithm>
#include <cfloat>
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>

void TransformGizmo::HandleInteraction(const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool isMouseDown, SceneObject* selectedObject) {
    if (!selectedObject) {
        m_HoveredAxis = GizmoAxis::None;
        m_DraggingAxis = GizmoAxis::None;
        m_WasMouseDown = false;
        return;
    }

    bool clickStarted = (isMouseDown && !m_WasMouseDown);
    bool clickEnded = (!isMouseDown && m_WasMouseDown);
    m_WasMouseDown = isMouseDown;

    if (clickEnded) m_DraggingAxis = GizmoAxis::None;

    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.z), glm::vec3(0, 0, 1));

    glm::vec3 pivot = selectedObject->position + glm::vec3(rotMat * glm::vec4(selectedObject->scale * 0.5f, 1.0f));

    glm::mat4 invRot = glm::inverse(rotMat);
    glm::vec3 localRayOrigin = pivot + glm::vec3(invRot * glm::vec4(rayOrigin - pivot, 1.0f));
    glm::vec3 localRayDir = glm::normalize(glm::vec3(invRot * glm::vec4(rayDir, 0.0f)));

    if (isMouseDown && m_DraggingAxis != GizmoAxis::None) {
        glm::vec3 axisVec(0.0f);
        if (m_DraggingAxis == GizmoAxis::X) axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Y) axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Z) axisVec = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 planeNormal = (m_Mode == GizmoMode::Rotate) ? axisVec : glm::normalize(glm::cross(glm::cross(localRayDir, axisVec), axisVec));
        glm::vec3 dragPivot = m_InitialPivot; 

        float t = 0.0f;

        if (IntersectRayPlane(localRayOrigin, localRayDir, dragPivot, planeNormal, t)) {
            glm::vec3 hitPoint = localRayOrigin + localRayDir * t;

            if (m_Mode == GizmoMode::Rotate) {
                glm::vec3 currentVec = glm::normalize(hitPoint - dragPivot);
                float angleRad = std::atan2(glm::dot(glm::cross(m_InitialHitVector, currentVec), planeNormal), 
                                            glm::dot(m_InitialHitVector, currentVec));

                glm::mat4 initialRotMat = glm::mat4(1.0f);
                initialRotMat = glm::rotate(initialRotMat, glm::radians(m_InitialRotation.x), glm::vec3(1, 0, 0));
                initialRotMat = glm::rotate(initialRotMat, glm::radians(m_InitialRotation.y), glm::vec3(0, 1, 0));
                initialRotMat = glm::rotate(initialRotMat, glm::radians(m_InitialRotation.z), glm::vec3(0, 0, 1));

                glm::mat4 deltaRot = glm::mat4(1.0f);
                if (m_DraggingAxis == GizmoAxis::X) deltaRot = glm::rotate(deltaRot, angleRad, glm::vec3(1, 0, 0));
                if (m_DraggingAxis == GizmoAxis::Y) deltaRot = glm::rotate(deltaRot, angleRad, glm::vec3(0, 1, 0));
                if (m_DraggingAxis == GizmoAxis::Z) deltaRot = glm::rotate(deltaRot, angleRad, glm::vec3(0, 0, 1));

                glm::mat4 newRotMat = initialRotMat * deltaRot;

                glm::vec3 euler;
                glm::extractEulerAngleXYZ(newRotMat, euler.x, euler.y, euler.z);

                selectedObject->rotation = glm::degrees(euler);
            } else {
                float currentOffset = glm::dot(hitPoint - dragPivot, axisVec);
                if (m_Mode == GizmoMode::Translate) {

                    glm::vec3 worldAxis = glm::normalize(glm::vec3(rotMat * glm::vec4(axisVec, 0.0f)));
                    selectedObject->position = m_InitialDragPos + worldAxis * (currentOffset - m_DragOffset);
                } else if (m_Mode == GizmoMode::Scale) {
                    selectedObject->scale = m_InitialScale + axisVec * (currentOffset - m_DragOffset);
                }
            }
        }
        return; 
    }

    float distanceToCamera = glm::length(rayOrigin - pivot);
    float dynamicScale = distanceToCamera * 0.25f; 

    m_HoveredAxis = GizmoAxis::None;
    float minT = FLT_MAX;
    float tX = FLT_MAX, tY = FLT_MAX, tZ = FLT_MAX;

    if (m_Mode == GizmoMode::Rotate) {
        float outerRadius = 0.85f * dynamicScale;
        float innerRadius = 0.65f * dynamicScale; 

        bool hitX = IntersectRayRing(localRayOrigin, localRayDir, pivot, glm::vec3(1,0,0), innerRadius, outerRadius, tX);
        bool hitY = IntersectRayRing(localRayOrigin, localRayDir, pivot, glm::vec3(0,1,0), innerRadius, outerRadius, tY);
        bool hitZ = IntersectRayRing(localRayOrigin, localRayDir, pivot, glm::vec3(0,0,1), innerRadius, outerRadius, tZ);

        if (hitX && tX < minT) { minT = tX; m_HoveredAxis = GizmoAxis::X; }
        if (hitY && tY < minT) { minT = tY; m_HoveredAxis = GizmoAxis::Y; }
        if (hitZ && tZ < minT) { minT = tZ; m_HoveredAxis = GizmoAxis::Z; }
    } else {
        float gizmoScale = 0.75f * dynamicScale; 
        float thickness = 0.15f * dynamicScale;  

        glm::vec3 xMin = pivot + glm::vec3(0.0f, -thickness/2, -thickness/2);
        glm::vec3 xMax = pivot + glm::vec3(gizmoScale, thickness/2, thickness/2);

        glm::vec3 yMin = pivot + glm::vec3(-thickness/2, 0.0f, -thickness/2);
        glm::vec3 yMax = pivot + glm::vec3(thickness/2, gizmoScale, thickness/2);

        glm::vec3 zMin = pivot + glm::vec3(-thickness/2, -thickness/2, 0.0f);
        glm::vec3 zMax = pivot + glm::vec3(thickness/2, thickness/2, gizmoScale);

        bool hitX = IntersectRayAABB(localRayOrigin, localRayDir, xMin, xMax, tX);
        bool hitY = IntersectRayAABB(localRayOrigin, localRayDir, yMin, yMax, tY);
        bool hitZ = IntersectRayAABB(localRayOrigin, localRayDir, zMin, zMax, tZ);

        if (hitX && tX < minT) { minT = tX; m_HoveredAxis = GizmoAxis::X; }
        if (hitY && tY < minT) { minT = tY; m_HoveredAxis = GizmoAxis::Y; }
        if (hitZ && tZ < minT) { minT = tZ; m_HoveredAxis = GizmoAxis::Z; }
    }

    if (clickStarted && m_HoveredAxis != GizmoAxis::None) {
        m_DraggingAxis = m_HoveredAxis;
        m_InitialDragPos = selectedObject->position;
        m_InitialScale = selectedObject->scale; 
        m_InitialRotation = selectedObject->rotation; 
        m_InitialPivot = pivot;

        glm::vec3 axisVec(0.0f);
        if (m_DraggingAxis == GizmoAxis::X) axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Y) axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Z) axisVec = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 planeNormal = (m_Mode == GizmoMode::Rotate) ? axisVec : glm::normalize(glm::cross(glm::cross(localRayDir, axisVec), axisVec));
        glm::vec3 dragPivot = m_InitialPivot;
        float t = 0.0f;

        if (IntersectRayPlane(localRayOrigin, localRayDir, dragPivot, planeNormal, t)) {
            glm::vec3 hitPoint = localRayOrigin + localRayDir * t;
            if (m_Mode == GizmoMode::Rotate) {
                m_InitialHitVector = glm::normalize(hitPoint - dragPivot); 
            } else {
                m_DragOffset = glm::dot(hitPoint - dragPivot, axisVec); 
            }
        }
    }
}
