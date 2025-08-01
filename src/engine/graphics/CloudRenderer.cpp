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
    
    // Generate initial cloud mesh around origin
    // This mesh will be reused and repositioned smoothly
    generateCloudMeshAroundPosition(glm::vec3(0.0f));
    
    std::cout << "CloudRenderer initialized successfully" << std::endl;
    return true;
}


void CloudRenderer::generateCloudMeshAroundPosition(const glm::vec3& centerPos) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int vertexIndex = 0;
    
    // Create a moderately sized grid - not too many clouds
    const int cloudGridSize = g_worldConfig.clouds.gridSize * 2; // Reduced from 4x to 2x
    const float cloudSpacing = g_worldConfig.clouds.spacing;
    const float gridCenter = (cloudGridSize - 1) * cloudSpacing * 0.5f;
    const int cloudLayers = g_worldConfig.clouds.layers;
    const float layerSpacing = g_worldConfig.clouds.layerSpacing;
    
    // Use position-based seeding for perfectly consistent, repeating cloud patterns
    // Create single layer of 3D cloud cubes like Minecraft
    float cloudLayerHeight = g_worldConfig.clouds.height;
    float layerDensity = g_worldConfig.clouds.density * 0.2f; // Sparse distribution
        
        for (int x = 0; x < cloudGridSize; x++) {
            for (int z = 0; z < cloudGridSize; z++) {
                float worldX = centerPos.x + (x * cloudSpacing - gridCenter);
                float worldZ = centerPos.z + (z * cloudSpacing - gridCenter);
                
                // Add random offset to break the grid pattern
                float randomX = sin(worldX * 0.1f + worldZ * 0.07f) * cloudSpacing * 0.4f;
                float randomZ = cos(worldX * 0.08f + worldZ * 0.12f) * cloudSpacing * 0.4f;
                worldX += randomX;
                worldZ += randomZ;
                
                // Use larger spacing for bigger, more Minecraft-like cloud areas
                float patternX = fmod(worldX + 1000.0f, 16.0f); // Minecraft-style spacing
                float patternZ = fmod(worldZ + 1000.0f, 16.0f);
                
                // Multiple noise layers for more natural cloud shapes like Minecraft
                float cloudNoise1 = sin(patternX * 0.3f) * cos(patternZ * 0.3f);
                float cloudNoise2 = sin(patternX * 0.6f + 50.0f) * cos(patternZ * 0.6f + 50.0f) * 0.5f;
                float cloudNoise3 = sin(patternX * 0.15f + 100.0f) * cos(patternZ * 0.15f + 100.0f) * 0.3f;
                float combinedNoise = cloudNoise1 + cloudNoise2 + cloudNoise3;
                
                // Balanced threshold for nice scattered clouds like Minecraft
                float threshold = 0.2f; // Moderate threshold for good cloud coverage
                
                // Place clouds based on the noise value and layer density
                if (combinedNoise > threshold) {
                    // Create larger Minecraft-style cloud cubes that connect naturally
                    float cloudSize = 6.0f; // Larger size like Minecraft
                    float cloudHeight = 4.0f; // Taller clouds for more presence
                    
                    createCloudCube(worldX, worldZ, cloudLayerHeight, cloudSize, cloudHeight, vertices, indices, vertexIndex);
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

void CloudRenderer::createCloudCube(float x, float z, float baseHeight, float size, float height, std::vector<float>& vertices, std::vector<unsigned int>& indices, unsigned int& vertexIndex) {
    float halfSize = size * 0.5f;
    float halfHeight = height * 0.5f;
    
    // Create a 3D cube for each cloud - 6 faces like a Minecraft block
    
    // Top face (most important - what you see from below)
    vertices.insert(vertices.end(), {
        x - halfSize, baseHeight + halfHeight, z - halfSize, 0.0f, 0.0f,  // Bottom-left
        x + halfSize, baseHeight + halfHeight, z - halfSize, 1.0f, 0.0f,  // Bottom-right
        x + halfSize, baseHeight + halfHeight, z + halfSize, 1.0f, 1.0f,  // Top-right
        x - halfSize, baseHeight + halfHeight, z + halfSize, 0.0f, 1.0f   // Top-left
    });
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    vertexIndex += 4;
    
    // Bottom face
    vertices.insert(vertices.end(), {
        x - halfSize, baseHeight - halfHeight, z - halfSize, 0.0f, 0.0f,
        x - halfSize, baseHeight - halfHeight, z + halfSize, 0.0f, 1.0f,
        x + halfSize, baseHeight - halfHeight, z + halfSize, 1.0f, 1.0f,
        x + halfSize, baseHeight - halfHeight, z - halfSize, 1.0f, 0.0f
    });
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    vertexIndex += 4;
    
    // Front face
    vertices.insert(vertices.end(), {
        x - halfSize, baseHeight - halfHeight, z + halfSize, 0.0f, 0.0f,
        x - halfSize, baseHeight + halfHeight, z + halfSize, 0.0f, 1.0f,
        x + halfSize, baseHeight + halfHeight, z + halfSize, 1.0f, 1.0f,
        x + halfSize, baseHeight - halfHeight, z + halfSize, 1.0f, 0.0f
    });
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    vertexIndex += 4;
    
    // Back face
    vertices.insert(vertices.end(), {
        x + halfSize, baseHeight - halfHeight, z - halfSize, 0.0f, 0.0f,
        x + halfSize, baseHeight + halfHeight, z - halfSize, 0.0f, 1.0f,
        x - halfSize, baseHeight + halfHeight, z - halfSize, 1.0f, 1.0f,
        x - halfSize, baseHeight - halfHeight, z - halfSize, 1.0f, 0.0f
    });
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    vertexIndex += 4;
    
    // Left face
    vertices.insert(vertices.end(), {
        x - halfSize, baseHeight - halfHeight, z - halfSize, 0.0f, 0.0f,
        x - halfSize, baseHeight + halfHeight, z - halfSize, 0.0f, 1.0f,
        x - halfSize, baseHeight + halfHeight, z + halfSize, 1.0f, 1.0f,
        x - halfSize, baseHeight - halfHeight, z + halfSize, 1.0f, 0.0f
    });
    indices.insert(indices.end(), {
        vertexIndex, vertexIndex + 1, vertexIndex + 2,
        vertexIndex, vertexIndex + 2, vertexIndex + 3
    });
    vertexIndex += 4;
    
    // Right face
    vertices.insert(vertices.end(), {
        x + halfSize, baseHeight - halfHeight, z + halfSize, 0.0f, 0.0f,
        x + halfSize, baseHeight + halfHeight, z + halfSize, 0.0f, 1.0f,
        x + halfSize, baseHeight + halfHeight, z - halfSize, 1.0f, 1.0f,
        x + halfSize, baseHeight - halfHeight, z - halfSize, 1.0f, 0.0f
    });
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
    
    // Only regenerate mesh if player moved VERY far (like switching worlds or teleporting)
    float distanceFromLastUpdate = glm::length(playerPos - m_lastPlayerPos);
    if (distanceFromLastUpdate > g_worldConfig.clouds.updateDistance * 3.0f) {
        generateCloudMeshAroundPosition(playerPos);
        m_lastPlayerPos = playerPos;
        m_lastUpdateTime = time;
    }
    
    // Enable alpha blending for cloud transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write to depth buffer to prevent overlapping issues
    
    m_shader->use();
    
    // Create model matrix with smooth world-space movement like Minecraft
    glm::mat4 model = glm::mat4(1.0f);
    
    // Add smooth, continuous movement in world space like Minecraft
    // Clouds drift very slowly across the world at a peaceful pace
    float cloudSpeed = g_worldConfig.clouds.speed * 1.5f; // Moderate speed for Minecraft feel
    float cloudOffsetX = time * cloudSpeed;
    float cloudOffsetZ = time * cloudSpeed * 0.15f; // Very slight diagonal like Minecraft
    
    // Apply the movement translation - this makes clouds float across the world
    model = glm::translate(model, glm::vec3(cloudOffsetX, 0.0f, cloudOffsetZ));
    
    // Set matrices
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    
    // Set simple lighting (clouds should be bright)
    m_shader->setVec3("lightPos", glm::vec3(100.0f, 100.0f, 100.0f));
    m_shader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    m_shader->setVec3("viewPos", playerPos);
    
    // Pass time for smooth animated cloud movement - the shader handles all movement
    m_shader->setFloat("time", time * g_worldConfig.clouds.speed);
    
    // Pass player position for advanced effects if needed
    m_shader->setVec3("playerPos", playerPos);
    
    // No texture needed - shader creates procedural cloud patterns
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
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
