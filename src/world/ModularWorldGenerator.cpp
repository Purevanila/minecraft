#include "world/ModularWorldGenerator.h"
#include "world/TerrainGenerator.h"
#include "world/Chunk.h"
#include "world/features/TreeFeature.h"
#include "world/WorldConfig.h"
#include <algorithm>
#include <iostream>

ModularWorldGenerator::ModularWorldGenerator(unsigned int seed) {
    m_baseGenerator = std::make_unique<TerrainGenerator>(seed);
}

void ModularWorldGenerator::addFeature(std::unique_ptr<TerrainFeature> feature) {
    std::cout << "Added terrain feature: " << feature->getName() << std::endl;
    
    // If this is a TreeFeature, give it access to the base generator
    if (feature->getName() == "TreeFeature") {
        TreeFeature* treeFeature = static_cast<TreeFeature*>(feature.get());
        treeFeature->setBaseGenerator(m_baseGenerator.get());
    }
    
    m_features.push_back(std::move(feature));
    sortFeaturesByPriority();
}

void ModularWorldGenerator::generateChunk(Chunk& chunk) {
    glm::ivec2 chunkPos = chunk.getPosition();
    
    // ⚡ PERFORMANCE: Pre-calculate terrain data for entire chunk
    int heightMap[CHUNK_SIZE][CHUNK_SIZE];
    bool lakeMap[CHUNK_SIZE][CHUNK_SIZE];
    int maxHeights[CHUNK_SIZE][CHUNK_SIZE];
    int waterLevel = m_baseGenerator->getWaterLevel();
    
    // Batch calculate all terrain properties
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = chunkPos.x * CHUNK_SIZE + x;
            int worldZ = chunkPos.y * CHUNK_SIZE + z;
            
            heightMap[x][z] = m_baseGenerator->getTerrainHeight(worldX, worldZ);
            lakeMap[x][z] = m_baseGenerator->shouldGenerateLake(worldX, worldZ);
            
            // Determine maximum height to generate
            if (lakeMap[x][z]) {
                maxHeights[x][z] = std::min(waterLevel, CHUNK_HEIGHT - 1);
            } else {
                maxHeights[x][z] = std::min(heightMap[x][z] + 10, CHUNK_HEIGHT - 1);
            }
        }
    }
    
    // Generate base terrain using pre-calculated data
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = chunkPos.x * CHUNK_SIZE + x;
            int worldZ = chunkPos.y * CHUNK_SIZE + z;
            int terrainHeight = heightMap[x][z];
            bool isLake = lakeMap[x][z];
            int maxY = maxHeights[x][z];
            
            // Generate base terrain first
            for (int y = 0; y <= maxY; ++y) {
                // Get base block type
                BlockType blockType = m_baseGenerator->getBlockType(worldX, y, worldZ, terrainHeight);
                
                // ⚡ ULTRA-FAST: Skip air blocks entirely for massive performance
                if (blockType == BlockType::AIR) continue;
                
                // Use ultra-fast block setting
                chunk.setBlockFast(x, y, z, blockType);
            }
        }
    }
    
    // FEATURE GENERATION: Generate features after base terrain is complete
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = chunkPos.x * CHUNK_SIZE + x;
            int worldZ = chunkPos.y * CHUNK_SIZE + z;
            int terrainHeight = heightMap[x][z];
            bool isLake = lakeMap[x][z];
            
            // Only check for features at the surface level (1 block above terrain)
            int surfaceY = terrainHeight + 1;
            if (surfaceY < CHUNK_HEIGHT) {
                glm::ivec3 worldPos(worldX, surfaceY, worldZ);
                TerrainContext context(chunkPos.x, chunkPos.y, terrainHeight, isLake, waterLevel, worldPos);
                
                // Check each feature for this surface position
                for (auto& feature : m_features) {
                    if (feature->shouldGenerate(context)) {
                        feature->generate(chunk, context);
                        break; // First feature that wants to generate gets priority
                    }
                }
            }
        }
    }
    
    // POST-PROCESSING: Ensure all tree positions have complete trees
    // This will find any tree trunks without adequate leaves and fix them
    for (auto& feature : m_features) {
        if (feature->getName() == "TreeFeature") {
            TreeFeature* treeFeature = static_cast<TreeFeature*>(feature.get());
            treeFeature->ensureAllTreesGenerated(chunk);
            break;
        }
    }
    

    // fixFloatingWaterBlocks(chunk);
}

void ModularWorldGenerator::fixFloatingWaterBlocks(Chunk& chunk) {
    // Scan for water blocks that might be floating without proper support
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) { // Skip bottom and top
                BlockType currentBlock = chunk.getBlock(x, y, z);
                
                if (currentBlock == BlockType::WATER) {
                    // Check if this water block is properly supported
                    bool hasSupport = false;
                    
                    // Check block below
                    BlockType blockBelow = chunk.getBlock(x, y - 1, z);
                    if (blockBelow != BlockType::AIR && blockBelow != BlockType::WATER) {
                        hasSupport = true;
                    }
                    
                    // If no direct support, check if there's solid ground within 3 blocks down
                    if (!hasSupport) {
                        for (int checkY = y - 2; checkY >= std::max(0, y - 4) && checkY >= 0; --checkY) {
                            BlockType checkBlock = chunk.getBlock(x, checkY, z);
                            if (checkBlock != BlockType::AIR && checkBlock != BlockType::WATER) {
                                hasSupport = true;
                                break;
                            }
                        }
                    }
                    
                    // Check if surrounded by solid blocks on sides (contained water)
                    if (!hasSupport) {
                        int solidNeighbors = 0;
                        int totalNeighbors = 0;
                        
                        // Check 4 horizontal neighbors
                        int neighbors[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                        for (int i = 0; i < 4; ++i) {
                            int nx = x + neighbors[i][0];
                            int nz = z + neighbors[i][1];
                            
                            if (nx >= 0 && nx < CHUNK_SIZE && nz >= 0 && nz < CHUNK_SIZE) {
                                totalNeighbors++;
                                BlockType neighborBlock = chunk.getBlock(nx, y, nz);
                                if (neighborBlock != BlockType::AIR && neighborBlock != BlockType::WATER) {
                                    solidNeighbors++;
                                }
                            }
                        }
                        
                        // If most neighbors are solid, water is contained
                        if (totalNeighbors > 0 && solidNeighbors >= totalNeighbors / 2) {
                            hasSupport = true;
                        }
                    }
                    
                    // Remove floating water blocks
                    if (!hasSupport) {
                        chunk.setBlock(x, y, z, BlockType::AIR);
                    }
                }
            }
        }
    }
}

int ModularWorldGenerator::getTerrainHeight(int worldX, int worldZ) const {
    return m_baseGenerator->getTerrainHeight(worldX, worldZ);
}

BlockType ModularWorldGenerator::getBaseBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    return m_baseGenerator->getBlockType(worldX, worldY, worldZ, surfaceHeight);
}

bool ModularWorldGenerator::shouldGenerateLake(int worldX, int worldZ) const {
    return m_baseGenerator->shouldGenerateLake(worldX, worldZ);
}

BlockType ModularWorldGenerator::getBlockType(int worldX, int worldY, int worldZ, int surfaceHeight) const {
    return m_baseGenerator->getBlockType(worldX, worldY, worldZ, surfaceHeight);
}

int ModularWorldGenerator::getWaterLevel() const {
    return m_baseGenerator->getParams().waterLevel;
}

int ModularWorldGenerator::getTreeHeight() const {
    return m_baseGenerator->getParams().treeHeight;
}

void ModularWorldGenerator::sortFeaturesByPriority() {
    std::sort(m_features.begin(), m_features.end(),
        [](const std::unique_ptr<TerrainFeature>& a, const std::unique_ptr<TerrainFeature>& b) {
            return a->getPriority() < b->getPriority();
        });
}
