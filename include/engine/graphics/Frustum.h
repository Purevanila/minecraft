#pragma once
#include <glm/glm.hpp>
#include <array>

/**
 * Frustum Culling for performance optimization
 * Only render chunks that are actually visible to the camera
 */
class Frustum {
public:
    // Frustum planes: left, right, bottom, top, near, far
    std::array<glm::vec4, 6> planes;
    
    void updateFromViewProjection(const glm::mat4& viewProj);
    bool isChunkVisible(const glm::vec3& chunkMin, const glm::vec3& chunkMax) const;
    
private:
    // Helper to normalize a plane equation
    void normalizePlane(glm::vec4& plane) const;
};

inline void Frustum::updateFromViewProjection(const glm::mat4& vp) {
    // Extract frustum planes from view-projection matrix
    // Left plane
    planes[0] = glm::vec4(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0], vp[3][3] + vp[3][0]);
    // Right plane  
    planes[1] = glm::vec4(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0], vp[3][3] - vp[3][0]);
    // Bottom plane
    planes[2] = glm::vec4(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1], vp[3][3] + vp[3][1]);
    // Top plane
    planes[3] = glm::vec4(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1], vp[3][3] - vp[3][1]);
    // Near plane
    planes[4] = glm::vec4(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2], vp[3][3] + vp[3][2]);
    // Far plane
    planes[5] = glm::vec4(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2], vp[3][3] - vp[3][2]);
    
    // Normalize all planes
    for (auto& plane : planes) {
        normalizePlane(plane);
    }
}

inline bool Frustum::isChunkVisible(const glm::vec3& chunkMin, const glm::vec3& chunkMax) const {
    // Check if chunk's bounding box intersects with frustum
    for (const auto& plane : planes) {
        glm::vec3 positive = chunkMin;
        glm::vec3 negative = chunkMax;
        
        // Find the positive and negative vertices
        if (plane.x >= 0) {
            positive.x = chunkMax.x;
            negative.x = chunkMin.x;
        }
        if (plane.y >= 0) {
            positive.y = chunkMax.y;
            negative.y = chunkMin.y;
        }
        if (plane.z >= 0) {
            positive.z = chunkMax.z;
            negative.z = chunkMin.z;
        }
        
        // Check if positive vertex is outside this plane
        if (glm::dot(glm::vec3(plane), positive) + plane.w < 0) {
            return false; // Chunk is completely outside this plane
        }
    }
    return true; // Chunk is visible
}

inline void Frustum::normalizePlane(glm::vec4& plane) const {
    float length = glm::length(glm::vec3(plane));
    if (length > 0.0f) {
        plane /= length;
    }
}
