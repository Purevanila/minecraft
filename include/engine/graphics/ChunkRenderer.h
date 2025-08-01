#pragma once
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "world/Block.h"

class Shader;
class Chunk;
class Texture;

struct TextureCoords {
    float u1, v1, u2, v2;
    
    TextureCoords() : u1(0), v1(0), u2(1), v2(1) {}
    TextureCoords(float u1, float v1, float u2, float v2) 
        : u1(u1), v1(v1), u2(u2), v2(v2) {}
};

class ChunkRenderer {
public:
    ChunkRenderer();
    ~ChunkRenderer();
    
    bool initialize();
    void renderChunk(const Chunk& chunk, const glm::mat4& view, const glm::mat4& projection);
    
    // Get texture coordinates for a block type
    TextureCoords getTextureCoords(BlockType blockType) const;
    
private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Texture> m_grassTexture;
    std::shared_ptr<Texture> m_stoneTexture;
    std::shared_ptr<Texture> m_waterTexture;
    std::shared_ptr<Texture> m_oakTexture;
    std::shared_ptr<Texture> m_oakLeavesTexture;
    std::shared_ptr<Texture> m_gravelTexture;
    
    // Lighting setup
    glm::vec3 m_lightPos;
    glm::vec3 m_lightColor;
    
    // Texture coordinate mapping (for future atlas support)
    std::unordered_map<BlockType, TextureCoords> m_textureCoords;
    
    void initializeTextureCoords();
    void renderBlockType(const Chunk& chunk, BlockType blockType, 
                        const glm::mat4& view, const glm::mat4& projection);
};
