#pragma once
#include "type.h"
#include "SceneObject.h"
#include "Chunk.h"
#include <string>
#include <vector>

class PointExplosion : public SceneObject {
public:
    ExplosionData data;
    bool visible = true;

    PointExplosion() {
        data.center = glm::vec3(0.0f);
        data.radius = 5.0f;
        data.force = 50.0f;
        data.falloff = true;
        data.falloffFactor = 1.0f;

        type = ObjectType::EXPLOSION;
    }

    void Detonate(std::vector<Chunk*>& chunks, std::vector<Chunk*>& outDebris);

    void Render(Shader& shader) override;

private:

    unsigned int gizmoVAO = 0;
    unsigned int gizmoVBO = 0;
    int gizmoVertexCount = 0;
    void GenerateSphereGizmo();

    std::vector<Chunk*> ExplodeChunk(Chunk* chunk);
};
