#include "Application.h"
#include "serializer.h"
#include <iostream>

void Application::Update(float deltaTime) {
    // 1. Physics Integration
    for (Chunk* chunk : m_SceneObjects) {
         chunk->UpdatePhysics(deltaTime);
    }

    // 2. Naive Collision Detection (O(N^2))
    // Note: In a real engine, use a Spatial Hash or Octree for optimization
    for (size_t i = 0; i < m_SceneObjects.size(); i++) {
        for (size_t j = i + 1; j < m_SceneObjects.size(); j++) {
            Chunk* c1 = m_SceneObjects[i];
            Chunk* c2 = m_SceneObjects[j];

            if (c1->CheckCollision(*c2)) {
                c1->ResolveCollision(*c2);
            }
        }
    }
}

void Application::TriggerExplosion(glm::vec3 target) {
    PointExplosion exp;
    exp.center = target;
    exp.radius = m_ExplosionRadius;
    exp.force = m_ExplosionForce;
    exp.falloff = true;
    exp.falloffFactor = 1.0f;

    std::vector<Chunk*> allNewDebris;

    for (Chunk* c : m_SceneObjects) {
        std::vector<Chunk*> debris = c->Explode(exp);
        allNewDebris.insert(allNewDebris.end(), debris.begin(), debris.end());
    }

    // Add generated debris to scene
    for (Chunk* d : allNewDebris) {
        m_SceneObjects.push_back(d);
    }
}

void Application::NewLevel() {
    for (Chunk* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    Chunk* c = new Chunk(16);
    c->GenerateCube();
    c->name = "Default Cube";
    c->physicsBody.isStatic = true;
    m_SceneObjects.push_back(c);
}

void Application::SaveLevel(const std::string& path) {
    if (LevelSerializer::SaveLevel(path, m_SceneObjects)) {
        std::cout << "Level saved successfully to " << path << std::endl;
    } else {
        std::cout << "Failed to save level!" << std::endl;
    }
}

void Application::LoadLevel(const std::string& path) {
    for (Chunk* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    if (LevelSerializer::LoadLevel(path, m_SceneObjects)) {
        std::cout << "Level loaded: " << path << std::endl;
    } else {
        std::cout << "Failed to load level!" << std::endl;
        NewLevel();
    }
}
