#include "TransformGizmo_Renderer.h"
#include "glad/glad.h"
#include <vector>
#include <cmath>

void TransformGizmo_Renderer::Init() {
    std::vector<float> tVerts;
    std::vector<float> sVerts;
    std::vector<float> rVerts;

    int segments = 12;
    float shaftRadius = 0.004f;
    float shaftLength = 0.4f;
    float headRadius = 0.030f;
    float headLength = 0.2f;

    const float PI = 3.14159265359f;

    for (int i = 0; i < segments; i++) {
        float theta1 = ((float)i / segments) * 2.0f * PI;
        float theta2 = ((float)(i + 1) / segments) * 2.0f * PI;

        float y1 = shaftRadius * std::cos(theta1), z1 = shaftRadius * std::sin(theta1);
        float y2 = shaftRadius * std::cos(theta2), z2 = shaftRadius * std::sin(theta2);

        std::vector<float> quad = {
            0.0f, y1, z1,  shaftLength, y1, z1,  shaftLength, y2, z2,
            0.0f, y1, z1,  shaftLength, y2, z2,  0.0f, y2, z2
        };

        tVerts.insert(tVerts.end(), quad.begin(), quad.end());
        sVerts.insert(sVerts.end(), quad.begin(), quad.end());
    }

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

    float x1 = shaftLength, x2 = shaftLength + headLength, r = headRadius;
    float box[] = {
        x1,-r, r,  x2,-r, r,  x2, r, r,  x1,-r, r,  x2, r, r,  x1, r, r,
        x1,-r,-r,  x2,-r,-r,  x2, r,-r,  x1,-r,-r,  x2, r,-r,  x1, r,-r,
        x1,-r,-r,  x1,-r, r,  x1, r, r,  x1,-r,-r,  x1, r, r,  x1, r,-r,
        x2,-r,-r,  x2,-r, r,  x2, r, r,  x2,-r,-r,  x2, r, r,  x2, r,-r,
        x1, r,-r,  x2, r,-r,  x2, r, r,  x1, r,-r,  x2, r, r,  x1, r, r,
        x1,-r,-r,  x2,-r,-r,  x2,-r, r,  x1,-r,-r,  x2,-r, r,  x1,-r, r
    };
    sVerts.insert(sVerts.end(), std::begin(box), std::end(box));

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

    translateVertexCount = tVerts.size() / 3;
    glGenVertexArrays(1, &translateVAO);
    glGenBuffers(1, &translateVBO);
    glBindVertexArray(translateVAO);
    glBindBuffer(GL_ARRAY_BUFFER, translateVBO);
    glBufferData(GL_ARRAY_BUFFER, tVerts.size() * sizeof(float), tVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    scaleVertexCount = sVerts.size() / 3;
    glGenVertexArrays(1, &scaleVAO);
    glGenBuffers(1, &scaleVBO);
    glBindVertexArray(scaleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
    glBufferData(GL_ARRAY_BUFFER, sVerts.size() * sizeof(float), sVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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