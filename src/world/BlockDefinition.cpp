#include "world/BlockDefinition.h"
#include <iostream>

// Default fallback definition
const BlockDefinition BlockDefinitionRegistry::s_defaultDefinition = 
    BlockDefinition("unknown", "assets/textures/grass.png");

BlockDefinitionRegistry& BlockDefinitionRegistry::getInstance() {
    static BlockDefinitionRegistry instance;
    return instance;
}

void BlockDefinitionRegistry::registerBlock(BlockType type, const BlockDefinition& definition) {
    m_definitions[type] = definition;
    std::cout << "Registered block: " << definition.name << " -> " << definition.texturePath << std::endl;
}

const BlockDefinition& BlockDefinitionRegistry::getDefinition(BlockType type) const {
    auto it = m_definitions.find(type);
    if (it != m_definitions.end()) {
        return it->second;
    }
    return s_defaultDefinition;
}

BlockType BlockDefinitionRegistry::getDropType(BlockType type) const {
    const BlockDefinition& def = getDefinition(type);
    if (def.dropsSelf) {
        return type;  // Block drops itself
    } else {
        return def.dropType;  // Block drops specified type
    }
}

bool BlockDefinitionRegistry::hasDefinition(BlockType type) const {
    return m_definitions.find(type) != m_definitions.end();
}

std::vector<BlockType> BlockDefinitionRegistry::getAllBlockTypes() const {
    std::vector<BlockType> types;
    for (const auto& pair : m_definitions) {
        types.push_back(pair.first);
    }
    return types;
}

void BlockDefinitionRegistry::initializeDefaultBlocks() {
    // Air block
    BlockDefinition airDef("air", "");
    airDef.solid = false;
    airDef.transparent = true;
    registerBlock(BlockType::AIR, airDef);
    
    // Grass block
    BlockDefinition grassDef("grass", "assets/textures/grass.png");
    grassDef.hardness = 0.6f;
    grassDef.toolType = "shovel";
    grassDef.dropsSelf = false;  // Grass drops dirt, not grass
    grassDef.dropType = BlockType::DIRT;
    registerBlock(BlockType::GRASS, grassDef);
    
    // Dirt block
    BlockDefinition dirtDef("dirt", "assets/textures/grass.png");  // Using grass texture for now
    dirtDef.hardness = 0.5f;
    dirtDef.toolType = "shovel";
    dirtDef.dropsSelf = true;  // Dirt drops dirt
    dirtDef.dropType = BlockType::DIRT;
    registerBlock(BlockType::DIRT, dirtDef);
    
    // Stone block
    BlockDefinition stoneDef("stone", "assets/textures/stone.png");  // Using proper stone texture
    stoneDef.hardness = 1.5f;
    stoneDef.toolType = "pickaxe";
    stoneDef.requiresTool = true;
    stoneDef.needsSeparateMesh = true;  // Separate mesh for different texture
    stoneDef.dropsSelf = true;  // Stone drops stone
    stoneDef.dropType = BlockType::STONE;
    registerBlock(BlockType::STONE, stoneDef);
    
    // Water block
    BlockDefinition waterDef("water", "assets/textures/water.webp");
    waterDef.solid = false;
    waterDef.transparent = true;
    waterDef.liquid = true;
    waterDef.hardness = 0.0f;
    waterDef.needsSeparateMesh = true;
    registerBlock(BlockType::WATER, waterDef);
    
    // Oak log block
    BlockDefinition oakLogDef("oak_log", "assets/textures/oak.png");
    oakLogDef.hardness = 2.0f;
    oakLogDef.toolType = "axe";
    oakLogDef.needsSeparateMesh = true;  // Separate mesh for different texture
    oakLogDef.dropsSelf = true;  // Oak log drops oak log
    oakLogDef.dropType = BlockType::OAK_LOG;
    registerBlock(BlockType::OAK_LOG, oakLogDef);
    
    // Oak leaves block
    BlockDefinition oakLeavesDef("oak_leaves", "assets/textures/oakleave.png");
    oakLeavesDef.hardness = 0.2f;
    oakLeavesDef.toolType = "shears";
    oakLeavesDef.transparent = true;  // Leaves are partially transparent
    oakLeavesDef.needsSeparateMesh = true;  // Separate mesh for different texture
    oakLeavesDef.dropsSelf = true;  // Leaves drop leaves (when using shears)
    oakLeavesDef.dropType = BlockType::LEAVES;
    registerBlock(BlockType::LEAVES, oakLeavesDef);
    
    // Sand block
    BlockDefinition sandDef("sand", "assets/textures/sand.png");
    sandDef.hardness = 0.5f;
    sandDef.toolType = "shovel";
    sandDef.transparent = false;  // Sand is solid
    sandDef.needsSeparateMesh = true;  // Separate mesh for different texture
    sandDef.dropsSelf = true;  // Sand drops sand items
    sandDef.dropType = BlockType::SAND;  // Explicitly specify sand drop
    registerBlock(BlockType::SAND, sandDef);
    
    // Gravel block
    BlockDefinition gravelDef("gravel", "assets/textures/gravel.png");
    gravelDef.hardness = 0.6f;
    gravelDef.toolType = "shovel";
    gravelDef.transparent = false;  // Gravel is solid
    gravelDef.needsSeparateMesh = true;  // Separate mesh for different texture
    gravelDef.dropsSelf = true;  // Gravel drops gravel
    gravelDef.dropType = BlockType::GRAVEL;
    registerBlock(BlockType::GRAVEL, gravelDef);
    
    std::cout << "Initialized " << m_definitions.size() << " block definitions" << std::endl;
}
