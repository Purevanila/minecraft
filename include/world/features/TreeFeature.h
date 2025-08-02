#pragma once
#include "world/ModularWorldGenerator.h"
#include "world/PerlinNoise.h"

// Forward declaration
class TerrainGenerator;

/**
 * Tree Generation Feature
 * Demonstrates how easy it is to add new world generation features!
 */
class TreeFeature : public TerrainFeature {
public:
    TreeFeature(unsigned int seed);
    
    void generate(Chunk& chunk, const TerrainContext& context) override;
    bool shouldGenerate(const TerrainContext& context) const override;
    std::string getName() const override { return "TreeFeature"; }
    int getPriority() const override { return 10; } // Generate after base terrain
    
    // Set the base terrain generator for height queries
    void setBaseGenerator(const TerrainGenerator* generator) { m_baseGenerator = generator; }
    
    // Tree generation parameters
    struct TreeParams {
        double frequency = 0.05;    // Good frequency for natural distribution
        double threshold = 0.3;     // Moderate threshold for reasonable tree density
        int minHeight = 4;          // Taller minimum trees
        int maxHeight = 7;          // Taller maximum trees for better leaf layers
        int minSpacing = 5;         // Reasonable spacing between trees
    };
    
    void setParams(const TreeParams& params) { m_params = params; }
    const TreeParams& getParams() const { return m_params; }
    
    // Post-processing to ensure all trees are complete
    void ensureAllTreesGenerated(Chunk& chunk) const;
    
private:
    PerlinNoise m_treeNoise;
    TreeParams m_params;
    const TerrainGenerator* m_baseGenerator = nullptr;
    
    bool checkSpacing(const TerrainContext& context) const;
    int getTreeHeight(const TerrainContext& context) const;
    
    // New improved tree generation methods
    void generateImprovedTree(Chunk& chunk, int localX, int baseY, int localZ, int height, const TerrainContext& context) const;
    void generateSimpleLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ, int treeHeight) const;
    void placeLeaf(Chunk& chunk, int x, int y, int z) const;
    int countNearbyLeaves(Chunk& chunk, int centerX, int leafStartY, int centerZ) const;
    
    // Helper methods for comprehensive tree generation
    bool shouldGenerateTreeAtPosition(const TerrainContext& context) const;
    int getTerrainHeightAt(int worldX, int worldZ) const;
};
