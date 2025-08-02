#pragma once

#include "world/Block.h"
#include "engine/graphics/Mesh.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include <atomic>

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 64;  // Increased height for terrain generation

// Forward declarations
class ModularWorldGenerator;
class TerrainGenerator;

class Mesh;
class TerrainGenerator;

class Chunk {
public:
    Chunk(const glm::ivec2& position, ModularWorldGenerator* terrainGen = nullptr, bool autoGenerate = true);
    ~Chunk();
    
    // Block management (optimized with inline functions)
    // Methods to set and retrieve block types in the chunk
    void setBlock(int x, int y, int z, BlockType type);
    BlockType getBlock(int x, int y, int z) const;
    const Block& getBlockObject(int x, int y, int z) const;
    
    // Ultra-fast block access for performance-critical code
    inline BlockType getBlockFast(int x, int y, int z) const {
        if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) 
            return BlockType::AIR;
        int index = x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
        // Safety check for vector bounds
        if (index >= 0 && index < static_cast<int>(m_blockTypes.size())) {
            return m_blockTypes[index];
        }
        return BlockType::AIR;
    }
    
    // Ultra-fast block setting for terrain generation
    inline void setBlockFast(int x, int y, int z, BlockType type) {
        if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE) {
            int index = x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
            // Safety check for vector bounds with debug info
            if (index >= 0 && index < static_cast<int>(m_blockTypes.size())) {
                m_blockTypes[index] = type;
            } else {
                // Should never happen if bounds checking is correct
                std::cerr << "ERROR: setBlockFast index out of bounds! x=" << x << " y=" << y << " z=" << z 
                         << " index=" << index << " size=" << m_blockTypes.size() << std::endl;
            }
        }
    }
    
    // Chunk operations
    // Methods to generate and render this chunk
    void generate();            // Generate terrain and mesh (main thread)
    void buildMesh();           // Create mesh for rendering (must run on render thread)
    void buildMeshWithCulling(const glm::vec3& cameraPos, const glm::vec3& cameraDir); // Build mesh with view culling
    // Generate terrain data only (mesh will be built separately)
    void generateTerrainOnly(); // Prepare terrain layout without building mesh
    void buildMeshData();      // Build vertex and index data on the CPU
    void uploadMesh();         // Send mesh data to the GPU
    void render(const glm::mat4& view, const glm::mat4& projection);
    void drawWaterMesh() const;  // Draw mesh for water blocks only
    void drawOakMesh() const;    // Draw mesh for oak log blocks only
    void drawLeavesMesh() const; // Draw mesh for leaves blocks only
    void drawStoneMesh() const;  // Draw mesh for stone blocks only
    void drawGravelMesh() const; // Draw mesh for gravel blocks only
    void drawSandMesh() const;   // Draw mesh for sand blocks only
    
    // Async mesh building support
    void markReadyForUpload();   // Flag chunk as having mesh data ready for GPU
    bool needsUpload() const;    // Check if mesh data needs to be uploaded to GPU
    
    // Helpers to check if the chunk needs generation or mesh rebuild
    bool needsGeneration() const { return !m_generated; }
    bool needsMeshRebuild() const { return m_needsRebuild; }
    bool isGenerated() const { return m_generated; }
    
    // Helpers to get chunk coordinates and world position
    glm::ivec2 getPosition() const { return m_position; }
    glm::vec3 getWorldPosition() const { 
        return glm::vec3(m_position.x * CHUNK_SIZE, 0, m_position.y * CHUNK_SIZE); 
    }
    
    // Helper functions for validating block positions
    bool isValidPosition(int x, int y, int z) const;
    int getIndex(int x, int y, int z) const;
    
private:
    glm::ivec2 m_position;  // Chunk coordinates within the world
    // ⚡ ULTRA-FAST block storage - just store block types, not full objects
    std::vector<BlockType> m_blockTypes;
    std::vector<std::unique_ptr<Block>> m_blocks;
    std::unique_ptr<Mesh> m_mesh;           // Mesh containing solid block geometry
    std::unique_ptr<Mesh> m_waterMesh;      // Mesh containing water block geometry
    std::unique_ptr<Mesh> m_oakMesh;        // Mesh containing oak log block geometry
    std::unique_ptr<Mesh> m_leavesMesh;     // Mesh containing leaves block geometry
    std::unique_ptr<Mesh> m_stoneMesh;      // Mesh containing stone block geometry
    std::unique_ptr<Mesh> m_gravelMesh;     // Mesh containing gravel block geometry
    std::unique_ptr<Mesh> m_sandMesh;       // Mesh containing sand block geometry
    bool m_needsRebuild;
    std::atomic<bool> m_generated{false};  // Atomic for thread safety
    bool m_readyForUpload = false;  // True if mesh data is built and ready for GPU upload
    ModularWorldGenerator* m_terrainGenerator; // Shared modular terrain generator instance
    
    void generateTerrain();
    void generateFlatTerrain(); // Use a simple flat terrain as fallback
    void addTerrainVariation(int x, int z, int surfaceHeight);
    void addFaceToMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                       const glm::vec3& blockPos, int faceIndex, const glm::vec3& normal, 
                       unsigned int& vertexIndex);
};
