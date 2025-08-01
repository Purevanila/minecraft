#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class Texture;
class Mesh;

enum class BlockType : uint16_t {
    AIR = 0,
    STONE = 1,
    GRASS = 2,
    DIRT = 3,
    WOOD = 4,
    LEAVES = 5,
    SAND = 6,
    WATER = 7,
    OAK_LOG = 8,  // Tree trunk/stem blocks
    GRAVEL = 9,   // Gravel blocks for lake shores
    // Future blocks can be added here
    CUSTOM_START = 1000  // Reserved range for custom blocks
};

enum class BlockFace : uint8_t {
    FRONT = 0,
    BACK = 1,
    LEFT = 2,
    RIGHT = 3,
    TOP = 4,
    BOTTOM = 5
};

struct BlockProperties {
    bool solid = true;
    bool transparent = false;
    bool liquid = false;
    bool breakable = true;
    float hardness = 1.0f;
    float lightLevel = 0.0f;  // 0-15 for light emission
    std::string name;
    
    // Texture mapping for each face
    std::unordered_map<BlockFace, std::string> textures;
    
    // Default constructor
    BlockProperties() = default;
    
    // Convenience constructor for uniform texture
    BlockProperties(const std::string& uniformTexture, bool isSolid = true) 
        : solid(isSolid), name(uniformTexture) {
        for (int i = 0; i < 6; ++i) {
            textures[static_cast<BlockFace>(i)] = uniformTexture;
        }
    }
    
    // Constructor for different top/side/bottom textures (like grass)
    BlockProperties(const std::string& top, const std::string& side, const std::string& bottom, bool isSolid = true)
        : solid(isSolid) {
        textures[BlockFace::TOP] = top;
        textures[BlockFace::BOTTOM] = bottom;
        textures[BlockFace::FRONT] = side;
        textures[BlockFace::BACK] = side;
        textures[BlockFace::LEFT] = side;
        textures[BlockFace::RIGHT] = side;
    }
};

class Block {
public:
    Block(BlockType type = BlockType::AIR);
    virtual ~Block() = default;
    
    // Core properties
    BlockType getType() const { return m_type; }
    const BlockProperties& getProperties() const { return m_properties; }
    
    // State management
    virtual void update(float deltaTime) {}
    virtual bool canPlace(const glm::ivec3& position) const { return true; }
    virtual void onPlace(const glm::ivec3& position) {}
    virtual void onBreak(const glm::ivec3& position) {}
    virtual void onInteract(const glm::ivec3& position) {}
    
    // Rendering
    virtual bool shouldRenderFace(BlockFace face, const Block& neighbor) const;
    virtual std::string getTexture(BlockFace face) const;
    
    // Utility
    bool isAir() const { return m_type == BlockType::AIR; }
    bool isSolid() const { return m_properties.solid; }
    bool isTransparent() const { return m_properties.transparent; }
    bool isLiquid() const { return m_properties.liquid; }
    
protected:
    BlockType m_type;
    BlockProperties m_properties;
    
    // Future-proof: allow blocks to store custom data
    std::unordered_map<std::string, float> m_customData;
};

// Block registry for managing all block types
class BlockRegistry {
public:
    static BlockRegistry& getInstance();
    
    void registerBlock(BlockType type, const BlockProperties& properties);
    std::unique_ptr<Block> createBlock(BlockType type) const;
    const BlockProperties& getProperties(BlockType type) const;
    
    // Initialize default blocks
    void initializeDefaultBlocks();
    
private:
    BlockRegistry() = default;
    std::unordered_map<BlockType, BlockProperties> m_blockProperties;
};
