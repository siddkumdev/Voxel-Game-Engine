#pragma once
#include "type.h"
#include "SceneObject.h"
#include "Chunk.h"
#include <string>
#include <vector>

class PointExplosion : public SceneObject {
public:
    ExplosionData data;
    bool visible = true; // Use this to toggle the debug wireframe on/off

    PointExplosion() {
        data.center = glm::vec3(0.0f);
        data.radius = 5.0f;
        data.force = 50.0f;
        data.falloff = true;
        data.falloffFactor = 1.0f;

        type = ObjectType::EXPLOSION;
    }

    // Trigger the explosion on a list of chunks
    void Detonate(std::vector<Chunk*>& chunks, std::vector<Chunk*>& outDebris);

    // Override the base class Render method
    void Render(Shader& shader) override;

private:
    // Cache for the wireframe mesh
    unsigned int gizmoVAO = 0;
    unsigned int gizmoVBO = 0;
    int gizmoVertexCount = 0;
    void GenerateSphereGizmo();

    std::vector<Chunk*> ExplodeChunk(Chunk* chunk);
};