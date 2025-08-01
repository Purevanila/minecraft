#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class Mesh;

/**
 * CloudRenderer - Renders smooth, infinitely moving clouds like Minecraft
 * Creates a large static cloud mesh that moves smoothly via shader animation
 * No discrete mesh regeneration - clouds move continuously through time
 */
class CloudRenderer {
public:
    CloudRenderer();
    ~CloudRenderer();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& playerPos);
    void update(float deltaTime);
    void cleanup();
    
    // Configuration
    void setCloudHeight(float height) { m_cloudHeight = height; }
    void setCloudSpeed(float speed) { m_cloudSpeed = speed; }
    void setCloudDensity(float density) { m_cloudDensity = density; }
    void setCloudUpdateDistance(float distance) { m_cloudUpdateDistance = distance; }
    void setCloudGridSize(int size) { m_cloudGridSize = size; }
    void setCloudSpacing(float spacing) { m_cloudSpacing = spacing; }
    void setCloudLayers(int layers) { m_cloudLayers = layers; }
    void setCloudLayerSpacing(float spacing) { m_cloudLayerSpacing = spacing; }
    
private:
    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<Mesh> m_cloudMesh;
    
    // Cloud parameters
    float m_cloudHeight = 80.0f;     // Height above ground (balanced height)
    float m_cloudSpeed = 0.01f;      // Movement speed
    float m_cloudDensity = 0.5f;     // How dense the clouds are
    float m_time = 0.0f;             // Current animation time
    
    // Player position tracking for cloud generation
    glm::vec3 m_lastPlayerPos = glm::vec3(0.0f);
    float m_cloudUpdateDistance = 64.0f;  // Regenerate clouds when player moves this far
    
    // Cloud generation parameters (configurable)
    int m_cloudGridSize = 32;
    float m_cloudSpacing = 8.0f;
    int m_cloudLayers = 6;
    float m_cloudLayerSpacing = 2.5f;
    
    // Smooth movement parameters
    float m_windVariation = 0.0f;     // For natural wind movement
    float m_lastUpdateTime = 0.0f;    // Track time for smooth updates
    
    void generateCloudMeshAroundPosition(const glm::vec3& centerPos);
    void createCloudCube(float x, float z, float baseHeight, float size, float height, std::vector<float>& vertices, std::vector<unsigned int>& indices, unsigned int& vertexIndex);
};
