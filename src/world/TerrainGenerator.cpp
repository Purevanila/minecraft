#include "world/TerrainGenerator.h"
#include "world/WorldConfig.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// External declaration for global world config
extern WorldConfig g_worldConfig;

TerrainGenerator::TerrainGenerator(unsigned int seed) 
    : m_heightNoise(seed)
    , m_detailNoise(seed + 1000)     // Different seed for detail
    , m_caveNoise(seed + 2000)       // Different seed for caves
    , m_lakeNoise(seed + 3000)       // Different seed for lakes
    , m_plainsNoise(seed + 5000)     // Different seed for plains
    , m_treeNoise(seed + 4000) {     // Different seed for trees
}

int TerrainGenerator::getTerrainHeight(int worldX, int worldZ) const {
    double heightValue = getHeightNoise(worldX, worldZ);
    
    // Convert noise (-1 to 1) to actual height with Minecraft-like distribution
    int height = static_cast<int>(m_params.heightOffset + heightValue * m_params.heightScale);
    
    // Apply plains flattening if in a plains area
    if (shouldGeneratePlains(worldX, worldZ)) {
        double plainsInfluence = getPlainsInfluence(worldX, worldZ);
        // Flatten terrain towards a target height (slightly above water level)
        int targetHeight = m_params.waterLevel + 3; // Flat plains just above water
        height = static_cast<int>(height * (1.0 - plainsInfluence) + targetHeight * plainsInfluence);
    }
    
    // Clamp to reasonable Minecraft-like height range first
    height = std::max(25, std::min(height, 55));
    
    // Only slightly lower terrain in lake areas to create shallow basins
    if (shouldGenerateLake(worldX, worldZ)) {
        height = std::max(height - 5, m_params.waterLevel - 4); // Shallow lakes, max 4 blocks deep
    }
    
    return height;
}

BlockType TerrainGenerator::getBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    // Check if should place water 
    bool isInLake = shouldGenerateLake(worldX, worldZ);
    int waterLevel = m_params.waterLevel;
    
    // Fill lake areas with water up to water level, with additional validation
    if (isInLake && worldY > surfaceHeight && worldY <= waterLevel) {
        // Additional safety check: make sure we're not creating floating water
        // Ensure the water has solid ground beneath it (within reasonable depth)
        int depthCheck = worldY - 1;
        while (depthCheck > surfaceHeight && depthCheck > worldY - 8) { // Check up to 8 blocks down
            if (depthCheck <= surfaceHeight) {
                break; // Found solid ground, water is safe to place
            }
            depthCheck--;
        }
        
        // Only place water if we're not too far above solid ground
        if (worldY - surfaceHeight <= 6) { // Max 6 blocks of water depth
            return BlockType::WATER;
        }
    }
    
    // Air above water level or above surface
    if (worldY > surfaceHeight && (!isInLake || worldY > waterLevel)) {
        // Check for tree stem placement ON TOP of grass surfaces (not in lakes)
        // Place oak logs from surfaceHeight+1 to surfaceHeight+4 (4 blocks tall)
        if (!isInLake && worldY >= surfaceHeight + 1 && worldY <= surfaceHeight + m_params.treeHeight) {
            if (shouldGenerateTree(worldX, worldZ)) {
                return BlockType::OAK_LOG;
            }
        }
        
        return BlockType::AIR;
    }
    
    // Surface layer - grass on top (but not under lakes)
    if (worldY == surfaceHeight) {
        if (isInLake) {
            return BlockType::DIRT; // Lake bottom is dirt
        } else {
            return BlockType::GRASS; // Normal surface is grass (don't replace with oak)
        }
    }
    
    // Dirt layer below surface
    if (worldY > surfaceHeight - m_params.dirtDepth && worldY < surfaceHeight) {
        // Check if gravel should be placed near lakes
        if (shouldGenerateGravel(worldX, worldY, worldZ, surfaceHeight)) {
            return BlockType::GRAVEL;
        }
        return BlockType::DIRT;
    }
    
    // Stone layer below dirt
    if (worldY > surfaceHeight - m_params.dirtDepth - m_params.stoneDepth) {
        // Also check for gravel in the upper stone layer near lakes
        if (shouldGenerateGravel(worldX, worldY, worldZ, surfaceHeight)) {
            return BlockType::GRAVEL;
        }
        return BlockType::STONE;
    }
    
    // Bedrock at the very bottom
    return BlockType::STONE;
}

double TerrainGenerator::getHeightNoise(double x, double z) const {
    // Use efficient FBM for main terrain shape
    double baseHeight = m_heightNoise.fbm(
        x * m_params.frequency, 
        z * m_params.frequency, 
        m_params.octaves, 
        m_params.persistence,
        2.0  // Classic lacunarity
    );
    
    // Add ridged mountains ONLY in specific areas (saves 90% of calculations)
    double ridgeContribution = 0.0;
    // Use simple noise to determine if this area should have ridges
    double ridgeSelector = m_heightNoise.noise(x * 0.003, z * 0.003);  // Very low frequency
    if (ridgeSelector > 0.3) {  // Only ~30% of terrain gets ridges
        double ridgeHeight = m_heightNoise.ridgedNoise(
            x * m_params.frequency * 0.5,
            z * m_params.frequency * 0.5,
            2,  // Fewer octaves for speed
            0.6
        );
        ridgeContribution = ridgeHeight * 0.2;  // Subtle ridge effect
    }
    
    // Add fast detail layer
    double detailHeight = m_detailNoise.octaveNoise(
        x * m_params.frequency * 3.0,
        z * m_params.frequency * 3.0,
        2,
        0.3
    );
    
    // Combine: base + selective ridges + detail
    return baseHeight * 0.75 + ridgeContribution + detailHeight * 0.15;
}

int TerrainGenerator::getBaseHeight(int worldX, int worldZ) const {
    double heightValue = getHeightNoise(worldX, worldZ);
    
    // Convert noise (-1 to 1) to actual height with Minecraft-like distribution
    int height = static_cast<int>(m_params.heightOffset + heightValue * m_params.heightScale);
    
    // Apply plains flattening if in a plains area
    if (shouldGeneratePlains(worldX, worldZ)) {
        double plainsInfluence = getPlainsInfluence(worldX, worldZ);
        // Flatten terrain towards a target height (slightly above water level)
        int targetHeight = m_params.waterLevel + 3; // Flat plains just above water
        height = static_cast<int>(height * (1.0 - plainsInfluence) + targetHeight * plainsInfluence);
    }
    
    // Clamp to reasonable Minecraft-like height range
    height = std::max(25, std::min(height, 55));
    
    return height;
}

bool TerrainGenerator::shouldGenerateCave(int x, int y, int z) const {
 // uhm future cave nigga?
    
    return false;
}

bool TerrainGenerator::shouldGenerateLake(int worldX, int worldZ) const {
    // Use simpler, faster noise for lake generation
    double lakeNoise = m_lakeNoise.octaveNoise(
        worldX * m_params.lakeFrequency, 
        worldZ * m_params.lakeFrequency, 
        3,      // Fewer octaves for speed
        0.6     // Higher persistence for defined edges
    );
    
    // Lakes form where noise is above threshold
    bool shouldGenerate = lakeNoise > m_params.lakeThreshold;
    
    // Enhanced boundary check: ensure lake has proper containment
    if (shouldGenerate) {
        // Check if this position is well-supported by checking nearby terrain heights
        int centerHeight = getBaseHeight(worldX, worldZ);
        
        // More comprehensive basin check with closer sampling
        int surroundingChecks = 0;
        int higherNeighbors = 0;
        int significantlyHigher = 0;  // Count neighbors significantly higher
        
        // Check in a cross pattern plus diagonals for better containment
        std::vector<std::pair<int, int>> checkPositions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1},    // Cardinal directions
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1},  // Diagonals
            {-2, 0}, {2, 0}, {0, -2}, {0, 2}     // Extended cardinal
        };
        
        for (const auto& offset : checkPositions) {
            int checkX = worldX + offset.first;
            int checkZ = worldZ + offset.second;
            
            int neighborHeight = getBaseHeight(checkX, checkZ);
            surroundingChecks++;
            
            // Count neighbors that can contain water
            if (neighborHeight >= centerHeight) {
                higherNeighbors++;
            }
            // Count neighbors significantly higher (good containment)
            if (neighborHeight >= centerHeight + 2) {
                significantlyHigher++;
            }
        }
        
        // Strict containment requirements to prevent leaks
        if (surroundingChecks > 0) {
            float containmentRatio = static_cast<float>(higherNeighbors) / surroundingChecks;
            float strongContainment = static_cast<float>(significantlyHigher) / surroundingChecks;
            
            // Require at least 70% containment and some strong containment
            if (containmentRatio < 0.7f || strongContainment < 0.2f) {
                return false; // Not enough containment, skip this lake position
            }
        }
    }
    
    return shouldGenerate;
}

bool TerrainGenerator::shouldGenerateTree(int worldX, int worldZ) const {
    // Use simple octave noise for better performance
    double treeNoise = m_treeNoise.octaveNoise(
        worldX * m_params.treeFrequency, 
        worldZ * m_params.treeFrequency, 
        2,      // Only 2 octaves for speed
        0.5     // Moderate persistence
    );
    
    // Only proceed if noise is above threshold
    if (treeNoise <= m_params.treeThreshold) {
        return false;
    }
    
    // Add spacing constraint: only allow trees every few blocks to prevent lines
    // Use simple modulo check with some randomness
    int spacing = 4; // Back to 4 blocks minimum spacing
    if ((worldX % spacing == 0) && (worldZ % spacing == 0)) {
        // Add some randomness based on world coordinates
        unsigned int hash = (worldX * 73856093) ^ (worldZ * 19349663);
        return (hash % 3) == 0; // Back to 1 in 3 chance for more scattered trees
    }
    
    return false;
}

bool TerrainGenerator::shouldGeneratePlains(int worldX, int worldZ) const {
    // Check if plains are enabled in the global config
    if (!g_worldConfig.terrain.plains.enabled) {
        return false;
    }
    
    // Use noise to determine if this area should be plains
    double plainsNoise = m_plainsNoise.octaveNoise(
        worldX * g_worldConfig.terrain.plains.frequency,
        worldZ * g_worldConfig.terrain.plains.frequency,
        3,      // 3 octaves for natural variation
        0.5     // Moderate persistence
    );
    
    return plainsNoise > g_worldConfig.terrain.plains.threshold;
}

double TerrainGenerator::getPlainsInfluence(int worldX, int worldZ) const {
    // Calculate distance-based influence within plains areas
    double plainsNoise = m_plainsNoise.octaveNoise(
        worldX * g_worldConfig.terrain.plains.frequency,
        worldZ * g_worldConfig.terrain.plains.frequency,
        3,
        0.5
    );
    
    // Convert noise to influence strength (0.0 to flatnessStrength)
    double influence = (plainsNoise - g_worldConfig.terrain.plains.threshold) / 
                      (1.0 - g_worldConfig.terrain.plains.threshold);
    influence = std::max(0.0, std::min(1.0, influence)); // Clamp to 0-1
    
    return influence * g_worldConfig.terrain.plains.flatnessStrength;
}

bool TerrainGenerator::shouldGenerateGravel(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    // Only generate gravel near the surface and slightly underground
    if (worldY > surfaceHeight + 1 || worldY < surfaceHeight - 4) {
        return false;
    }
    
    // Check distance to nearest lake with more efficient search
    float minDistance = 999.0f;
    int searchRadius = 6; // Slightly larger search radius
    
    // Use a more efficient sampling pattern (cross + some diagonals)
    std::vector<std::pair<int, int>> searchPattern;
    for (int r = 1; r <= searchRadius; r++) {
        // Add cardinal directions at each radius
        searchPattern.push_back({r, 0});
        searchPattern.push_back({-r, 0});
        searchPattern.push_back({0, r});
        searchPattern.push_back({0, -r});
        
        // Add some diagonal points for better coverage
        if (r <= 4) {
            searchPattern.push_back({r, r});
            searchPattern.push_back({-r, r});
            searchPattern.push_back({r, -r});
            searchPattern.push_back({-r, -r});
        }
    }
    
    for (const auto& offset : searchPattern) {
        if (shouldGenerateLake(worldX + offset.first, worldZ + offset.second)) {
            float distance = sqrt(offset.first * offset.first + offset.second * offset.second);
            minDistance = std::min(minDistance, distance);
            if (minDistance <= 1.0f) break; // Early exit for close water
        }
    }
    
    // No gravel if too far from lakes
    if (minDistance > 5.0f) {
        return false;
    }
    
    // Create distance-based probability (closer to water = more gravel)
    float distanceFactor = 1.0f - (minDistance / 5.0f); // 1.0 at water edge, 0.0 at max distance
    distanceFactor = distanceFactor * distanceFactor; // Square for more dramatic falloff
    
    // Use multiple noise layers for more natural, patchy distribution
    double primaryGravelNoise = m_detailNoise.octaveNoise(
        worldX * 0.06,  // Lower frequency for larger patches
        worldZ * 0.06,
        3,      // 3 octaves for good detail
        0.65    // Higher persistence for more defined patches
    );
    
    // Secondary noise for fine texture within patches
    double textureNoise = m_lakeNoise.octaveNoise(
        worldX * 0.18,  // Higher frequency for fine details
        worldZ * 0.18,
        2,
        0.35
    );
    
    // Tertiary noise for breaking up regular patterns
    double breakupNoise = m_heightNoise.octaveNoise(
        worldX * 0.12,
        worldZ * 0.12,
        2,
        0.4
    );
    
    // Depth-based probability (more gravel closer to surface and slightly below)
    float depthFromSurface = std::abs(worldY - surfaceHeight);
    float depthFactor;
    if (depthFromSurface <= 1.0f) {
        depthFactor = 1.0f; // Maximum at surface level
    } else {
        depthFactor = std::max(0.0f, 1.0f - (depthFromSurface - 1.0f) / 3.0f);
    }
    
    // Combine all noise layers
    double combinedNoise = primaryGravelNoise * 0.5 + textureNoise * 0.3 + breakupNoise * 0.2;
    
    // Calculate dynamic threshold based on distance and depth
    double baseChance = 0.4 * distanceFactor * depthFactor;
    double threshold = 0.15 - baseChance;
    
    // Special zones for different gravel densities
    if (minDistance <= 1.5f) {
        // Very close to water - high chance of gravel
        if (worldY >= surfaceHeight - 1 && worldY <= surfaceHeight) {
            return combinedNoise > -0.2; // Very liberal threshold
        }
    } else if (minDistance <= 3.0f) {
        // Medium distance - moderate gravel patches
        return combinedNoise > threshold * 0.8;
    }
    
    // Far from water - sparse gravel
    return combinedNoise > threshold;
}
