#pragma once

#include "world/Chunk.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

/**
 * Level of Detail (LOD) System for Chunks
 * Automatically reduces geometry complexity based on distance from camera
 * Maintains visual quality while dramatically improving performance
 */
class ChunkLODManager {
public:
    enum class LODLevel {
        FULL_DETAIL = 0,     // All blocks rendered normally
        MEDIUM_DETAIL = 1,   // Skip some interior faces, reduce water detail
        LOW_DETAIL = 2,      // Only render surface blocks
        BILLBOARD = 3        // Single textured quad representing entire chunk
    };
    
    struct LODChunk {
        std::unique_ptr<Chunk> fullChunk;
        std::unique_ptr<Mesh> lodMeshes[4];  // One mesh per LOD level
        LODLevel currentLOD;
        float distanceFromCamera;
        bool lodMeshesBuilt[4];
        
        LODChunk() : currentLOD(LODLevel::FULL_DETAIL), distanceFromCamera(0.0f) {
            for (int i = 0; i < 4; ++i) lodMeshesBuilt[i] = false;
        }
    };
    
    ChunkLODManager();
    ~ChunkLODManager();
    
    // Configure LOD distance thresholds
    void setLODDistances(float medium, float low, float billboard);
    
    // Update LOD levels based on camera position
    void updateLOD(const glm::vec3& cameraPosition);
    
    // Add a chunk to LOD management
    void addChunk(std::unique_ptr<Chunk> chunk);
    
    // Remove a chunk from LOD management
    void removeChunk(const glm::ivec2& chunkPos);
    
    // Get the appropriate mesh for rendering a chunk
    Mesh* getChunkMesh(const glm::ivec2& chunkPos);
    
    // Build LOD meshes for a chunk in background thread
    void buildLODMeshes(const glm::ivec2& chunkPos);
    
    // Performance statistics
    int getChunksAtLOD(LODLevel level) const;
    float getAverageTriangleReduction() const;

private:
    std::unordered_map<glm::ivec2, std::unique_ptr<LODChunk>, ChunkPositionHash> m_lodChunks;
    
    // LOD distance thresholds
    float m_mediumDetailDistance;
    float m_lowDetailDistance;
    float m_billboardDistance;
    
    // Calculate appropriate LOD level for distance
    LODLevel calculateLODLevel(float distance) const;
    
    // Build a specific LOD mesh for a chunk
    void buildLODMesh(LODChunk& lodChunk, LODLevel level);
    
    // Generate medium detail mesh (skip some interior faces)
    void generateMediumDetailMesh(const Chunk& chunk, Mesh& mesh);
    
    // Generate low detail mesh (surface only)
    void generateLowDetailMesh(const Chunk& chunk, Mesh& mesh);
    
    // Generate billboard mesh (single quad)
    void generateBillboardMesh(const Chunk& chunk, Mesh& mesh);
    
    // Sample chunk color for billboard
    glm::vec3 sampleChunkColor(const Chunk& chunk);
};
