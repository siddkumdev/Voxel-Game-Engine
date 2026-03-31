#include "Explosion.h"
#include <algorithm>
#include <map>
#include <iostream>
#include <gtc/random.hpp>
#include <cmath>
#include <glad/glad.h>
#include <vector>
#include <gtc/matrix_transform.hpp>
struct Shard {
    std::vector<glm::ivec3> voxels;
    glm::ivec3 minB = glm::ivec3(10000);
    glm::ivec3 maxB = glm::ivec3(-10000);
    glm::vec3 centerAcc = glm::vec3(0.0f);
};

void PointExplosion::Detonate(std::vector<Chunk*>& chunks, std::vector<Chunk*>& outDebris) {

    data.center = position;

    for (Chunk* c : chunks) {
        if (!c) continue;
        std::vector<Chunk*> newDebris = ExplodeChunk(c);
        outDebris.insert(outDebris.end(), newDebris.begin(), newDebris.end());
    }
}

std::vector<Chunk*> PointExplosion::ExplodeChunk(Chunk* chunk) {
    std::vector<Chunk*> debris;

    if (data.force <= 0.0f || chunk->layer == PhysicsLayer::DEBRIS) return debris;

    float voxelSize = chunk->scale.x / (float)chunk->GetResolution();
    glm::vec3 localCenter = (data.center - chunk->position) / voxelSize;
    float localRadius = data.radius / voxelSize;

    int numSeeds = std::clamp((int)(data.force / (chunk->resistance + 0.01f) * 2.0f), 2, 50);
    std::vector<glm::vec3> seeds;

    int chunkSize = chunk->GetResolution();

    for (int i = 0; i < numSeeds; i++) {

        glm::vec3 p = glm::ballRand(localRadius) + localCenter;
        p = glm::clamp(p, glm::vec3(0), glm::vec3(chunkSize-1));
        seeds.push_back(p);
    }

    std::map<int, Shard> shards;
    bool modified = false;

    int r = (int)std::ceil(localRadius);
    glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(localCenter) - r);
    glm::ivec3 max = glm::min(glm::ivec3(chunkSize-1), glm::ivec3(localCenter) + r);

    for (int x = min.x; x <= max.x; x++) {
        for (int y = min.y; y <= max.y; y++) {
            for (int z = min.z; z <= max.z; z++) {
                if (!chunk->IsActive(x, y, z)) continue;
                if (glm::distance(glm::vec3(x, y, z), localCenter) > localRadius) continue;

                chunk->SetBlock(x, y, z, false);
                modified = true;

                int best = 0; float minD = 1e9;
                for (int i=0; i<seeds.size(); i++) {
                    float d = glm::distance(glm::vec3(x, y, z), seeds[i]);
                    if (d < minD) { minD = d; best = i; }
                }

                Shard& s = shards[best];
                s.voxels.push_back({x,y,z});
                s.minB = glm::min(s.minB, {x,y,z});
                s.maxB = glm::max(s.maxB, {x,y,z});
                s.centerAcc += glm::vec3(x,y,z);
            }
        }
    }

    for (auto& [id, s] : shards) {
        if (s.voxels.empty()) continue;

        glm::ivec3 size = s.maxB - s.minB + 1;
        int dim = std::max({size.x, size.y, size.z}) + 2;

        Chunk* d = new Chunk(dim);
        d->name = "Shard";
        d->layer = PhysicsLayer::DEBRIS;
        d->useGravity = true;

        d->scale = glm::vec3(dim) * voxelSize;
        glm::vec3 offset = glm::vec3(s.minB - 1);
        d->position = chunk->position + (offset * voxelSize);

        for (auto& v : s.voxels) {
            d->SetBlock(v.x - s.minB.x + 1, v.y - s.minB.y + 1, v.z - s.minB.z + 1, true);
        }

        d->mass = s.voxels.size() * 0.1f;

        glm::vec3 center = (s.centerAcc / (float)s.voxels.size()) * voxelSize + chunk->position;
        glm::vec3 dir = glm::normalize(center - data.center);
        if (glm::length(dir) < 0.001f) dir = {0,1,0};

        d->velocity = dir * std::min((data.force / d->mass) * 5.0f, 50.0f);
        d->rotation = glm::vec3(rand()%360, rand()%360, rand()%360);

        d->RecalculateBounds();
        d->UpdateMesh();
        debris.push_back(d);
    }

    if (modified) {
        chunk->RecalculateBounds();
        chunk->UpdateMesh();
    }
    return debris;
}

void PointExplosion::GenerateSphereGizmo() {
    std::vector<glm::vec3> vertices;
    int segments = 32; 

    for (int i = 0; i < segments; i++) {
        float theta1 = ((float)i / segments) * 2.0f * glm::pi<float>();
        float theta2 = ((float)(i + 1) / segments) * 2.0f * glm::pi<float>();

        vertices.push_back(glm::vec3(cos(theta1), sin(theta1), 0.0f));
        vertices.push_back(glm::vec3(cos(theta2), sin(theta2), 0.0f));

        vertices.push_back(glm::vec3(cos(theta1), 0.0f, sin(theta1)));
        vertices.push_back(glm::vec3(cos(theta2), 0.0f, sin(theta2)));

        vertices.push_back(glm::vec3(0.0f, cos(theta1), sin(theta1)));
        vertices.push_back(glm::vec3(0.0f, cos(theta2), sin(theta2)));
    }

    gizmoVertexCount = vertices.size();

    glGenVertexArrays(1, &gizmoVAO);
    glGenBuffers(1, &gizmoVBO);

    glBindVertexArray(gizmoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void PointExplosion::Render(Shader& shader) {

    if (!visible) return;

    if (gizmoVAO == 0) {
        GenerateSphereGizmo();
    }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position); 
    model = glm::scale(model, glm::vec3(data.radius));

    shader.setMat4("model", model);

    int colorLoc = glGetUniformLocation(shader.ID, "uColor");
    if (colorLoc != -1) {
        glUniform3f(colorLoc, 0.2f, 1.0f, 0.2f); 
    }

    glBindVertexArray(gizmoVAO);
    glDrawArrays(GL_LINES, 0, gizmoVertexCount);
    glBindVertexArray(0);
}
