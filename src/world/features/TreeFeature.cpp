#include "world/features/TreeFeature.h"
#include "world/Chunk.h"
#include <iostream>
#include <algorithm>
#include <random>

TreeFeature::TreeFeature(unsigned int seed) 
    : m_treeNoise(seed + 4000) { // Different seed from terrain generation
}

bool TreeFeature::shouldGenerate(const TerrainContext& context) const {
    // Don't generate trees in lakes or below/at water level
    if (context.isLakeArea || context.worldPos.y <= context.terrainHeight) {
        return false;
    }
    
    // NEW: Don't generate trees if terrain is at or below water level (ocean areas)
    if (context.terrainHeight <= context.waterLevel) {
        return false;
    }
    
    // NEW: Don't generate trees too close to water level (beaches)
    if (context.terrainHeight < context.waterLevel + 3) {
        return false;
    }
    
    // Only generate trees at the exact base position (1 block above terrain)
    if (context.worldPos.y != context.terrainHeight + 1) {
        return false;
    }
    
    // Use noise to determine tree areas with better distribution
    double treeNoise = m_treeNoise.octaveNoise(
        context.worldPos.x * m_params.frequency,
        context.worldPos.z * m_params.frequency,
        3, 0.6  // More octaves and higher persistence for better patterns
    );
    
    if (treeNoise <= m_params.threshold) {
        return false;
    }
    
    // Improved spacing constraint with Poisson disk sampling approach
    return checkSpacing(context);
}

void TreeFeature::generate(Chunk& chunk, const TerrainContext& context) {
    // Convert world position to chunk-local position
    int localX = context.worldPos.x - (context.chunkX * CHUNK_SIZE);
    int localZ = context.worldPos.z - (context.chunkZ * CHUNK_SIZE);
    
    // Validate chunk bounds - keep trees away from edges for better leaf placement
    if (localX < 2 || localX >= CHUNK_SIZE - 2 || localZ < 2 || localZ >= CHUNK_SIZE - 2) {
        return;
    }

    // Generate complete tree at the base position
    int treeHeight = getTreeHeight(context);
    int treeStartY = context.terrainHeight + 1;
    
    // Generate the complete tree with improved algorithm
    generateImprovedTree(chunk, localX, treeStartY, localZ, treeHeight, context);
}

// Simplified post-processing to ensure all trees have adequate leaves
void TreeFeature::ensureAllTreesGenerated(Chunk& chunk) const {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            // Look for oak logs that might be tree bases
            for (int y = 1; y < CHUNK_HEIGHT - 3; y++) {
                if (chunk.getBlock(x, y, z) == BlockType::OAK_LOG) {
                    // Check if this is the base of a tree (no log below)
                    bool isTreeBase = (y == 0 || chunk.getBlock(x, y - 1, z) != BlockType::OAK_LOG);
                    
                    if (isTreeBase) {
                        // Find the full height of this tree
                        int treeHeight = 1;
                        for (int checkY = y + 1; checkY < CHUNK_HEIGHT && checkY < y + m_params.maxHeight + 2; checkY++) {
                            if (chunk.getBlock(x, checkY, z) == BlockType::OAK_LOG) {
                                treeHeight++;
                            } else {
                                break;
                            }
                        }
                        
                        // Count existing leaves around this tree
                        int leafCount = countNearbyLeaves(chunk, x, y + treeHeight - 2, z);
                        
                        // If tree has insufficient leaves, generate them
                        if (leafCount < 8) { // Minimum 8 leaves for a complete tree
                            generateSimpleLeaves(chunk, x, y + treeHeight - 2, z, treeHeight);
                        }
                        
                        // Skip ahead to avoid processing this tree multiple times
                        y += treeHeight;
                    }
                }
            }
        }
    }
}

int TreeFeature::countNearbyLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ) const {
    int leafCount = 0;
    
    // Check a 5x5 area at multiple heights around the tree
    for (int dy = -1; dy <= 2; dy++) {
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
        3, 0.6
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
    // Improved spacing using Poisson disk sampling approach for more natural distribution
    int gridSize = m_params.minSpacing;
    int worldX = context.worldPos.x;
    int worldZ = context.worldPos.z;
    
    // Create a grid-based hash for consistent positioning
    int gridX = worldX / gridSize;
    int gridZ = worldZ / gridSize;
    
    // Use a simple hash function for pseudo-random positioning within each grid cell
    std::mt19937 rng((gridX * 73856093) ^ (gridZ * 19349663));
    std::uniform_int_distribution<int> dist(0, gridSize - 1);
    
    // Generate a random position within this grid cell
    int targetX = gridX * gridSize + dist(rng);
    int targetZ = gridZ * gridSize + dist(rng);
    
    // Only allow trees at the exact target position
    return (worldX == targetX && worldZ == targetZ);
}

int TreeFeature::getTreeHeight(const TerrainContext& context) const {
    // Use noise for height variation with better distribution
    double noiseValue = m_treeNoise.noise(context.worldPos.x * 0.1, context.worldPos.z * 0.1, 42.0);
    int height = m_params.minHeight + static_cast<int>((noiseValue + 1.0) * 0.5 * (m_params.maxHeight - m_params.minHeight));
    return std::clamp(height, m_params.minHeight, m_params.maxHeight);
}

void TreeFeature::generateImprovedTree(Chunk& chunk, int localX, int baseY, int localZ, int height, const TerrainContext& context) const {
    // Generate trunk (single column)
    for (int y = 0; y < height; y++) {
        int trunkY = baseY + y;
        if (trunkY >= 0 && trunkY < CHUNK_HEIGHT) {
            chunk.setBlock(localX, trunkY, localZ, BlockType::OAK_LOG);
        }
    }
    
    // Generate leaves using simplified but effective algorithm
    generateSimpleLeaves(chunk, localX, baseY + height - 2, localZ, height);
}

void TreeFeature::generateSimpleLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight) const {
    // Simple but effective leaf generation - creates natural-looking trees
    
    // Top leaf (crown)
    int topY = leafStartY + 2;
    if (topY >= 0 && topY < CHUNK_HEIGHT) {
        placeLeaf(chunk, centerX, topY, centerZ);
    }
    
    // Upper layer (plus pattern)
    int upperY = leafStartY + 1;
    if (upperY >= 0 && upperY < CHUNK_HEIGHT) {
        placeLeaf(chunk, centerX, upperY, centerZ);
        placeLeaf(chunk, centerX - 1, upperY, centerZ);
        placeLeaf(chunk, centerX + 1, upperY, centerZ);
        placeLeaf(chunk, centerX, upperY, centerZ - 1);
        placeLeaf(chunk, centerX, upperY, centerZ + 1);
    }
    
    // Main canopy layers (3x3 and 5x5)
    for (int layerOffset = 0; layerOffset <= 1; layerOffset++) {
        int layerY = leafStartY + layerOffset;
        if (layerY < 0 || layerY >= CHUNK_HEIGHT) continue;
        
        // 3x3 core
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0 && layerOffset == 0) continue; // Skip trunk
                placeLeaf(chunk, centerX + dx, layerY, centerZ + dz);
            }
        }
        
        // Extended 5x5 (skip corners for natural look)
        if (layerOffset == 0) {
            // Cardinal extensions
            placeLeaf(chunk, centerX - 2, layerY, centerZ);
            placeLeaf(chunk, centerX + 2, layerY, centerZ);
            placeLeaf(chunk, centerX, layerY, centerZ - 2);
            placeLeaf(chunk, centerX, layerY, centerZ + 2);
            
            // Some additional positions for natural look
            placeLeaf(chunk, centerX - 1, layerY, centerZ - 2);
            placeLeaf(chunk, centerX + 1, layerY, centerZ - 2);
            placeLeaf(chunk, centerX - 1, layerY, centerZ + 2);
            placeLeaf(chunk, centerX + 1, layerY, centerZ + 2);
            placeLeaf(chunk, centerX - 2, layerY, centerZ - 1);
            placeLeaf(chunk, centerX - 2, layerY, centerZ + 1);
            placeLeaf(chunk, centerX + 2, layerY, centerZ - 1);
            placeLeaf(chunk, centerX + 2, layerY, centerZ + 1);
        }
    }
    
    // Bottom layer (smaller 3x3)
    int bottomY = leafStartY - 1;
    if (bottomY >= 0 && bottomY < CHUNK_HEIGHT) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue; // Skip trunk
                placeLeaf(chunk, centerX + dx, bottomY, centerZ + dz);
            }
        }
    }
}

void TreeFeature::placeLeaf(Chunk& chunk, int x, int y, int z) const {
    // Simple leaf placement with bounds checking
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT) {
        BlockType current = chunk.getBlock(x, y, z);
        if (current == BlockType::AIR || current == BlockType::LEAVES) {
            chunk.setBlock(x, y, z, BlockType::LEAVES);
        }
    }
}
