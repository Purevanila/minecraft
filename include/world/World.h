#pragma once

#include "world/Chunk.h"
#include "world/ModularWorldGenerator.h"
#include "world/features/TreeFeature.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ChunkRenderer;
class Camera;

// Hash function for glm::ivec2 to use as key in unordered_map
struct ChunkPositionHash {
    std::size_t operator()(const glm::ivec2& pos) const {
        return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.y) << 1);
    }
};

class World {
public:
    World();
    ~World();
    
    // Main update and render functions
    void update(const glm::vec3& playerPosition);
    void render(ChunkRenderer* renderer, const glm::mat4& view, const glm::mat4& projection);
    
    // Block access
    Chunk* getChunk(const glm::ivec2& chunkPos);
    BlockType getBlock(int x, int y, int z) const;
    BlockType getBlockType(const glm::ivec3& worldPos) const; // Convenience method for vector input
    void setBlock(int x, int y, int z, BlockType type);
    
    // Coordinate conversion utilities
    glm::ivec2 worldToChunkPosition(const glm::vec3& worldPos) const;
    glm::ivec3 worldToLocalPosition(const glm::vec3& worldPos) const;
    
    // Configuration
    void setRenderDistance(int distance) { m_renderDistance = distance; }
    int getRenderDistance() const { return m_renderDistance; }
    
    // Loading progress tracking
    int getLoadedChunkCount() const;
    int getRequiredChunkCount(const glm::vec3& playerPosition) const;
    bool isInitialLoadingComplete(const glm::vec3& playerPosition) const;
    
    // 🌍 Terrain generation access
    ModularWorldGenerator* getTerrainGenerator() const { return m_terrainGenerator.get(); }

private:
    // Core data
    std::unordered_map<glm::ivec2, std::unique_ptr<Chunk>, ChunkPositionHash> m_chunks;
    std::unique_ptr<ModularWorldGenerator> m_terrainGenerator; // 🌍 Natural world generation with features
    
    // World state
    int m_renderDistance;
    glm::ivec2 m_lastPlayerChunkPos;
    bool m_firstUpdate;
    
    // ⚡ Async chunk generation system with thread pool
    std::queue<glm::ivec2> m_chunksNeedingGeneration;
    std::unordered_set<glm::ivec2, ChunkPositionHash> m_chunksInGenerationQueue;  // Track chunks already queued
    std::mutex m_generationQueueMutex;
    mutable std::mutex m_chunksMutex;  // Protects access to m_chunks map
    std::vector<std::thread> m_generationThreads; // Multiple worker threads
    std::condition_variable m_generationCondition;
    std::atomic<bool> m_stopGeneration;
    static constexpr int NUM_GENERATION_THREADS = 4; // Use 4 threads for generation
    
    // Performance settings - optimized for smoothness
    static constexpr float UNLOAD_DISTANCE_MULTIPLIER = 1.5f; // When to unload chunks
    
    // Internal methods
    void generateChunksAroundPlayer(const glm::vec3& playerPosition);
    void preloadChunksAhead(const glm::vec3& playerPosition, const glm::ivec2& currentChunk);
    void unloadDistantChunks(const glm::vec3& playerPosition);
    void generateTerrainAsync(); // ⚡ Background terrain generation
    void terrainGenerationWorker(); // ⚡ Background worker thread
    
    // Utility functions
    bool isChunkLoaded(const glm::ivec2& chunkPos) const;
    std::vector<glm::ivec2> getChunksInRange(const glm::ivec2& center, int range) const;
    float getChunkDistance(const glm::ivec2& chunk1, const glm::ivec2& chunk2) const;
    float calculateChunkPriority(const glm::ivec2& chunkPos, const glm::ivec2& playerChunk) const;
};
