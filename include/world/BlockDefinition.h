#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "world/Block.h"

/**
 * Block Definition System - Centralized block properties
 * Makes adding new blocks much easier by defining everything in one place
 */
struct BlockDefinition {
    std::string name;
    std::string texturePath;
    bool solid = true;
    bool transparent = false;
    bool liquid = false;
    float hardness = 1.0f;
    bool requiresTool = false;
    std::string toolType = "hand";
    
    // Drop behavior - what item this block drops when broken
    BlockType dropType = BlockType::AIR;  // Default: drops nothing
    bool dropsSelf = true;  // Most blocks drop themselves
    
    // Rendering properties
    bool needsSeparateMesh = false;  // For liquids, transparent blocks, etc.
    
    BlockDefinition() = default;
    BlockDefinition(const std::string& name, const std::string& texture)
        : name(name), texturePath(texture) {}
};

/**
 * Centralized Block Registry with data-driven definitions
 * No more scattered block registration across multiple files!
 */
class BlockDefinitionRegistry {
public:
    static BlockDefinitionRegistry& getInstance();
    
    // Register a block definition
    void registerBlock(BlockType type, const BlockDefinition& definition);
    
    // Get block definition
    const BlockDefinition& getDefinition(BlockType type) const;
    
    // Get what a block drops when broken
    BlockType getDropType(BlockType type) const;
    
    // Check if block exists
    bool hasDefinition(BlockType type) const;
    
    // Get all block types (for iteration)
    std::vector<BlockType> getAllBlockTypes() const;
    
    // Initialize all default blocks
    void initializeDefaultBlocks();
    
private:
    BlockDefinitionRegistry() = default;
    std::unordered_map<BlockType, BlockDefinition> m_definitions;
    
    // Fallback for undefined blocks
    static const BlockDefinition s_defaultDefinition;
};
