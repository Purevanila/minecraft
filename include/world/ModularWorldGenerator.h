#pragma once
#include <vector>
#include <memory>
#include "world/Block.h"
#include "world/TerrainGenerator.h"
#include <glm/glm.hpp>

// Forward declarations
class Chunk;

/**
 * Context information passed to terrain features during generation
 */
struct TerrainContext {
    int chunkX, chunkZ;           // Chunk coordinates
    int terrainHeight;            // Base terrain height at this position
    bool isLakeArea;              // Whether this area should be a lake
    int waterLevel;               // Current water level
    glm::ivec3 worldPos;          // World position being generated
    
    TerrainContext(int cx, int cz, int height, bool lake, int water, const glm::ivec3& pos)
        : chunkX(cx), chunkZ(cz), terrainHeight(height), isLakeArea(lake), waterLevel(water), worldPos(pos) {}
};

/**
 * Base class for terrain features (trees, ores, structures, etc.)
 * Makes adding new world generation features much easier!
 */
class TerrainFeature {
public:
    virtual ~TerrainFeature() = default;
    
    // Generate this feature in the given chunk
    virtual void generate(Chunk& chunk, const TerrainContext& context) = 0;
    
    // Check if this feature should generate at the given position
    virtual bool shouldGenerate(const TerrainContext& context) const = 0;
    
    // Get feature name for debugging
    virtual std::string getName() const = 0;
    
    // Priority for generation order (higher = later)
    virtual int getPriority() const { return 0; }
};

/**
 * Modular World Generator
 * Features can be easily added and removed without touching core generation code
 */
class ModularWorldGenerator {
public:
    ModularWorldGenerator(unsigned int seed);
    
    // Add a terrain feature
    void addFeature(std::unique_ptr<TerrainFeature> feature);
    
    // Generate terrain for a chunk
    void generateChunk(Chunk& chunk);
    
    // Get base terrain height (delegates to original TerrainGenerator)
    int getTerrainHeight(int worldX, int worldZ) const;
    
    // Get block type for basic terrain (grass, dirt, stone, water)
    BlockType getBaseBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const;
    
    // Delegate methods to base generator for compatibility
    bool shouldGenerateLake(int worldX, int worldZ) const;
    BlockType getBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const;
    int getWaterLevel() const;
    int getTreeHeight() const;
    
private:
    std::unique_ptr<TerrainGenerator> m_baseGenerator;
    std::vector<std::unique_ptr<TerrainFeature>> m_features;
    
    // Sort features by priority
    void sortFeaturesByPriority();
    
    // Post-processing to fix floating water blocks
    void fixFloatingWaterBlocks(Chunk& chunk);
};
