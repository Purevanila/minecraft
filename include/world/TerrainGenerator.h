#pragma once
#include "world/PerlinNoise.h"
#include "world/Block.h"
#include <glm/glm.hpp>

/**
 * Terrain Generator - Creates Natural Minecraft-Style Worlds
 * 
 * This class uses Perlin noise to generate realistic terrain with:
 * - Rolling hills and valleys
 * - Different biome heights
 * - Natural-looking coastlines
 * - Varied but believable landscapes
 */
class TerrainGenerator {
public:
    TerrainGenerator(unsigned int seed = 12345);
    
    // Get the terrain height at any world coordinate
    int getTerrainHeight(int worldX, int worldZ) const;
    
    // Get the appropriate block type for a position
    BlockType getBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const;
    
    // Lake generation
    bool shouldGenerateLake(int worldX, int worldZ) const;
    int getWaterLevel() const { return m_params.waterLevel; }
    
    // Plains generation
    bool shouldGeneratePlains(int worldX, int worldZ) const;
    double getPlainsInfluence(int worldX, int worldZ) const;
    
    // Gravel generation near lakes
    bool shouldGenerateGravel(int worldX, int worldY, int worldZ, int surfaceHeight) const;
    
    // Tree generation
    bool shouldGenerateTree(int worldX, int worldZ) const;
    
    // Terrain generation parameters - Minecraft-style varied terrain
    struct TerrainParams {
        double heightScale = 15.0;      // Good variation for interesting terrain
        double heightOffset = 35.0;     // Nice base level with variety above and below
        double frequency = 0.02;        // Balanced frequency for natural-looking hills
        int octaves = 4;                // Good detail without too much noise
        double persistence = 0.4;       // Gentler octave influence (was 0.6)
        
        // Layer thicknesses
        int grassDepth = 1;             // How thick the grass layer is
        int dirtDepth = 3;              // How deep dirt goes (was 4)
        int stoneDepth = 30;            // How deep stone goes before bedrock
        
        // Water and lake generation
        int waterLevel = 32;            // Sea level - lakes form at this height
        double lakeFrequency = 0.015;   // Slightly reduced frequency for better quality lakes
        double lakeThreshold = 0.45;    // Higher threshold for fewer but better contained lakes
        
        // Tree generation
        double treeFrequency = 0.03;    // Moderate frequency for decent tree distribution
        double treeThreshold = 0.3;     // Slightly higher threshold for fewer trees
        int treeHeight = 4;             // Height of tree stems
    };
    
    void setParams(const TerrainParams& params) { m_params = params; }
    const TerrainParams& getParams() const { return m_params; }
    
private:
    PerlinNoise m_heightNoise;          // Primary terrain height
    PerlinNoise m_detailNoise;          // Fine detail noise
    PerlinNoise m_caveNoise;            // For future cave generation
    PerlinNoise m_lakeNoise;            // For lake generation
    PerlinNoise m_plainsNoise;          // For plains generation
    PerlinNoise m_treeNoise;            // For tree generation
    TerrainParams m_params;
    
    // Helper functions
    double getHeightNoise(double x, double z) const;
    int getBaseHeight(int worldX, int worldZ) const;  // Get height without lake modifications
    bool shouldGenerateCave(int x, int y, int z) const;
};
