#pragma once
#include "type.h"
#include "Chunk.h"
#include <string>
#include <vector>

class PointExplosion : public SceneObject {
public:
    ExplosionData data;
    bool visible = true; // For drawing a debug wireframe if needed

    PointExplosion() {
        data.center = glm::vec3(0.0f);
        data.radius = 5.0f;
        data.force = 50.0f;
        data.falloff = true;
        data.falloffFactor = 1.0f;

        type = ObjectType::EXPLOSION;
    }

    // Trigger the explosion on a list of chunks
    void Detonate(std::vector<Chunk*>& chunks, std::vector<Chunk*>& outDebris) {
        // Sync center with position
        data.center = position;

        for (Chunk* c : chunks) {
            if (!c) continue;
            std::vector<Chunk*> newDebris = c->Explode(data);
            outDebris.insert(outDebris.end(), newDebris.begin(), newDebris.end());
        }
    }
};
