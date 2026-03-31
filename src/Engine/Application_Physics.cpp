#include "Application.h"
#include "serializer.h"
#include "Explosion.h"
#include <iostream>

void Application::Update(float deltaTime) {

    for (SceneObject* obj : m_SceneObjects) {
         obj->UpdatePhysics(deltaTime);
    }

    for (size_t i = 0; i < m_SceneObjects.size(); i++) {
        for (size_t j = i + 1; j < m_SceneObjects.size(); j++) {
            SceneObject* o1 = m_SceneObjects[i];
            SceneObject* o2 = m_SceneObjects[j];

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

    PointExplosion explosion;
    explosion.data.center = target;
    explosion.data.radius = m_ExplosionRadius;
    explosion.data.force = m_ExplosionForce;
    explosion.data.falloff = true;
    explosion.data.falloffFactor = 1.0f;
    explosion.position = target;

    std::vector<Chunk*> allNewDebris;
    std::vector<Chunk*> targets;

    for (SceneObject* obj : m_SceneObjects) {
        if (obj->type == ObjectType::CHUNK) {
            targets.push_back(static_cast<Chunk*>(obj));
        }
    }

    explosion.Detonate(targets, allNewDebris);

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

    c->type = ObjectType::CHUNK; 
    m_SceneObjects.push_back(c);
}

void Application::SaveLevel(const std::string& path) {

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

    std::vector<Chunk*> loadedChunks;
    if (LevelSerializer::LoadLevel(path, loadedChunks)) {
        std::cout << "Level loaded: " << path << std::endl;

        for (Chunk* c : loadedChunks) {
            c->type = ObjectType::CHUNK;
            m_SceneObjects.push_back(c);
        }
    } else {
        std::cout << "Failed to load level!" << std::endl;
        NewLevel();
    }
}
