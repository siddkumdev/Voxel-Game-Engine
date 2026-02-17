#include "Application.h"
#include "serializer.h"
#include "Explosion.h"
#include <iostream>

void Application::Update(float deltaTime) {
    // 1. Physics Integration
    // Works generically for all SceneObjects due to virtual UpdatePhysics
    for (SceneObject* obj : m_SceneObjects) {
         obj->UpdatePhysics(deltaTime);
    }

    // 2. Naive Collision Detection (O(N^2))
    // Note: We only check collisions between Chunks for now
    for (size_t i = 0; i < m_SceneObjects.size(); i++) {
        for (size_t j = i + 1; j < m_SceneObjects.size(); j++) {
            SceneObject* o1 = m_SceneObjects[i];
            SceneObject* o2 = m_SceneObjects[j];

            // Only Chunks have Voxel Collision logic
            if (o1->type == ObjectType::CHUNK && o2->type == ObjectType::CHUNK) {
                Chunk* c1 = static_cast<Chunk*>(o1);
                Chunk* c2 = static_cast<Chunk*>(o2);

                if (c1->CheckCollision(*c2)) {
                    c1->ResolveCollision(*c2);
                }
            }
        }
    }
}

void Application::TriggerExplosion(glm::vec3 target) {
    // Create a temporary PointExplosion object to handle logic
    PointExplosion explosion;
    explosion.data.center = target;
    explosion.data.radius = m_ExplosionRadius;
    explosion.data.force = m_ExplosionForce;
    explosion.data.falloff = true;
    explosion.data.falloffFactor = 1.0f;
    explosion.position = target; // Set transform position as well

    std::vector<Chunk*> allNewDebris;
    std::vector<Chunk*> targets;

    for (SceneObject* obj : m_SceneObjects) {
        if (obj->type == ObjectType::CHUNK) {
            targets.push_back(static_cast<Chunk*>(obj));
        }
    }

    explosion.Detonate(targets, allNewDebris);

    // Add generated debris to scene
    for (Chunk* d : allNewDebris) {
        m_SceneObjects.push_back(d);
    }
}

void Application::NewLevel() {
    for (SceneObject* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    Chunk* c = new Chunk(16);
    c->GenerateCube();
    c->name = "Default Cube";
    c->isStatic = true;
    // Set type explicitly if not done in constructor
    c->type = ObjectType::CHUNK; 
    m_SceneObjects.push_back(c);
}

void Application::SaveLevel(const std::string& path) {
    // Filter out only Chunks for saving
    std::vector<Chunk*> chunksToSave;
    for (SceneObject* obj : m_SceneObjects) {
        if (obj->type == ObjectType::CHUNK) {
            chunksToSave.push_back(static_cast<Chunk*>(obj));
        }
    }

    if (LevelSerializer::SaveLevel(path, chunksToSave)) {
        std::cout << "Level saved successfully to " << path << std::endl;
    } else {
        std::cout << "Failed to save level!" << std::endl;
    }
}

void Application::LoadLevel(const std::string& path) {
    for (SceneObject* c : m_SceneObjects) delete c;
    m_SceneObjects.clear();
    m_SelectedObject = nullptr;

    // Load generic chunks
    std::vector<Chunk*> loadedChunks;
    if (LevelSerializer::LoadLevel(path, loadedChunks)) {
        std::cout << "Level loaded: " << path << std::endl;
        // Move them into the main SceneObject list
        for (Chunk* c : loadedChunks) {
            c->type = ObjectType::CHUNK; // Ensure type is set
            m_SceneObjects.push_back(c);
        }
    } else {
        std::cout << "Failed to load level!" << std::endl;
        NewLevel();
    }
}