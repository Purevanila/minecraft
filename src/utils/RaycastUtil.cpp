#include "utils/RaycastUtil.h"
#include "utils/RaycastDebug.h"
#include "world/World.h"
#include "world/Chunk.h"
#include <cmath>

RaycastUtil::RaycastResult RaycastUtil::raycast(const glm::vec3& rayStart,
                                               const glm::vec3& rayDirection,
                                               const World& world,
                                               float maxDistance) {
    RaycastResult result;
    result.hit = false;
    
    // DEBUG: Print ray data to check for garbage
    RAYCAST_DEBUG("Ray Start: (" << rayStart.x << ", " << rayStart.y << ", " << rayStart.z << ")");
    RAYCAST_DEBUG("Ray Direction (raw): (" << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << ")");
    RAYCAST_DEBUG("Ray Direction Length: " << glm::length(rayDirection));
    
    // Normalize ray direction
    glm::vec3 rayDir = glm::normalize(rayDirection);
    
    // DEBUG: Print normalized direction
    RAYCAST_DEBUG("Ray Direction (normalized): (" << rayDir.x << ", " << rayDir.y << ", " << rayDir.z << ")");
    RAYCAST_DEBUG("Normalized Length: " << glm::length(rayDir));
    
    // Avoid division by zero
    const float EPSILON = 1e-6f;
    if (std::abs(rayDir.x) < EPSILON) rayDir.x = (rayDir.x > 0) ? EPSILON : -EPSILON;
    if (std::abs(rayDir.y) < EPSILON) rayDir.y = (rayDir.y > 0) ? EPSILON : -EPSILON;
    if (std::abs(rayDir.z) < EPSILON) rayDir.z = (rayDir.z > 0) ? EPSILON : -EPSILON;

    // DEBUG: Print direction after epsilon correction
    RAYCAST_DEBUG("Ray Direction (after epsilon): (" << rayDir.x << ", " << rayDir.y << ", " << rayDir.z << ")");

    // Starting voxel - use precise calculation
    glm::ivec3 voxel(
        static_cast<int>(std::floor(rayStart.x)),
        static_cast<int>(std::floor(rayStart.y)),
        static_cast<int>(std::floor(rayStart.z))
    );
    
    // DEBUG: Print starting voxel
    RAYCAST_DEBUG("Starting Voxel: (" << voxel.x << ", " << voxel.y << ", " << voxel.z << ")");

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

    // DEBUG: Print initial raycast parameters
    RAYCAST_DEBUG("Max Distance: " << maxDistance << ", Max Steps: " << maxSteps);
    RAYCAST_DEBUG("Step Dir: (" << stepDir.x << ", " << stepDir.y << ", " << stepDir.z << ")");
    RAYCAST_DEBUG("Delta Dist: (" << deltaDist.x << ", " << deltaDist.y << ", " << deltaDist.z << ")");
    RAYCAST_DEBUG("Side Dist: (" << sideDist.x << ", " << sideDist.y << ", " << sideDist.z << ")");

    for (int step = 0; step < maxSteps && t < maxDistance; ++step) {
        // DEBUG: Print step info (only first few steps to avoid spam)
        if (step < 5) {
            RAYCAST_DEBUG("Step " << step << ": Voxel(" << voxel.x << ", " << voxel.y << ", " << voxel.z << "), t=" << t);
        }
        
        // Bounds check
        if (voxel.y < 0 || voxel.y >= CHUNK_HEIGHT) {
            RAYCAST_DEBUG("Out of bounds - breaking at step " << step);
            break;
        }

        // Check current block
        BlockType blockType = world.getBlockType(voxel);
        
        // DEBUG: Print block check (only for first few steps or when hit)
        if (step < 5 || blockType != BlockType::AIR) {
            RAYCAST_DEBUG("Block at (" << voxel.x << ", " << voxel.y << ", " << voxel.z << ") = " << static_cast<int>(blockType));
        }
        
        // Skip the first block if we're starting inside it (t is very small)
        if (blockType != BlockType::AIR && t > 0.01f) {
            RAYCAST_DEBUG("HIT! Block type: " << static_cast<int>(blockType) << " at step " << step);
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

    // DEBUG: No hit detected
    RAYCAST_DEBUG("NO HIT - Raycast completed without hitting any blocks");
    RAYCAST_DEBUG("Final t: " << t << ", Max Distance: " << maxDistance);

    return result;
}