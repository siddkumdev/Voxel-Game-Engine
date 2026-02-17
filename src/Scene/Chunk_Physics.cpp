#include "Chunk.h"
#include <gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

std::pair<glm::vec3, glm::vec3> Chunk::GetAABB() const {
    glm::vec3 voxelSize = scale / (float)m_ChunkSize;

    glm::vec3 minV = glm::vec3(boundsMin);
    glm::vec3 maxV = glm::vec3(boundsMax + glm::ivec3(1));

    glm::vec3 localCenter = (minV + maxV) * 0.5f * voxelSize;
    glm::vec3 localExtent = (maxV - minV) * 0.5f * voxelSize;

    // Apply Rotation
    glm::mat4 rot = glm::mat4(1.0f);
    rot = glm::rotate(rot, glm::radians(rotation.x), {1,0,0});
    rot = glm::rotate(rot, glm::radians(rotation.y), {0,1,0});
    rot = glm::rotate(rot, glm::radians(rotation.z), {0,0,1});
    glm::mat3 R = glm::mat3(rot);

    glm::vec3 worldCenter = position + (R * localCenter);
    glm::vec3 newExtent(
        std::abs(R[0][0]) * localExtent.x + std::abs(R[1][0]) * localExtent.y + std::abs(R[2][0]) * localExtent.z,
        std::abs(R[0][1]) * localExtent.x + std::abs(R[1][1]) * localExtent.y + std::abs(R[2][1]) * localExtent.z,
        std::abs(R[0][2]) * localExtent.x + std::abs(R[1][2]) * localExtent.y + std::abs(R[2][2]) * localExtent.z
    );

    return { worldCenter - newExtent, worldCenter + newExtent };
}
