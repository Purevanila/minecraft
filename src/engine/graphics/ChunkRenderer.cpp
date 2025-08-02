#include "engine/graphics/ChunkRenderer.h"
#include "engine/AssetManager.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/Texture.h"
#include "world/Chunk.h"
#include "world/BlockDefinition.h"
#include "engine/graphics/OpenGL.h"
#include <iostream>

ChunkRenderer::ChunkRenderer() 
    : m_lightPos(100.0f, 100.0f, 100.0f)
    , m_lightColor(1.0f, 1.0f, 0.9f) {
}

ChunkRenderer::~ChunkRenderer() = default;

bool ChunkRenderer::initialize() {
    m_shader = AssetManager::getInstance().loadShader(
        "assets/shaders/basic.vert", 
        "assets/shaders/basic.frag"
    );
    if (!m_shader) {
        return false;
    }
    
    m_grassTexture = AssetManager::getInstance().loadTexture("assets/textures/grass.png");
    if (!m_grassTexture) {
        std::cout << "Failed to load grass texture!" << std::endl;
        return false;
    }
    
    m_stoneTexture = AssetManager::getInstance().loadTexture("assets/textures/stone.png");
    if (!m_stoneTexture) {
        std::cout << "Failed to load stone texture!" << std::endl;
        return false;
    }
    
    m_waterTexture = AssetManager::getInstance().loadTexture("assets/textures/water.webp");
    if (!m_waterTexture) {
        std::cout << "Failed to load water texture!" << std::endl;
        return false;
    }
    
    m_oakTexture = AssetManager::getInstance().loadTexture("assets/textures/oak.png");
    if (!m_oakTexture) {
        std::cout << "Failed to load oak texture!" << std::endl;
        return false;
    }
    
    m_oakLeavesTexture = AssetManager::getInstance().loadTexture("assets/textures/oakleave.png");
    if (!m_oakLeavesTexture) {
        std::cout << "Failed to load oak leaves texture!" << std::endl;
        return false;
    }
    
    m_gravelTexture = AssetManager::getInstance().loadTexture("assets/textures/gravel.png");
    if (!m_gravelTexture) {
        std::cout << "Failed to load gravel texture!" << std::endl;
        return false;
    }
    
    m_sandTexture = AssetManager::getInstance().loadTexture("assets/textures/sand.png");
    if (!m_sandTexture) {
        std::cout << "Failed to load sand texture!" << std::endl;
        return false;
    }
    
    // Initialize texture coordinates
    initializeTextureCoords();
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

void ChunkRenderer::renderChunk(const Chunk& chunk, const glm::mat4& view, const glm::mat4& projection) {
    if (!m_shader) return;
    
    // Activate shader
    m_shader->use();
    
    // Set up matrices
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    
    // Set up lighting
    m_shader->setVec3("lightPos", m_lightPos);
    m_shader->setVec3("lightColor", m_lightColor);
    m_shader->setVec3("viewPos", glm::vec3(0.0f, 10.0f, 0.0f));
    
    // Get all registered block types from BlockDefinitionRegistry
    auto& registry = BlockDefinitionRegistry::getInstance();
    auto blockTypes = registry.getAllBlockTypes();
    
    // 🎨 PHASE 1: Render all SOLID blocks first (better depth sorting)
    if (m_grassTexture) {
        m_grassTexture->bind(0);
        m_shader->setInt("texture1", 0);
        const_cast<Chunk&>(chunk).render(view, projection);
    }
    
    // Render solid blocks that need separate meshes
    for (BlockType blockType : blockTypes) {
        const auto& definition = registry.getDefinition(blockType);
        
        if (definition.needsSeparateMesh && !definition.transparent) {
            renderBlockType(chunk, blockType, view, projection);
        }
    }
    
    // 🌿 PHASE 2: Render all TRANSPARENT blocks last (proper alpha blending)
    for (BlockType blockType : blockTypes) {
        const auto& definition = registry.getDefinition(blockType);
        
        if (definition.needsSeparateMesh && definition.transparent) {
            renderBlockType(chunk, blockType, view, projection);
        }
    }
}

void ChunkRenderer::renderBlockType(const Chunk& chunk, BlockType blockType, 
                                          const glm::mat4& view, const glm::mat4& projection) {
    const auto& definition = BlockDefinitionRegistry::getInstance().getDefinition(blockType);
    
    // Select appropriate texture and rendering method
    std::shared_ptr<Texture> texture = nullptr;
    
    switch (blockType) {
        case BlockType::WATER:
            texture = m_waterTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawWaterMesh();
            }
            break;
            
        case BlockType::OAK_LOG:
            texture = m_oakTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawOakMesh();
            }
            break;
            
        case BlockType::LEAVES:
            texture = m_oakLeavesTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawLeavesMesh();
            }
            break;
            
        case BlockType::STONE:
            texture = m_stoneTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawStoneMesh();
            }
            break;
            
        case BlockType::GRAVEL:
            texture = m_gravelTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawGravelMesh();
            }
            break;
            
        case BlockType::SAND:
            texture = m_sandTexture;
            if (texture) {
                texture->bind(0);
                m_shader->setInt("texture1", 0);
                chunk.drawSandMesh();
            }
            break;
            
        default:
            // Future block types can be added here easily
            break;
    }
}

TextureCoords ChunkRenderer::getTextureCoords(BlockType blockType) const {
    auto it = m_textureCoords.find(blockType);
    if (it != m_textureCoords.end()) {
        return it->second;
    }
    return TextureCoords(); // Default full texture coordinates
}

void ChunkRenderer::initializeTextureCoords() {
    // For now, each texture uses full coordinates (0,0) to (1,1)
    // In the future, this will support texture atlases
    
    m_textureCoords[BlockType::GRASS] = TextureCoords(0.0f, 0.0f, 1.0f, 1.0f);
    m_textureCoords[BlockType::DIRT] = TextureCoords(0.0f, 0.0f, 1.0f, 1.0f);
    m_textureCoords[BlockType::STONE] = TextureCoords(0.0f, 0.0f, 1.0f, 1.0f);
    m_textureCoords[BlockType::WATER] = TextureCoords(0.0f, 0.0f, 1.0f, 1.0f);
    m_textureCoords[BlockType::OAK_LOG] = TextureCoords(0.0f, 0.0f, 1.0f, 1.0f);
    
    std::cout << "Initialized texture coordinates for " << m_textureCoords.size() << " block types" << std::endl;
}
