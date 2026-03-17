#pragma once
#include <glm.hpp>

// Forward declare SceneObject so we can modify it
class SceneObject;

enum class GizmoMode { Translate, Scale, Rotate };
enum class GizmoAxis { None, X, Y, Z };

class TransformGizmo {
public:
    TransformGizmo() = default;

    // Mode switching
    GizmoMode GetMode() const { return m_Mode; }
    void SetMode(GizmoMode mode) { m_Mode = mode; }

    // State reading for the Renderer
    GizmoAxis GetHoveredAxis() const { return m_HoveredAxis; }
    GizmoAxis GetDraggingAxis() const { return m_DraggingAxis; }

    // The main math loop
    void HandleInteraction(const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool isMouseDown, SceneObject* selectedObject);

private:
    GizmoMode m_Mode = GizmoMode::Translate;
    GizmoAxis m_HoveredAxis = GizmoAxis::None;
    GizmoAxis m_DraggingAxis = GizmoAxis::None;

    glm::vec3 m_InitialDragPos = glm::vec3(0.0f);
    glm::vec3 m_InitialScale = glm::vec3(1.0f);
    glm::vec3 m_InitialRotation = glm::vec3(0.0f);
    glm::vec3 m_InitialHitVector = glm::vec3(0.0f);
    glm::vec3 m_InitialPivot = glm::vec3(0.0f); // <--- ADD THIS LINE
    float m_DragOffset = 0.0f;
    bool m_WasMouseDown = false;

    // Helper math 
    bool IntersectRayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& boxMin, const glm::vec3& boxMax, float& t);
    bool IntersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t);
    bool IntersectRayRing(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& center, const glm::vec3& normal, float innerRadius, float outerRadius, float& t);
};