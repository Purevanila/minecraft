#include "entities/ItemEntity.h"
#include "engine/AssetManager.h"
#include "engine/graphics/Texture.h"
#include "engine/graphics/Shader.h"
#include "world/World.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

ItemEntity::ItemEntity(const glm::vec3& position, BlockType blockType, World* world)
    : Entity(position), m_blockType(blockType), m_rotationY(0.0f), m_bobOffset(0.0f),
      m_bobSpeed(2.0f), m_timeAlive(0.0f), m_collectionRadius(0.8f), m_collected(false),
      m_onGround(false), m_world(world) {
    
    std::cout << "Creating ItemEntity at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    
    // Initialize with some upward velocity and random horizontal spread
    float randomX = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
    float randomZ = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
    m_velocity = glm::vec3(randomX, 0.2f, randomZ);
    
    m_size = glm::vec3(0.25f, 0.25f, 0.25f); // Small cube
    
    // Get texture for this block type
    m_texture = getTextureForBlockType(blockType);
    
    if (!m_texture) {
        std::cout << "Warning: No texture found for block type " << static_cast<int>(blockType) << std::endl;
    } else {
        std::cout << "Texture loaded successfully for block type " << static_cast<int>(blockType) << std::endl;
    }
    
    // Get shader
    auto& assetManager = AssetManager::getInstance();
    m_shader = assetManager.loadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    
    if (!m_shader) {
        std::cout << "Warning: Failed to get basic shader for ItemEntity" << std::endl;
    } else {
        std::cout << "Basic shader loaded successfully for ItemEntity" << std::endl;
    }
    
    initializeRenderData();
    std::cout << "ItemEntity created successfully" << std::endl;
}

void ItemEntity::update(float deltaTime) {
    if (m_collected) return;
    
    m_timeAlive += deltaTime;
    
    // Apply physics
    applyPhysics(deltaTime);
    
    // Update position
    m_position += m_velocity * deltaTime;
    
    // Rotation animation
    m_rotationY += 90.0f * deltaTime; // Rotate 90 degrees per second
    if (m_rotationY >= 360.0f) {
        m_rotationY -= 360.0f;
    }
    
    // Bobbing animation when on ground (subtle)
    if (m_onGround) {
        m_bobOffset = sin(m_timeAlive * m_bobSpeed) * 0.02f; // Reduced from 0.05f to 0.02f
    }
}

void ItemEntity::render() {
    if (m_collected || !m_texture || !m_shader) return;
    
    m_shader->use();
    
    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position + glm::vec3(0.0f, m_bobOffset, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, m_size);
    
    m_shader->setMat4("model", model);
    
    // Bind texture
    m_texture->bind(0);
    m_shader->setInt("texture1", 0);
    
    // Render the cube
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool ItemEntity::canBeCollected() const {
    return !m_collected && m_timeAlive > 0.5f; // Can't be collected immediately after spawn
}

void ItemEntity::applyPhysics(float deltaTime) {
    if (!m_world) return;
    
    const float gravity = -9.81f;
    const float terminalVelocity = -10.0f;
    const float groundFriction = 0.9f;
    const float airResistance = 0.98f;
    
    if (!m_onGround) {
        // Apply gravity
        m_velocity.y += gravity * deltaTime;
        
        // Terminal velocity
        if (m_velocity.y < terminalVelocity) {
            m_velocity.y = terminalVelocity;
        }
        
        // Air resistance
        m_velocity.x *= airResistance;
        m_velocity.z *= airResistance;
        
        // Check for ground collision using world
        glm::vec3 nextPosition = m_position + m_velocity * deltaTime;
        
        // Check the block below the item
        int blockX = static_cast<int>(std::floor(nextPosition.x));
        int blockY = static_cast<int>(std::floor(nextPosition.y - 0.1f)); // Check slightly below
        int blockZ = static_cast<int>(std::floor(nextPosition.z));
        
        BlockType blockBelow = m_world->getBlock(blockX, blockY, blockZ);
        
        // If there's a solid block below, stop falling
        if (blockBelow != BlockType::AIR && blockBelow != BlockType::WATER) {
            // Place the item on top of the block (closer to the surface)
            m_position.y = blockY + 1.01f; // 1.0 for the block height + 0.01 for minimal clearance
            m_velocity.y = 0.0f;
            m_onGround = true;
        }
    } else {
        // Ground friction
        m_velocity.x *= groundFriction;
        m_velocity.z *= groundFriction;
        
        // Stop very small movements
        if (glm::length(glm::vec2(m_velocity.x, m_velocity.z)) < 0.01f) {
            m_velocity.x = 0.0f;
            m_velocity.z = 0.0f;
        }
        
        // Check if still on ground (in case block below was removed)
        int blockX = static_cast<int>(std::floor(m_position.x));
        int blockY = static_cast<int>(std::floor(m_position.y - 0.2f));
        int blockZ = static_cast<int>(std::floor(m_position.z));
        
        BlockType blockBelow = m_world->getBlock(blockX, blockY, blockZ);
        
        if (blockBelow == BlockType::AIR || blockBelow == BlockType::WATER) {
            m_onGround = false; // Start falling again
        }
    }
}

std::shared_ptr<Texture> ItemEntity::getTextureForBlockType(BlockType blockType) {
    auto& assetManager = AssetManager::getInstance();
    
    switch (blockType) {
        case BlockType::STONE:
            return assetManager.getTexture("assets/textures/stone.png");
        case BlockType::GRASS:
            return assetManager.getTexture("assets/textures/grass.png");
        case BlockType::DIRT:
            return assetManager.loadTexture("assets/textures/grass.png"); // Use grass texture for dirt as fallback
        case BlockType::WOOD:
        case BlockType::OAK_LOG:
            return assetManager.getTexture("assets/textures/oak.png");
        case BlockType::LEAVES:
            return assetManager.getTexture("assets/textures/oakleave.png");
        case BlockType::GRAVEL:
            return assetManager.getTexture("assets/textures/gravel.png");
        case BlockType::WATER:
            return assetManager.getTexture("assets/textures/water.webp");
        default:
            // Default to stone texture if no specific texture found
            return assetManager.getTexture("assets/textures/stone.png");
    }
}

void ItemEntity::initializeRenderData() {
    // Cube vertices with texture coordinates
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        
        // Back face
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        
        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        
        // Right face
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0,  1,  2,   2,  3,  0,   // Front
        4,  5,  6,   6,  7,  4,   // Back
        8,  9, 10,  10, 11,  8,   // Left
       12, 13, 14,  14, 15, 12,   // Right
       16, 17, 18,  18, 19, 16,   // Top
       20, 21, 22,  22, 23, 20    // Bottom
    };
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}
