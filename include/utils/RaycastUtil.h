#pragma once

#include <glm/glm.hpp>
#include "world/Block.h"

class World;

class RaycastUtil {
public:
    struct RaycastResult {
        bool hit = false;
        glm::ivec3 blockPos;
        glm::vec3 hitPoint;
        glm::ivec3 normal;
        BlockType blockType;
    };

    static RaycastResult raycast(const glm::vec3& rayStart,
                                 const glm::vec3& rayDirection,
                                 const World& world,
                                 float maxDistance);
};