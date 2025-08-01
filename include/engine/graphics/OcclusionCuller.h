#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

class Shader;

/**
 * GPU Occlusion Culling System
 * Uses OpenGL queries to determine which chunks are actually visible
 * Dramatically improves performance in complex scenes
 */
class OcclusionCuller {
public:
    OcclusionCuller();
    ~OcclusionCuller();
    
    bool initialize();
    void cleanup();
    
    // Test if a chunk bounding box is visible
    bool isChunkVisible(const glm::vec3& chunkMin, const glm::vec3& chunkMax, 
                       const glm::mat4& viewProjection);
    
    // Begin occlusion testing frame
    void beginFrame();
    
    // End occlusion testing frame and retrieve results
    void endFrame();
    
    // Enable/disable occlusion culling
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    // Performance statistics
    int getTestedChunks() const { return m_testedChunks; }
    int getVisibleChunks() const { return m_visibleChunks; }
    float getCullingEfficiency() const { 
        return m_testedChunks > 0 ? (float)m_visibleChunks / m_testedChunks : 1.0f; 
    }

private:
    struct OcclusionQuery {
        unsigned int queryId;
        glm::vec3 chunkMin, chunkMax;
        bool resultReady;
        bool isVisible;
        int framesSinceTest;
    };
    
    std::vector<std::unique_ptr<OcclusionQuery>> m_queries;
    std::unique_ptr<Shader> m_occlusionShader;
    
    unsigned int m_boundingBoxVAO, m_boundingBoxVBO;
    bool m_enabled;
    bool m_initialized;
    
    // Performance tracking
    int m_testedChunks;
    int m_visibleChunks;
    
    // Create simple bounding box geometry for occlusion testing
    void createBoundingBoxGeometry();
    
    // Render a bounding box for occlusion testing
    void renderBoundingBox(const glm::vec3& min, const glm::vec3& max, 
                          const glm::mat4& viewProjection);
    
    // Get or create an occlusion query for a chunk
    OcclusionQuery* getQuery(const glm::vec3& chunkMin, const glm::vec3& chunkMax);
};
