#include "TransformGizmo.h"
#include "SceneObject.h" // Update this path to wherever your SceneObject is
#include <algorithm>
#include <cfloat>
#include <gtc/matrix_transform.hpp>
bool TransformGizmo::IntersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& boxMin, const glm::vec3& boxMax, float& t) {
    glm::vec3 invDir = 1.0f / rayDir;
    glm::vec3 t0 = (boxMin - rayOrigin) * invDir;
    glm::vec3 t1 = (boxMax - rayOrigin) * invDir;
    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);
    float tmin_max = std::max(std::max(tmin.x, tmin.y), tmin.z);
    float tmax_min = std::min(std::min(tmax.x, tmax.y), tmax.z);
    if (tmax_min >= tmin_max && tmax_min > 0.0f) {
        t = tmin_max > 0.0f ? tmin_max : tmax_min;
        return true;
    }
    return false;
}

bool TransformGizmo::IntersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t) {
    float denom = glm::dot(planeNormal, rayDir);
    if (std::abs(denom) > 1e-6) {
        glm::vec3 p0l0 = planePoint - rayOrigin;
        t = glm::dot(p0l0, planeNormal) / denom;
        return t >= 0.0f;
    }
    return false;
}

bool TransformGizmo::IntersectRayRing(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& center, const glm::vec3& normal, float innerRadius, float outerRadius, float& t) {
    if (IntersectRayPlane(rayOrigin, rayDir, center, normal, t)) {
        glm::vec3 hitPoint = rayOrigin + rayDir * t;
        float dist = glm::length(hitPoint - center);
        return (dist >= innerRadius && dist <= outerRadius);
    }
    return false;
}

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

    // --- NEW: Local Space Matrix Setup ---
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(selectedObject->rotation.z), glm::vec3(0, 0, 1));
    
    // Find the exact physical center of the chunk
    glm::vec3 pivot = selectedObject->position + glm::vec3(rotMat * glm::vec4(selectedObject->scale * 0.5f, 1.0f));
    
    // Un-rotate the mouse ray so we can do math as if the object was perfectly flat
    glm::mat4 invRot = glm::inverse(rotMat);
    glm::vec3 localRayOrigin = pivot + glm::vec3(invRot * glm::vec4(rayOrigin - pivot, 1.0f));
    glm::vec3 localRayDir = glm::normalize(glm::vec3(invRot * glm::vec4(rayDir, 0.0f)));

    // --- 1. DRAGGING LOGIC ---
    if (isMouseDown && m_DraggingAxis != GizmoAxis::None) {
        glm::vec3 axisVec(0.0f);
        if (m_DraggingAxis == GizmoAxis::X) axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Y) axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Z) axisVec = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 planeNormal = (m_Mode == GizmoMode::Rotate) ? axisVec : glm::normalize(glm::cross(glm::cross(localRayDir, axisVec), axisVec));
        glm::vec3 dragPivot = m_InitialPivot; 
        
        float t = 0.0f;
        // Use the LOCAL ray!
        if (IntersectRayPlane(localRayOrigin, localRayDir, dragPivot, planeNormal, t)) {
            glm::vec3 hitPoint = localRayOrigin + localRayDir * t;
            
            if (m_Mode == GizmoMode::Rotate) {
                glm::vec3 currentVec = glm::normalize(hitPoint - dragPivot);
                float angleRad = std::atan2(glm::dot(glm::cross(m_InitialHitVector, currentVec), planeNormal), 
                                            glm::dot(m_InitialHitVector, currentVec));
                
                glm::vec3 rotDelta(0.0f);
                if (m_DraggingAxis == GizmoAxis::X) rotDelta.x = glm::degrees(angleRad);
                if (m_DraggingAxis == GizmoAxis::Y) rotDelta.y = glm::degrees(angleRad);
                if (m_DraggingAxis == GizmoAxis::Z) rotDelta.z = glm::degrees(angleRad);
                
                selectedObject->rotation = m_InitialRotation + rotDelta;
            } else {
                float currentOffset = glm::dot(hitPoint - dragPivot, axisVec);
                if (m_Mode == GizmoMode::Translate) {
                    // Convert the movement back to world space to move the object
                    glm::vec3 worldAxis = glm::normalize(glm::vec3(rotMat * glm::vec4(axisVec, 0.0f)));
                    selectedObject->position = m_InitialDragPos + worldAxis * (currentOffset - m_DragOffset);
                } else if (m_Mode == GizmoMode::Scale) {
                    selectedObject->scale = m_InitialScale + axisVec * (currentOffset - m_DragOffset);
                }
            }
        }
        return; 
    }

    // --- 2. HOVER DETECT LOGIC ---
    float distanceToCamera = glm::length(rayOrigin - pivot);
    float dynamicScale = distanceToCamera * 0.25f; 

    m_HoveredAxis = GizmoAxis::None;
    float minT = FLT_MAX;
    float tX = FLT_MAX, tY = FLT_MAX, tZ = FLT_MAX;

    if (m_Mode == GizmoMode::Rotate) {
        float outerRadius = 0.85f * dynamicScale;
        float innerRadius = 0.65f * dynamicScale; 
        
        // Use localRayOrigin and localRayDir
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

        // Use localRayOrigin and localRayDir
        bool hitX = IntersectRayAABB(localRayOrigin, localRayDir, xMin, xMax, tX);
        bool hitY = IntersectRayAABB(localRayOrigin, localRayDir, yMin, yMax, tY);
        bool hitZ = IntersectRayAABB(localRayOrigin, localRayDir, zMin, zMax, tZ);

        if (hitX && tX < minT) { minT = tX; m_HoveredAxis = GizmoAxis::X; }
        if (hitY && tY < minT) { minT = tY; m_HoveredAxis = GizmoAxis::Y; }
        if (hitZ && tZ < minT) { minT = tZ; m_HoveredAxis = GizmoAxis::Z; }
    }

    // --- 3. CLICK START LOGIC ---
    if (clickStarted && m_HoveredAxis != GizmoAxis::None) {
        m_DraggingAxis = m_HoveredAxis;
        m_InitialDragPos = selectedObject->position;
        m_InitialScale = selectedObject->scale; 
        m_InitialRotation = selectedObject->rotation; 
        m_InitialPivot = pivot; // Freeze pivot!

        glm::vec3 axisVec(0.0f);
        if (m_DraggingAxis == GizmoAxis::X) axisVec = glm::vec3(1.0f, 0.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Y) axisVec = glm::vec3(0.0f, 1.0f, 0.0f);
        if (m_DraggingAxis == GizmoAxis::Z) axisVec = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 planeNormal = (m_Mode == GizmoMode::Rotate) ? axisVec : glm::normalize(glm::cross(glm::cross(localRayDir, axisVec), axisVec));
        glm::vec3 dragPivot = m_InitialPivot;
        float t = 0.0f;
        
        // Use localRayOrigin and localRayDir
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