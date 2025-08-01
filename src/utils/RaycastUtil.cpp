#include "utils/RaycastUtil.h"
#include "world/World.h"
#include "world/Chunk.h"
#include <cmath>

RaycastUtil::RaycastResult RaycastUtil::raycast(const glm::vec3& rayStart,
                                               const glm::vec3& rayDirection,
                                               const World& world,
                                               float maxDistance) {
    RaycastResult result;
    result.hit = false;

    // Normalize ray direction
    glm::vec3 rayDir = glm::normalize(rayDirection);

    // Avoid division by zero
    const float EPSILON = 1e-6f;
    if (std::abs(rayDir.x) < EPSILON) rayDir.x = (rayDir.x > 0) ? EPSILON : -EPSILON;
    if (std::abs(rayDir.y) < EPSILON) rayDir.y = (rayDir.y > 0) ? EPSILON : -EPSILON;
    if (std::abs(rayDir.z) < EPSILON) rayDir.z = (rayDir.z > 0) ? EPSILON : -EPSILON;

    // Starting voxel - use precise calculation
    glm::ivec3 voxel(
        static_cast<int>(std::floor(rayStart.x)),
        static_cast<int>(std::floor(rayStart.y)),
        static_cast<int>(std::floor(rayStart.z))
    );

    // Step direction
    glm::ivec3 stepDir(
        rayDir.x >= 0.0f ? 1 : -1,
        rayDir.y >= 0.0f ? 1 : -1,
        rayDir.z >= 0.0f ? 1 : -1
    );

    // Delta distances
    glm::vec3 deltaDist(
        std::abs(1.0f / rayDir.x),
        std::abs(1.0f / rayDir.y),
        std::abs(1.0f / rayDir.z)
    );

    // Side distances - corrected calculation
    glm::vec3 sideDist;
    
    if (rayDir.x >= 0) {
        sideDist.x = ((float)voxel.x + 1.0f - rayStart.x) / rayDir.x;
    } else {
        sideDist.x = ((float)voxel.x - rayStart.x) / rayDir.x;
    }
    
    if (rayDir.y >= 0) {
        sideDist.y = ((float)voxel.y + 1.0f - rayStart.y) / rayDir.y;
    } else {
        sideDist.y = ((float)voxel.y - rayStart.y) / rayDir.y;
    }
    
    if (rayDir.z >= 0) {
        sideDist.z = ((float)voxel.z + 1.0f - rayStart.z) / rayDir.z;
    } else {
        sideDist.z = ((float)voxel.z - rayStart.z) / rayDir.z;
    }

    float t = 0.0f;  // Ray parameter
    const int maxSteps = static_cast<int>(maxDistance * 2.0f + 1);
    glm::ivec3 normal(0, 0, 0);

    for (int step = 0; step < maxSteps && t < maxDistance; ++step) {
        // Bounds check
        if (voxel.y < 0 || voxel.y >= CHUNK_HEIGHT) {
            break;
        }

        // Check current block
        BlockType blockType = world.getBlockType(voxel);
        if (blockType != BlockType::AIR) {
            result.hit = true;
            result.blockPos = voxel;
            result.hitPoint = rayStart + t * rayDir;
            result.normal = normal;
            result.blockType = blockType;
            return result;
        }

        // Determine which axis to step along
        if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
            t = sideDist.x;
            sideDist.x += deltaDist.x;
            voxel.x += stepDir.x;
            normal = glm::ivec3(-stepDir.x, 0, 0);
        } else if (sideDist.y < sideDist.z) {
            t = sideDist.y;
            sideDist.y += deltaDist.y;
            voxel.y += stepDir.y;
            normal = glm::ivec3(0, -stepDir.y, 0);
        } else {
            t = sideDist.z;
            sideDist.z += deltaDist.z;
            voxel.z += stepDir.z;
            normal = glm::ivec3(0, 0, -stepDir.z);
        }
    }

    return result;
}