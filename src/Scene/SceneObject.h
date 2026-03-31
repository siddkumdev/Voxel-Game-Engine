#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <string>
#include <glm.hpp>
#include <utility>
#include "type.h"

class Shader;

class SceneObject {
public:
    ObjectType type;
    std::string name;

    glm::vec3 position     = glm::vec3(0.0f);
    glm::vec3 rotation     = glm::vec3(0.0f);
    glm::vec3 scale        = glm::vec3(1.0f);
    glm::vec3 color        = glm::vec3(1.0f);

    bool isSelected           = false;

    virtual ~SceneObject() = default;

    virtual void UpdatePhysics(float dt) {}
    virtual void Render(Shader& shader) {}

    virtual std::pair<glm::vec3, glm::vec3> GetAABB() const {
        return { position, position + scale };
    }
};

#endif
