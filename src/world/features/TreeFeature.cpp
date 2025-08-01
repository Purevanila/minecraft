#include "world/features/TreeFeature.h"
#include "world/Chunk.h"
#include <iostream>
#include <algorithm>

TreeFeature::TreeFeature(unsigned int seed) 
    : m_treeNoise(seed + 4000) { // Different seed from terrain generation
}

bool TreeFeature::shouldGenerate(const TerrainContext& context) const {
    // Don't generate trees in lakes or below/at water level
    if (context.isLakeArea || context.worldPos.y <= context.terrainHeight) {
        return false;
    }
    
    // Only generate trees at the exact base position (1 block above terrain)
    if (context.worldPos.y != context.terrainHeight + 1) {
        return false;
    }
    
    // Use noise to determine tree areas
    double treeNoise = m_treeNoise.octaveNoise(
        context.worldPos.x * m_params.frequency,
        context.worldPos.z * m_params.frequency,
        2, 0.5
    );
    
    if (treeNoise <= m_params.threshold) {
        return false;
    }
    
    // Check spacing constraint
    return checkSpacing(context);
}

void TreeFeature::generate(Chunk& chunk, const TerrainContext& context) {
    // Convert world position to chunk-local position
    int localX = context.worldPos.x - (context.chunkX * CHUNK_SIZE);
    int localZ = context.worldPos.z - (context.chunkZ * CHUNK_SIZE);
    
    // Validate chunk bounds
    if (localX < 0 || localX >= CHUNK_SIZE || localZ < 0 || localZ >= CHUNK_SIZE) {
        return;
    }

    // Generate complete tree at the base position
    int treeHeight = getTreeHeight(context);
    int treeStartY = context.terrainHeight + 1;
    
    // Always generate the complete tree when this method is called
    generateCompleteTree(chunk, localX, treeStartY, localZ, treeHeight, context);
}

// Add a post-processing method to ensure ALL possible tree positions have trees
void TreeFeature::ensureAllTreesGenerated(Chunk& chunk) const {
    // NUCLEAR OPTION: Every single tree trunk gets leaves, no exceptions
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            
            // Look for ANY oak log and ensure leaves around it
            for (int y = 1; y < CHUNK_HEIGHT - 3; y++) {
                if (chunk.getBlock(x, y, z) == BlockType::OAK_LOG) {
                    
                    // Check if this is the base of a tree
                    bool isTreeBase = (y == 0 || chunk.getBlock(x, y - 1, z) != BlockType::OAK_LOG);
                    
                    if (isTreeBase) {
                        // Find the full height of this tree
                        int treeHeight = 1;
                        for (int checkY = y + 1; checkY < CHUNK_HEIGHT; checkY++) {
                            if (chunk.getBlock(x, checkY, z) == BlockType::OAK_LOG) {
                                treeHeight++;
                            } else {
                                break;
                            }
                        }
                        
                        // ALWAYS generate a canopy for EVERY tree - no threshold checks
                        // The previous approach was too lenient
                        int leafStartY = y + treeHeight - 2;
                        generateCompleteCanopy(chunk, x, leafStartY, z, treeHeight);
                        
                        // Skip ahead to avoid processing this tree multiple times
                        y += treeHeight;
                    }
                }
            }
        }
    }
}

void TreeFeature::generateCompleteCanopy(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight) const {
    // BULLETPROOF canopy generation - every tree WILL have leaves
    
    // Layer 1: Top of tree (crown) - try multiple levels if needed
    for (int topOffset = 0; topOffset <= 3; topOffset++) {
        int topY = leafStartY + 2 - topOffset;
        if (topY >= 0 && topY < CHUNK_HEIGHT) {
            placeLeafIfValid(chunk, centerX, topY, centerZ);
            break; // Stop once we place the top leaf
        }
    }
    
    // Layer 2: Upper crown (plus pattern) - also try multiple levels
    for (int upperOffset = 0; upperOffset <= 2; upperOffset++) {
        int upperY = leafStartY + 1 - upperOffset;
        if (upperY >= 0 && upperY < CHUNK_HEIGHT) {
            placeLeafIfValid(chunk, centerX, upperY, centerZ);
            placeLeafIfValid(chunk, centerX - 1, upperY, centerZ);
            placeLeafIfValid(chunk, centerX + 1, upperY, centerZ);
            placeLeafIfValid(chunk, centerX, upperY, centerZ - 1);
            placeLeafIfValid(chunk, centerX, upperY, centerZ + 1);
            break; // Stop once we place the upper layer
        }
    }
    
    // Layer 3 & 4: Main canopy layers (full 5x5) - try multiple Y levels
    for (int layerOffset = -1; layerOffset <= 2; layerOffset++) {
        int layerY = leafStartY + layerOffset;
        if (layerY < 0 || layerY >= CHUNK_HEIGHT) continue;
        
        // Generate a full 5x5 square of leaves
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                // Skip the center trunk position for the main layer only
                if (dx == 0 && dz == 0 && layerOffset == 0) continue;
                
                // Place ALL leaves, including corners
                placeLeafIfValid(chunk, centerX + dx, layerY, centerZ + dz);
            }
        }
    }
    
    // Layer 5: Extended bottom layer (larger area)
    for (int bottomOffset = 0; bottomOffset <= 1; bottomOffset++) {
        int bottomY = leafStartY - 1 - bottomOffset;
        if (bottomY >= 0 && bottomY < CHUNK_HEIGHT) {
            for (int dx = -2; dx <= 2; dx++) {
                for (int dz = -2; dz <= 2; dz++) {
                    if (dx == 0 && dz == 0) continue; // Skip trunk
                    
                    // Skip far corners for this layer
                    if (abs(dx) == 2 && abs(dz) == 2) continue;
                    
                    placeLeafIfValid(chunk, centerX + dx, bottomY, centerZ + dz);
                }
            }
        }
    }
    
    // Emergency backup: If all else fails, brute force place leaves anywhere nearby
    // This ensures NO tree is left bare
    for (int emergencyY = leafStartY - 2; emergencyY <= leafStartY + 3; emergencyY++) {
        if (emergencyY < 0 || emergencyY >= CHUNK_HEIGHT) continue;
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue; // Skip trunk
                placeLeafIfValid(chunk, centerX + dx, emergencyY, centerZ + dz);
            }
        }
    }
}

void TreeFeature::placeLeafIfValid(Chunk& chunk, int x, int y, int z) const {
    // Ultra-aggressive leaf placement that finds alternative spots when blocked
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT) {
        BlockType current = chunk.getBlock(x, y, z);
        
        // Place leaves if the space is empty OR already has leaves
        if (current == BlockType::AIR || current == BlockType::LEAVES) {
            chunk.setBlock(x, y, z, BlockType::LEAVES);
            return; // Success!
        }
        
        // If there's a trunk here, try to place leaves nearby instead
        if (current == BlockType::OAK_LOG) {
            // Try to place leaves in adjacent positions as fallback
            std::vector<std::pair<int, int>> offsets = {
                {0, 1}, {0, -1}, {1, 0}, {-1, 0},  // Cardinal directions
                {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // Diagonals
            };
            
            for (auto& offset : offsets) {
                int fallbackX = x + offset.first;
                int fallbackZ = z + offset.second;
                
                if (fallbackX >= 0 && fallbackX < CHUNK_SIZE && 
                    fallbackZ >= 0 && fallbackZ < CHUNK_SIZE) {
                    BlockType fallbackBlock = chunk.getBlock(fallbackX, y, fallbackZ);
                    if (fallbackBlock == BlockType::AIR || fallbackBlock == BlockType::LEAVES) {
                        chunk.setBlock(fallbackX, y, fallbackZ, BlockType::LEAVES);
                        return; // Found a good spot!
                    }
                }
            }
            
            // If horizontal placement failed, try one level up
            if (y + 1 < CHUNK_HEIGHT) {
                BlockType aboveBlock = chunk.getBlock(x, y + 1, z);
                if (aboveBlock == BlockType::AIR || aboveBlock == BlockType::LEAVES) {
                    chunk.setBlock(x, y + 1, z, BlockType::LEAVES);
                    return;
                }
            }
            
            // If one level up failed, try one level down
            if (y - 1 >= 0) {
                BlockType belowBlock = chunk.getBlock(x, y - 1, z);
                if (belowBlock == BlockType::AIR || belowBlock == BlockType::LEAVES) {
                    chunk.setBlock(x, y - 1, z, BlockType::LEAVES);
                    return;
                }
            }
        }
    }
}

int TreeFeature::countLeavesAroundTree(Chunk& chunk, int centerX, int leafStartY, int centerZ) const {
    // Count existing leaves in the area where this tree's canopy should be
    int leafCount = 0;
    
    // Check a 5x5 area at multiple heights around the tree
    for (int dy = -1; dy <= 3; dy++) {
        int checkY = leafStartY + dy;
        if (checkY < 0 || checkY >= CHUNK_HEIGHT) continue;
        
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                int checkX = centerX + dx;
                int checkZ = centerZ + dz;
                
                if (checkX >= 0 && checkX < CHUNK_SIZE && 
                    checkZ >= 0 && checkZ < CHUNK_SIZE) {
                    if (chunk.getBlock(checkX, checkY, checkZ) == BlockType::LEAVES) {
                        leafCount++;
                    }
                }
            }
        }
    }
    
    return leafCount;
}

bool TreeFeature::shouldGenerateTreeAtPosition(const TerrainContext& context) const {
    // Use noise to determine tree areas
    double treeNoise = m_treeNoise.octaveNoise(
        context.worldPos.x * m_params.frequency,
        context.worldPos.z * m_params.frequency,
        2, 0.5
    );
    
    if (treeNoise <= m_params.threshold) {
        return false;
    }
    
    // Check spacing constraint
    return checkSpacing(context);
}

int TreeFeature::getTerrainHeightAt(int worldX, int worldZ) const {
    if (m_baseGenerator) {
        return m_baseGenerator->getTerrainHeight(worldX, worldZ);
    }
    // Fallback if no base generator is set
    return 64;
}

bool TreeFeature::checkSpacing(const TerrainContext& context) const {
    // More flexible spacing that allows multiple positions per grid
    int gridSize = m_params.minSpacing;
    int gridX = context.worldPos.x / gridSize;
    int gridZ = context.worldPos.z / gridSize;
    int localX = context.worldPos.x % gridSize;
    int localZ = context.worldPos.z % gridSize;
    
    // Use grid coordinates for pseudo-random positioning
    unsigned int hash = (gridX * 73856093) ^ (gridZ * 19349663);
    
    // Allow trees in the center 3x3 area of each grid cell for more flexibility
    int centerStart = gridSize / 2 - 1;
    int centerEnd = gridSize / 2 + 1;
    
    if (localX >= centerStart && localX <= centerEnd && 
        localZ >= centerStart && localZ <= centerEnd) {
        // Use hash to determine if this specific position should have a tree
        return (hash % 4) == 0;  // 25% chance within the center area
    }
    
    return false;
}

int TreeFeature::getTreeHeight(const TerrainContext& context) const {
    // Use noise for height variation
    double noiseValue = m_treeNoise.noise(context.worldPos.x * 0.1, context.worldPos.z * 0.1, 42.0);
    int height = m_params.minHeight + static_cast<int>((noiseValue + 1.0) * 0.5 * (m_params.maxHeight - m_params.minHeight));
    return std::clamp(height, m_params.minHeight, m_params.maxHeight);
}

void TreeFeature::generateCompleteTree(Chunk& chunk, int localX, int baseY, int localZ, int height, const TerrainContext& context) const {
    // Generate single-width trunk
    for (int y = 0; y < height; y++) {
        int trunkY = baseY + y;
        if (trunkY < CHUNK_HEIGHT) {
            // Only place the trunk in a 1x1 column
            chunk.setBlock(localX, trunkY, localZ, BlockType::OAK_LOG);
        }
    }
    
    // Generate leaves using the complete canopy method for consistency
    int leafStartY = baseY + height - 2; // Start leaves 2 blocks from top
    generateCompleteCanopy(chunk, localX, leafStartY, localZ, height);
}

void TreeFeature::generateRobustLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight) const {
    // ULTRA SIMPLE BULLETPROOF APPROACH - just place leaves everywhere around the tree
    // No complex logic, no edge case handling - just brute force place leaves
    
    // Calculate the area we'll fill with leaves (generous bounds)
    int startY = std::max(0, leafStartY - 2);
    int endY = std::min(CHUNK_HEIGHT - 1, leafStartY + 4);
    
    // Place leaves in a simple 5x5 area around the tree at multiple heights
    for (int y = startY; y <= endY; y++) {
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                int x = centerX + dx;
                int z = centerZ + dz;
                
                // Skip if out of chunk bounds
                if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) continue;
                
                // Skip the center trunk column
                if (dx == 0 && dz == 0) continue;
                
                // Just place leaves everywhere - no complex conditions
                BlockType current = chunk.getBlock(x, y, z);
                if (current == BlockType::AIR || current == BlockType::LEAVES) {
                    chunk.setBlock(x, y, z, BlockType::LEAVES);
                }
            }
        }
    }
    
    // Also place some leaves directly adjacent to ensure NO tree is bare
    for (int y = leafStartY; y <= leafStartY + 2; y++) {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;
        
        // Force place leaves in cardinal directions
        if (centerX - 1 >= 0) {
            BlockType current = chunk.getBlock(centerX - 1, y, centerZ);
            if (current == BlockType::AIR || current == BlockType::LEAVES) {
                chunk.setBlock(centerX - 1, y, centerZ, BlockType::LEAVES);
            }
        }
        if (centerX + 1 < CHUNK_SIZE) {
            BlockType current = chunk.getBlock(centerX + 1, y, centerZ);
            if (current == BlockType::AIR || current == BlockType::LEAVES) {
                chunk.setBlock(centerX + 1, y, centerZ, BlockType::LEAVES);
            }
        }
        if (centerZ - 1 >= 0) {
            BlockType current = chunk.getBlock(centerX, y, centerZ - 1);
            if (current == BlockType::AIR || current == BlockType::LEAVES) {
                chunk.setBlock(centerX, y, centerZ - 1, BlockType::LEAVES);
            }
        }
        if (centerZ + 1 < CHUNK_SIZE) {
            BlockType current = chunk.getBlock(centerX, y, centerZ + 1);
            if (current == BlockType::AIR || current == BlockType::LEAVES) {
                chunk.setBlock(centerX, y, centerZ + 1, BlockType::LEAVES);
            }
        }
    }
}

void TreeFeature::generateAggressiveLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight) const {
    // More aggressive leaf generation with better chunk boundary handling
    
    // Layer 1: Top of tree (single leaf block)
    int topY = leafStartY + 2;
    if (topY < CHUNK_HEIGHT) {
        forceLeafBlock(chunk, centerX, topY, centerZ);
    }
    
    // Layer 2: Plus pattern around top
    int layer2Y = leafStartY + 1;
    if (layer2Y < CHUNK_HEIGHT) {
        forceLeafBlock(chunk, centerX, layer2Y, centerZ);
        forceLeafBlock(chunk, centerX - 1, layer2Y, centerZ);
        forceLeafBlock(chunk, centerX + 1, layer2Y, centerZ);
        forceLeafBlock(chunk, centerX, layer2Y, centerZ - 1);
        forceLeafBlock(chunk, centerX, layer2Y, centerZ + 1);
        
        // Add diagonal corners for fuller look
        forceLeafBlock(chunk, centerX - 1, layer2Y, centerZ - 1);
        forceLeafBlock(chunk, centerX + 1, layer2Y, centerZ - 1);
        forceLeafBlock(chunk, centerX - 1, layer2Y, centerZ + 1);
        forceLeafBlock(chunk, centerX + 1, layer2Y, centerZ + 1);
    }
    
    // Layer 3 & 4: Main leaf layers (3x3 core to stay in chunk)
    for (int layerOffset = 0; layerOffset <= 1; layerOffset++) {
        int layerY = leafStartY + layerOffset;
        if (layerY >= CHUNK_HEIGHT) continue;
        
        // Core 3x3 pattern that should always fit
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                // Skip the center if there's a trunk
                if (dx == 0 && dz == 0 && layerOffset == 0) {
                    continue; // Trunk goes here
                }
                
                forceLeafBlock(chunk, centerX + dx, layerY, centerZ + dz);
            }
        }
        
        // Add selective outer ring only if within chunk bounds
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                // Skip the core 3x3 we already handled
                if (abs(dx) <= 1 && abs(dz) <= 1) continue;
                
                // Skip corners for natural look
                if (abs(dx) == 2 && abs(dz) == 2) continue;
                
                // Only place if it would be within chunk bounds
                int leafX = centerX + dx;
                int leafZ = centerZ + dz;
                if (leafX >= 1 && leafX < CHUNK_SIZE - 1 && 
                    leafZ >= 1 && leafZ < CHUNK_SIZE - 1) {
                    forceLeafBlock(chunk, leafX, layerY, leafZ);
                }
            }
        }
    }
    
    // Layer 5: Bottom layer (2x2 core only)
    int bottomY = leafStartY - 1;
    if (bottomY >= 0 && bottomY < CHUNK_HEIGHT) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue;
                forceLeafBlock(chunk, centerX + dx, bottomY, centerZ + dz);
            }
        }
    }
}

bool TreeFeature::forceLeafBlock(Chunk& chunk, int x, int y, int z) const {
    // Simple leaf placement - just place it if the spot is valid
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT) {
        BlockType currentBlock = chunk.getBlock(x, y, z);
        if (currentBlock == BlockType::AIR || currentBlock == BlockType::LEAVES) {
            chunk.setBlock(x, y, z, BlockType::LEAVES);
            return true;
        }
    }
    return false;
}

void TreeFeature::generateMinecraftLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight, const TerrainContext& context) const {
    // Generate a more Minecraft-like leaf pattern
    // Layer 1: Top of tree (single leaf block)
    int topY = leafStartY + 2;
    if (topY < CHUNK_HEIGHT) {
        placeLeafBlock(chunk, centerX, topY, centerZ, context);
    }
    
    // Layer 2: Cross pattern around top
    int layer2Y = leafStartY + 1;
    if (layer2Y < CHUNK_HEIGHT) {
        // Place center leaf
        placeLeafBlock(chunk, centerX, layer2Y, centerZ, context);
        
        // Cardinal directions
        placeLeafBlock(chunk, centerX - 1, layer2Y, centerZ, context);
        placeLeafBlock(chunk, centerX + 1, layer2Y, centerZ, context);
        placeLeafBlock(chunk, centerX, layer2Y, centerZ - 1, context);
        placeLeafBlock(chunk, centerX, layer2Y, centerZ + 1, context);
    }
    
    // Layer 3 & 4: Main leaf layers (5x5 with corners cut)
    for (int layerOffset = 0; layerOffset <= 1; layerOffset++) {
        int layerY = leafStartY + layerOffset;
        if (layerY >= CHUNK_HEIGHT) continue;
        
        for (int dx = -2; dx <= 2; dx++) {
            for (int dz = -2; dz <= 2; dz++) {
                // Skip far corners to make it more natural
                if (abs(dx) == 2 && abs(dz) == 2) {
                    // Only place corner leaves sometimes for variation
                    if ((dx + dz + centerX + centerZ) % 3 == 0) {
                        placeLeafBlock(chunk, centerX + dx, layerY, centerZ + dz, context);
                    }
                    continue;
                }
                
                // Skip the center if there's a trunk
                if (dx == 0 && dz == 0 && layerOffset == 0) {
                    continue; // Trunk goes here
                }
                
                placeLeafBlock(chunk, centerX + dx, layerY, centerZ + dz, context);
            }
        }
    }
    
    // Layer 5: Bottom layer (3x3)
    int bottomY = leafStartY - 1;
    if (bottomY >= 0 && bottomY < CHUNK_HEIGHT) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                // Skip center (trunk)
                if (dx == 0 && dz == 0) continue;
                
                placeLeafBlock(chunk, centerX + dx, bottomY, centerZ + dz, context);
            }
        }
    }
}

void TreeFeature::placeLeafBlock(Chunk& chunk, int x, int y, int z, const TerrainContext& context) const {
    // Smart leaf placement that respects other tree trunks
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT) {
        BlockType currentBlock = chunk.getBlock(x, y, z);
        
        // Only place leaves if the space is empty or already has leaves
        // Don't overwrite tree trunks - this allows trees to coexist peacefully
        if (currentBlock == BlockType::AIR || currentBlock == BlockType::LEAVES) {
            chunk.setBlock(x, y, z, BlockType::LEAVES);
        }
        // If there's an OAK_LOG, respect it and don't place leaves there
    }
    // Note: Leaves outside chunk boundaries will be handled by post-processing
    // or when neighboring chunks are generated
}
