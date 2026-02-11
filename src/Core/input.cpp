#include "core.h"

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnMouse(xposIn, yposIn);
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnScroll(xoffset, yoffset);
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app) app->OnMouseButton(button, action, mods);
}

void Application::ProcessInput() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!m_RightClickHolding) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_FirstMouse = true;
            m_RightClickHolding = true;
        }
        bool sprint = (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) m_Camera.ProcessKeyboard(FORWARD, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) m_Camera.ProcessKeyboard(BACKWARD, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) m_Camera.ProcessKeyboard(LEFT, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) m_Camera.ProcessKeyboard(RIGHT, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS) m_Camera.ProcessKeyboard(UP, m_DeltaTime, sprint);
        if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS) m_Camera.ProcessKeyboard(DOWN, m_DeltaTime, sprint);
    } else {
        if (m_RightClickHolding) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_RightClickHolding = false;
        }
    }
}

void Application::HandleSelection(float mouseX, float mouseY, int screenW, int screenH) {
    // 1. Ray Calculation (Same as before)
    float x = (2.0f * mouseX) / screenW - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenH;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 100.0f);
    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::mat4 view = m_Camera.GetViewMatrix();
    glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

    Ray ray;
    ray.origin = m_Camera.Position;
    ray.direction = ray_wor;

    // 2. Raycast against Scene
    float closestDist = 10000.0f;
    Chunk* hitObject = nullptr;

    for (Chunk* chunk : m_SceneObjects) {
        std::pair<glm::vec3, glm::vec3> aabb = chunk->GetAABB();
        float dist;
        if (IntersectRayAABB(ray, aabb.first, aabb.second, dist)) {
            if (dist < closestDist) {
                closestDist = dist;
                hitObject = chunk;
            }
        }
    }

    // 3. Handle Result based on Mode
    if (hitObject) {
        if (m_ExplosionMode) {
            // Calculate exact hit point
            glm::vec3 hitPoint = ray.origin + (ray.direction * closestDist);
            TriggerExplosion(hitPoint);
        } else {
            // Normal Select Mode
            m_SelectedObject = hitObject;
        }
    } else {
        if (!m_ExplosionMode) m_SelectedObject = nullptr;
    }
}
void Application::OnMouse(double xposIn, double yposIn) {
    if (!m_RightClickHolding) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (m_FirstMouse) { m_LastX = xpos; m_LastY = ypos; m_FirstMouse = false; }
    float xoffset = xpos - m_LastX;
    float yoffset = m_LastY - ypos;
    m_LastX = xpos; m_LastY = ypos;
    m_Camera.ProcessMouseMovement(xoffset, yoffset);
}

void Application::OnScroll(double xoffset, double yoffset) {
    m_Camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Application::OnMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        m_MouseClicked = true;
    }
}
