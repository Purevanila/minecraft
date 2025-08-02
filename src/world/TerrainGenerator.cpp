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
    , m_treeNoise(seed + 4000)       // Different seed for trees
    , m_continentNoise(seed + 6000)  // Different seed for continents
    , m_islandNoise(seed + 7000) {   // Different seed for island details
}

int TerrainGenerator::getTerrainHeight(int worldX, int worldZ) const {
    // Use continental noise to determine terrain type
    double continentalNoise = m_continentNoise.octaveNoise(
        worldX * 0.0008,  // Even lower frequency for smoother continents
        worldZ * 0.0008,
        3,                // Fewer octaves for smoother transitions
        0.5               // Lower persistence for less detail
    );
    
    // Ocean areas (low continental noise) should be much lower
    if (continentalNoise < -0.3) {
        // Deep ocean floor
        return m_params.waterLevel - 8;
    } else if (continentalNoise < -0.1) {
        // Shallow ocean/coastal areas
        return m_params.waterLevel - 3;
    }
    
    // Land areas - generate normal terrain
    double heightValue = getHeightNoise(worldX, worldZ);
    int height = static_cast<int>(m_params.heightOffset + heightValue * m_params.heightScale);
    
    // Apply plains flattening if in a plains area
    if (shouldGeneratePlains(worldX, worldZ)) {
        double plainsInfluence = getPlainsInfluence(worldX, worldZ);
        int targetHeight = m_params.waterLevel + 3; // Flat plains just above water
        height = static_cast<int>(height * (1.0 - plainsInfluence) + targetHeight * plainsInfluence);
    }
    
    // Create gentle shores near ocean boundaries
    if (continentalNoise < 0.4) {
        float shoreEffect = std::max(0.0f, (float)continentalNoise + 0.3f) / 0.7f; // 0 to 1
        int minShoreHeight = m_params.waterLevel + 2;
        height = static_cast<int>(height * shoreEffect + minShoreHeight * (1.0f - shoreEffect));
    }
    
    // Clamp to reasonable Minecraft-like height range
    height = std::max(10, std::min(height, 65));
    
    // Small lakes can still exist on continents
    if (shouldGenerateLake(worldX, worldZ) && continentalNoise > 0.1) {
        height = std::max(height - 3, m_params.waterLevel - 2); // Shallow inland lakes
    }
    
    return height;
}

BlockType TerrainGenerator::getBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    // Check if should place water 
    bool isInLake = shouldGenerateLake(worldX, worldZ);
    bool isInOcean = isInOceanArea(worldX, worldZ);
    int waterLevel = m_params.waterLevel;
    
    // Fill ALL underwater areas with water (oceans, lakes, etc.)
    if (worldY > surfaceHeight && worldY <= waterLevel) {
        // If terrain is below water level, fill with water
        return BlockType::WATER;
    }
    
    // Air above water level or above surface
    if (worldY > surfaceHeight && worldY > waterLevel) {
        // EXTREMELY STRICT tree placement: only on high, inland areas
        bool isInOcean = isInOceanArea(worldX, worldZ);
        if (!isInOcean && !isInLake && surfaceHeight >= waterLevel + 5) {
            // Use continental noise to determine if we're well inland
            double continentalNoise = m_continentNoise.octaveNoise(worldX * 0.0008, worldZ * 0.0008, 3, 0.5);
            
            // Only place trees on land that's very well inland - much stricter
            // Triple check we're not in ocean and require very high continental noise
            if (continentalNoise > 0.8 && !isInOceanArea(worldX, worldZ) && worldY >= surfaceHeight + 1 && worldY <= surfaceHeight + m_params.treeHeight) {
                if (shouldGenerateTree(worldX, worldZ)) {
                    return BlockType::OAK_LOG;
                }
            }
        }
        
        return BlockType::AIR;
    }
    
    // Surface layer - different blocks based on location
    if (worldY == surfaceHeight) {
        bool isInOcean = isInOceanArea(worldX, worldZ);
        bool isInLake = shouldGenerateLake(worldX, worldZ);
        
        if (isInOcean) {
            // Ocean floor - use sand for shallow areas, dirt for deep areas
            if (surfaceHeight < waterLevel - 3) {
                return BlockType::DIRT; // Deep ocean floor
            } else {
                return BlockType::SAND; // Shallow ocean floor
            }
        } else if (isInLake) {
            return BlockType::DIRT; // Lake bottom is dirt
        } else {
            // Land surface - create beaches all around landmasses for island feeling
            double continentalNoise = m_continentNoise.octaveNoise(worldX * 0.0008, worldZ * 0.0008, 3, 0.5);
            
            // Check for ocean in a larger radius to ensure beaches everywhere around islands
            bool isNearOcean = false;
            
            // Larger search radius to catch all coastlines
            for (int dx = -6; dx <= 6; dx++) {
                for (int dz = -6; dz <= 6; dz++) {
                    if (dx == 0 && dz == 0) continue;
                    
                    // Check if nearby area is ocean
                    if (isInOceanArea(worldX + dx, worldZ + dz)) {
                        isNearOcean = true;
                        break;
                    }
                }
                if (isNearOcean) break;
            }
            
            // Create beaches around all coastlines - more generous conditions
            if (isNearOcean && surfaceHeight <= waterLevel + 3 && continentalNoise > -0.2) {
                // Add some variation so not every coastal block is sand
                double beachVariation = m_lakeNoise.octaveNoise(worldX * 0.04, worldZ * 0.04, 2, 0.5);
                if (beachVariation > -0.2) {  // Most coastal areas get sand
                    return BlockType::SAND;
                }
            }
            
            // Default to grass for all other land areas
            return BlockType::GRASS;
        }
    }
    
    // Dirt layer below surface
    if (worldY > surfaceHeight - m_params.dirtDepth && worldY < surfaceHeight) {
        // Extend sand deeper in all coastal areas for island feel
        double continentalNoise = m_continentNoise.octaveNoise(worldX * 0.0008, worldZ * 0.0008, 3, 0.5);
        
        // Same larger search radius as surface
        bool isNearOcean = false;
        for (int dx = -6; dx <= 6; dx++) {
            for (int dz = -6; dz <= 6; dz++) {
                if (dx == 0 && dz == 0) continue;
                if (isInOceanArea(worldX + dx, worldZ + dz)) {
                    isNearOcean = true;
                    break;
                }
            }
            if (isNearOcean) break;
        }
        
        // Extend sand down in coastal areas - 2 blocks deep for better beaches
        if (isNearOcean && surfaceHeight <= waterLevel + 3 && continentalNoise > -0.2 && worldY > surfaceHeight - 2) {
            double beachVariation = m_lakeNoise.octaveNoise(worldX * 0.04, worldZ * 0.04, 2, 0.5);
            if (beachVariation > -0.2) {
                return BlockType::SAND;
            }
        }
        
        // Check for gravel near lakes, otherwise dirt
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

bool TerrainGenerator::isInOceanArea(int worldX, int worldZ) const {
    // Use continental noise to determine if this is ocean
    double continentalNoise = m_continentNoise.octaveNoise(
        worldX * 0.0008,  // Same scale as height generation
        worldZ * 0.0008,
        3,
        0.5
    );
    
    // Ocean areas have low continental noise values
    return continentalNoise < -0.1; // Consistent with height generation
}

float TerrainGenerator::getDistanceToOcean(int worldX, int worldZ) const {
    // Sample in a grid pattern to find nearest ocean
    float minDistance = 999.0f;
    int searchRadius = 50; // Search up to 50 blocks
    
    for (int dx = -searchRadius; dx <= searchRadius; dx += 5) {
        for (int dz = -searchRadius; dz <= searchRadius; dz += 5) {
            if (isInOceanArea(worldX + dx, worldZ + dz)) {
                float distance = std::sqrt(dx * dx + dz * dz);
                minDistance = std::min(minDistance, distance);
                
                // Early exit if we found ocean very close
                if (minDistance < 10.0f) {
                    return minDistance;
                }
            }
        }
    }
    
    return minDistance;
}

bool TerrainGenerator::shouldGenerateSand(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    // Generate sand in coastal areas and deserts
    
    // Only generate sand near the surface and slightly underground
    if (worldY > surfaceHeight + 1 || worldY < surfaceHeight - 2) {
        return false;
    }
    
    // Use continental noise to determine proximity to ocean
    double continentalNoise = m_continentNoise.octaveNoise(worldX * 0.001, worldZ * 0.001, 4, 0.6);
    
    // Ocean floor gets sand in shallow areas
    if (continentalNoise < -0.1 && continentalNoise > -0.3) {
        return true; // Shallow ocean floor
    }
    
    // Beach sand - close to ocean (low continental noise but not ocean itself)
    if (continentalNoise > -0.1 && continentalNoise < 0.3) {
        double beachNoise = m_lakeNoise.octaveNoise(
            worldX * 0.08,
            worldZ * 0.08,
            2,
            0.5
        );
        
        // More sand closer to ocean
        float proximityFactor = 1.0f - ((continentalNoise + 0.1f) / 0.4f);
        return beachNoise > (0.2 - proximityFactor * 0.3);
    }
    
    // Desert sand - in dry areas well inland
    if (continentalNoise > 0.6) {
        double desertNoise = m_plainsNoise.octaveNoise(
            worldX * 0.05,
            worldZ * 0.05,
            3,
            0.6
        );
        
        // Additional noise for desert patches
        double patchNoise = m_detailNoise.octaveNoise(
            worldX * 0.15,
            worldZ * 0.15,
            2,
            0.4
        );
        
        double combinedDesertNoise = desertNoise * 0.7 + patchNoise * 0.3;
        return combinedDesertNoise > 0.4; // Threshold for desert areas
    }
    
    return false;
}

bool TerrainGenerator::isOnContinent(int worldX, int worldZ) const {
    // Use very low frequency noise for large continental shapes
    double continentNoise = m_continentNoise.octaveNoise(
        worldX * 0.003,  // Very low frequency for continent-scale features
        worldZ * 0.003,
        4,               // Multiple octaves for complex coastlines
        0.65             // High persistence for smoother transitions
    );
    
    // Add island detail noise for more interesting coastlines
    double islandDetail = m_islandNoise.octaveNoise(
        worldX * 0.008,  // Medium frequency for island-scale features
        worldZ * 0.008,
        3,
        0.5
    );
    
    // Combine noises - continent noise dominates, island detail adds complexity
    double combinedNoise = continentNoise * 0.8 + islandDetail * 0.2;
    
    // Threshold for land vs ocean - adjust this to control land/ocean ratio
    return combinedNoise > -0.1;  // Slightly biased toward land
}
