#include "world/Block.h"
#include <stdexcept>

Block::Block(BlockType type) : m_type(type) {
    // Get properties from registry
    m_properties = BlockRegistry::getInstance().getProperties(type);
}

bool Block::shouldRenderFace(BlockFace face, const Block& neighbor) const {
    // Don't render faces adjacent to solid, opaque blocks
    if (neighbor.isSolid() && !neighbor.isTransparent()) {
        return false;
    }
    
    // Always render if neighbor is air
    if (neighbor.isAir()) {
        return true;
    }
    
    // Render if this block is transparent and neighbor is different
    if (isTransparent() && neighbor.getType() != getType()) {
        return true;
    }
    
    return false;
}

std::string Block::getTexture(BlockFace face) const {
    auto it = m_properties.textures.find(face);
    if (it != m_properties.textures.end()) {
        return it->second;
    }
    // Fallback to front texture if specific face not found
    return m_properties.textures.at(BlockFace::FRONT);
}

// BlockRegistry implementation
BlockRegistry& BlockRegistry::getInstance() {
    static BlockRegistry instance;
    return instance;
}

void BlockRegistry::registerBlock(BlockType type, const BlockProperties& properties) {
    m_blockProperties[type] = properties;
}

std::unique_ptr<Block> BlockRegistry::createBlock(BlockType type) const {
    return std::make_unique<Block>(type);
}

const BlockProperties& BlockRegistry::getProperties(BlockType type) const {
    auto it = m_blockProperties.find(type);
    if (it != m_blockProperties.end()) {
        return it->second;
    }
    
    // Return air properties as fallback
    static BlockProperties airProperties;
    airProperties.solid = false;
    airProperties.transparent = true;
    airProperties.name = "air";
    return airProperties;
}

void BlockRegistry::initializeDefaultBlocks() {
    // Air block
    BlockProperties airProps;
    airProps.solid = false;
    airProps.transparent = true;
    airProps.name = "air";
    registerBlock(BlockType::AIR, airProps);
    
    // Stone block
    BlockProperties stoneProps("stone");
    stoneProps.name = "stone";
    stoneProps.hardness = 1.5f;
    registerBlock(BlockType::STONE, stoneProps);
    
    // Grass block (using grass.png texture for all faces)
    BlockProperties grassProps("grass.png");
    grassProps.name = "grass";
    grassProps.hardness = 0.6f;
    registerBlock(BlockType::GRASS, grassProps);
    
    // Dirt block
    BlockProperties dirtProps("dirt");
    dirtProps.name = "dirt";
    dirtProps.hardness = 0.5f;
    registerBlock(BlockType::DIRT, dirtProps);
    
    // Wood block
    BlockProperties woodProps("wood_top", "wood_side", "wood_top");
    woodProps.name = "wood";
    woodProps.hardness = 2.0f;
    registerBlock(BlockType::WOOD, woodProps);
    
    // Leaves block
    BlockProperties leavesProps("leaves");
    leavesProps.name = "leaves";
    leavesProps.transparent = true;
    leavesProps.hardness = 0.2f;
    registerBlock(BlockType::LEAVES, leavesProps);
    
    // Sand block
    BlockProperties sandProps("sand");
    sandProps.name = "sand";
    sandProps.hardness = 0.5f;
    registerBlock(BlockType::SAND, sandProps);
    
    // Water block
    BlockProperties waterProps("water");
    waterProps.name = "water";
    waterProps.solid = false;
    waterProps.transparent = true;
    waterProps.liquid = true;
    waterProps.hardness = 0.0f;
    registerBlock(BlockType::WATER, waterProps);
    
    // Oak log block (tree trunk)
    BlockProperties oakLogProps("oak.png");
    oakLogProps.name = "oak_log";
    oakLogProps.hardness = 2.0f;
    registerBlock(BlockType::OAK_LOG, oakLogProps);
    
    // Gravel block
    BlockProperties gravelProps("gravel.png");
    gravelProps.name = "gravel";
    gravelProps.hardness = 0.6f;
    registerBlock(BlockType::GRAVEL, gravelProps);
}
