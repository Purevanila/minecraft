#include "engine/graphics/CloudRenderer.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/Mesh.h"
#include "engine/AssetManager.h"
#include "engine/graphics/OpenGL.h"
#include "world/WorldConfig.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <functional>

// External reference to global world configuration
extern WorldConfig g_worldConfig;

CloudRenderer::CloudRenderer() {
}

CloudRenderer::~CloudRenderer() {
    cleanup();
}

bool CloudRenderer::initialize() {
    std::cout << "Initializing CloudRenderer..." << std::endl;
    
    // Create cloud shader
    m_shader = AssetManager::getInstance().loadShader(
        "assets/shaders/cloud.vert", 
        "assets/shaders/cloud.frag"
    );
    
    if (!m_shader) {
        std::cout << "Failed to load cloud shaders" << std::endl;
        return false;
    }
    
    // Generate cloud mesh
    generateCloudMesh();
    
    std::cout << "CloudRenderer initialized successfully" << std::endl;
    return true;
}

void CloudRenderer::generateCloudMesh() {
    generateCloudMeshAroundPosition(glm::vec3(0.0f)); // Initial generation around origin
}

void CloudRenderer::generateCloudMeshAroundPosition(const glm::vec3& centerPos) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int vertexIndex = 0;
    
    // Create a larger grid for smoother transitions - expand by 50%
    const int cloudGridSize = static_cast<int>(g_worldConfig.clouds.gridSize * 1.5f);
    const float cloudSpacing = g_worldConfig.clouds.spacing;
    const float gridCenter = (cloudGridSize - 1) * cloudSpacing * 0.5f;
    const int cloudLayers = g_worldConfig.clouds.layers;
    const float layerSpacing = g_worldConfig.clouds.layerSpacing;
    
    // Use position-based seeding for consistent clouds that change with location
    // Use a hash-based approach to ensure clouds are consistent for each area
    std::mt19937 gen(static_cast<unsigned int>(
        std::hash<float>{}(centerPos.x) ^ 
        std::hash<float>{}(centerPos.z) ^ 
        12345
    ));
    std::uniform_real_distribution<float> sizeDist(6.0f, 14.0f); // Slightly larger clouds
    std::uniform_real_distribution<float> densityDist(0.0f, 1.0f);
    
    for (int layer = 0; layer < cloudLayers; layer++) {
        // Start layers at base height and extend upward with some downward
        float layerHeight = g_worldConfig.clouds.height - (cloudLayers/4) * layerSpacing + (layer * layerSpacing);
        float layerDensity = g_worldConfig.clouds.density * (0.8f - abs(layer - cloudLayers/2) * 0.1f); // Denser in middle layers
        
        for (int x = 0; x < cloudGridSize; x++) {
            for (int z = 0; z < cloudGridSize; z++) {
                // Only place clouds randomly based on layer density
                if (densityDist(gen) < layerDensity) {
                    float worldX = centerPos.x + (x * cloudSpacing - gridCenter);
                    float worldZ = centerPos.z + (z * cloudSpacing - gridCenter);
                    // Vary cloud size based on distance from center layer
                    float layerFactor = 1.0f - abs(layer - cloudLayers/2) * 0.05f;
                    float cloudSize = sizeDist(gen) * layerFactor;
                    
                    createCloudQuad(worldX, worldZ, layerHeight, cloudSize, vertices, indices, vertexIndex);
                }
            }
        }
    }
    
    // Create mesh
    m_cloudMesh = std::make_unique<Mesh>();
    
    // Convert vertices to proper Vertex format
    std::vector<Vertex> meshVertices;
    for (size_t i = 0; i < vertices.size(); i += 5) {
        glm::vec3 position(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec2 texCoords(vertices[i + 3], vertices[i + 4]);
        glm::vec3 normal(0.0f, 1.0f, 0.0f); // Clouds face up
        meshVertices.emplace_back(position, texCoords, normal);
    }
    
    m_cloudMesh->setVertices(meshVertices);
    m_cloudMesh->setIndices(indices);
    m_cloudMesh->upload();
}

void CloudRenderer::createCloudQuad(float x, float z, float height, float size, std::vector<float>& vertices, std::vector<unsigned int>& indices, unsigned int& vertexIndex) {
    float halfSize = size * 0.5f;
    
    // Cloud quad vertices (y is up)
    // Bottom-left
    vertices.insert(vertices.end(), {
        x - halfSize, height, z - halfSize, 0.0f, 0.0f
    });
    
    // Bottom-right  
    vertices.insert(vertices.end(), {
        x + halfSize, height, z - halfSize, 1.0f, 0.0f
    });
    
    // Top-right
    vertices.insert(vertices.end(), {
        x + halfSize, height, z + halfSize, 1.0f, 1.0f
    });
    
    // Top-left
    vertices.insert(vertices.end(), {
        x - halfSize, height, z + halfSize, 0.0f, 1.0f
    });
    
    // Two triangles to form a quad
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    
    vertexIndex += 4;
}

void CloudRenderer::update(float deltaTime) {
    m_time += deltaTime;
}

void CloudRenderer::render(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& playerPos) {
    if (!m_shader || !m_cloudMesh) {
        return;
    }
    
    // Smoothly check if player has moved far enough to warrant cloud regeneration
    float distanceFromLastUpdate = glm::length(playerPos - m_lastPlayerPos);
    if (distanceFromLastUpdate > g_worldConfig.clouds.updateDistance * 0.8f) { // Update earlier for smoother transition
        // Only regenerate if enough time has passed to avoid constant updates
        if (time - m_lastUpdateTime > 2.0f) { // Minimum 2 seconds between updates
            generateCloudMeshAroundPosition(playerPos);
            m_lastPlayerPos = playerPos;
            m_lastUpdateTime = time;
        }
    }
    
    // Enable alpha blending for cloud transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write to depth buffer
    
    m_shader->use();
    
    // Create model matrix with smooth, Minecraft-like cloud movement
    glm::mat4 model = glm::mat4(1.0f);
    
    // Smooth, continuous movement like in Minecraft - use fractional time for smoothness
    float cloudOffsetX = time * g_worldConfig.clouds.speed;
    float cloudOffsetZ = time * g_worldConfig.clouds.speed * 0.3f; // Slight diagonal movement
    
    // Apply smooth translation - this should be continuous, not discrete
    model = glm::translate(model, glm::vec3(cloudOffsetX, 0.0f, cloudOffsetZ));
    
    // Set matrices
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    
    // Set simple lighting (clouds should be bright)
    m_shader->setVec3("lightPos", glm::vec3(100.0f, 100.0f, 100.0f));
    m_shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shader->setVec3("viewPos", glm::vec3(0.0f, 10.0f, 0.0f));
    
    // Pass smooth time for animated cloud effects
    m_shader->setFloat("time", time);
    
    // Use a simple white color for clouds (no texture needed)
    m_shader->setInt("texture1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0); // No texture, will use white
    
    // Render cloud mesh
    m_cloudMesh->render();
    
    // Restore render state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void CloudRenderer::cleanup() {
    m_cloudMesh.reset();
    m_shader.reset();
}
