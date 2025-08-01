#include "world/World.h"
#include "world/WorldConfig.h"
#include "engine/graphics/ChunkRenderer.h"
#include "engine/graphics/Frustum.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>

World::World() 
    : m_renderDistance(16)        // Increased to 16 chunks for high render distance
    , m_lastPlayerChunkPos(0, 0) // Track where the player was last frame
    , m_firstUpdate(true)        // Flag to force initial chunk generation
    , m_stopGeneration(false) {  // Control flag for background generation
    
    // Initialize modular terrain generator with a random seed for varied worlds
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    m_terrainGenerator = std::make_unique<ModularWorldGenerator>(seed);
    
    // Add tree generation feature with config parameters
    auto treeFeature = std::make_unique<TreeFeature>(seed);
    
    // Configure tree parameters from world config
    TreeFeature::TreeParams treeParams;
    treeParams.frequency = g_worldConfig.trees.frequency;
    treeParams.threshold = g_worldConfig.trees.threshold;
    treeParams.minHeight = g_worldConfig.trees.minHeight;
    treeParams.maxHeight = g_worldConfig.trees.maxHeight;
    treeParams.minSpacing = g_worldConfig.trees.minSpacing;
    treeFeature->setParams(treeParams);
    
    m_terrainGenerator->addFeature(std::move(treeFeature));
    
    // World created with ModularWorldGenerator and TreeFeature
    
    // ⚡ PERFORMANCE: Start multiple background worker threads for chunk generation
    m_generationThreads.reserve(NUM_GENERATION_THREADS);
    for (int i = 0; i < NUM_GENERATION_THREADS; ++i) {
        m_generationThreads.emplace_back(&World::terrainGenerationWorker, this);
    }
    
    // World initialization complete
}

World::~World() {
    // Signal all background threads to stop
    m_stopGeneration = true;
    m_generationCondition.notify_all(); // Wake up all worker threads
    
    // Wait for all background generation threads to finish
    for (auto& thread : m_generationThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void World::update(const glm::vec3& playerPosition) {
    // Start timing this update to monitor performance
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Convert the player's world coordinates to chunk coordinates
    // This uhm Ok which chunk the player is currently standing in
    glm::ivec2 currentPlayerChunk = worldToChunkPosition(playerPosition);
    
    // Always check for missing chunks around the player, not just when moving
    // This ensures chunks are continuously generated even when standing still
    generateChunksAroundPlayer(playerPosition);
    
    // Pre-load chunks in a wider radius for smoother experience
    preloadChunksAhead(playerPosition, currentPlayerChunk);
    
    // Only unload distant chunks when player moves to avoid constant unloading
    if (m_firstUpdate || currentPlayerChunk != m_lastPlayerChunkPos) {
        // Player entered new chunk (remove debug output)
        
        // Remove chunks that are too far away to save memory
        unloadDistantChunks(playerPosition);
        
        // Remember where the player is now for next frame
        m_lastPlayerChunkPos = currentPlayerChunk;
        m_firstUpdate = false;
    }
    
    // ⚡ MAIN THREAD: Build more meshes per frame for faster world loading
    int meshesBuilt = 0;
    const int MAX_MESHES_PER_FRAME = 8; // Increased from 3 to 8 for faster loading
    
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        for (auto& [pos, chunk] : m_chunks) {
            if (meshesBuilt >= MAX_MESHES_PER_FRAME) break;
            
            if (chunk && chunk->isGenerated() && chunk->needsMeshRebuild()) {
                chunk->buildMesh(); // Build mesh on main thread (includes GPU upload)
                meshesBuilt++;
            }
        }
    }
    
    // Calculate how long this update took
    auto endTime = std::chrono::high_resolution_clock::now();
    auto updateDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Only show timing info if the update took a noticeable amount of time
    // Removed debug output for cleaner console
}

void World::render(ChunkRenderer* renderer, const glm::mat4& view, const glm::mat4& projection) {
    // Keep track of how many chunks we actually draw this frame
    int chunksRendered = 0;
    
    // ⚡ PERFORMANCE: Extract camera position from view matrix for distance calculations
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    glm::ivec2 cameraChunk = glm::ivec2(floor(cameraPos.x / 16.0f), floor(cameraPos.z / 16.0f));
    
    // ⚡ FRUSTUM CULLING: Setup frustum for view culling
    static Frustum frustum;
    frustum.updateFromViewProjection(projection * view);
    
    // ⚡ PERFORMANCE: Sort chunks by distance and apply LOD + Frustum Culling
    std::vector<std::pair<float, Chunk*>> sortedChunks;
    
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        sortedChunks.reserve(m_chunks.size());
        
        // Go through every chunk we have loaded in memory
        for (const auto& chunkPair : m_chunks) {
            const auto& chunk = chunkPair.second;  // Get the actual chunk object
            
            // Make sure the chunk exists and has been generated
            if (chunk && chunk->isGenerated()) {
                // ⚡ PERFORMANCE: Calculate distance to chunk for LOD
                glm::ivec2 chunkPos = chunk->getPosition();
                float distance = glm::length(glm::vec2(chunkPos - cameraChunk));
                
                // Only render chunks within reasonable distance
                if (distance <= m_renderDistance) {
                    // ⚡ FRUSTUM CULLING: Check if chunk is visible in camera frustum
                    glm::vec3 chunkMin = glm::vec3(chunkPos.x * 16.0f, 0.0f, chunkPos.y * 16.0f);
                    glm::vec3 chunkMax = chunkMin + glm::vec3(16.0f, 128.0f, 16.0f); // Chunk size + height
                    
                    if (frustum.isChunkVisible(chunkMin, chunkMax)) {
                        sortedChunks.emplace_back(distance, chunk.get());
                    }
                }
            }
        }
    }
    
    // ⚡ PERFORMANCE: Sort by distance (closest first) for better GPU cache performance
    std::sort(sortedChunks.begin(), sortedChunks.end());
    
    // Render chunks with distance-based optimizations
    for (const auto& [distance, chunk] : sortedChunks) {
        // ⚡ PERFORMANCE: Skip detailed rendering for very distant chunks
        if (distance > m_renderDistance * 0.8f) {
            // For very distant chunks, could implement simplified rendering here
            // For now, just render normally but this is where LOD would go
        }
        
        // Tell the renderer to draw this chunk with the camera's view
        renderer->renderChunk(*chunk, view, projection);
        chunksRendered++;
    }
    
    // Every 5 seconds or so (at 60fps), log some stats to help with debugging
    static int frameCounter = 0;
    frameCounter++;
    if (frameCounter % 300 == 0) {  
    // Removed debug output for cleaner console
    }
}

Chunk* World::getChunk(const glm::ivec2& chunkPos) {
    std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
    auto it = m_chunks.find(chunkPos);
    return (it != m_chunks.end()) ? it->second.get() : nullptr;
}

BlockType World::getBlock(int x, int y, int z) const {
    // Check Y bounds first - if outside world height, return AIR
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return BlockType::AIR;
    }
    
    // Convert world coordinates to chunk coordinates
    glm::ivec2 chunkPos(x / CHUNK_SIZE, z / CHUNK_SIZE);
    
    // Get local coordinates within the chunk
    int localX = x % CHUNK_SIZE;
    int localZ = z % CHUNK_SIZE;
    
    // Handle negative coordinates properly
    if (localX < 0) {
        localX += CHUNK_SIZE;
        chunkPos.x--;
    }
    if (localZ < 0) {
        localZ += CHUNK_SIZE;
        chunkPos.y--;
    }
    
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        auto it = m_chunks.find(chunkPos);
        if (it != m_chunks.end() && it->second) {
            return it->second->getBlock(localX, y, localZ);
        }
    }
    
    return BlockType::AIR;
}

BlockType World::getBlockType(const glm::ivec3& worldPos) const {
    return getBlock(worldPos.x, worldPos.y, worldPos.z);
}

void World::setBlock(int x, int y, int z, BlockType type) {
    // Check Y bounds first - if outside world height, ignore
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return;
    }
    
    // Convert world coordinates to chunk coordinates
    glm::ivec2 chunkPos(x / CHUNK_SIZE, z / CHUNK_SIZE);
    
    // Get local coordinates within the chunk
    int localX = x % CHUNK_SIZE;
    int localZ = z % CHUNK_SIZE;
    
    // Handle negative coordinates properly
    if (localX < 0) {
        localX += CHUNK_SIZE;
        chunkPos.x--;
    }
    if (localZ < 0) {
        localZ += CHUNK_SIZE;
        chunkPos.y--;
    }
    
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        auto it = m_chunks.find(chunkPos);
        if (it != m_chunks.end() && it->second) {
            it->second->setBlock(localX, y, localZ, type);
        }
    }
}

glm::ivec2 World::worldToChunkPosition(const glm::vec3& worldPos) const {
    return glm::ivec2(
        static_cast<int>(std::floor(worldPos.x / CHUNK_SIZE)),
        static_cast<int>(std::floor(worldPos.z / CHUNK_SIZE))
    );
}

glm::ivec3 World::worldToLocalPosition(const glm::vec3& worldPos) const {
    glm::ivec2 chunkPos = worldToChunkPosition(worldPos);
    return glm::ivec3(
        static_cast<int>(worldPos.x) - chunkPos.x * CHUNK_SIZE,
        static_cast<int>(worldPos.y),
        static_cast<int>(worldPos.z) - chunkPos.y * CHUNK_SIZE
    );
}

void World::generateChunksAroundPlayer(const glm::vec3& playerPosition) {
    glm::ivec2 playerChunk = worldToChunkPosition(playerPosition);
    std::vector<glm::ivec2> neededChunks = getChunksInRange(playerChunk, m_renderDistance);
    
    // Sort chunks by distance - closest first for better player experience
    // Use spiral pattern to minimize visible chunk borders
    std::sort(neededChunks.begin(), neededChunks.end(), 
        [playerChunk, this](const glm::ivec2& a, const glm::ivec2& b) {
            float distA = getChunkDistance(a, playerChunk);
            float distB = getChunkDistance(b, playerChunk);
            
            // If distances are similar, prefer chunks in front of player movement
            if (std::abs(distA - distB) < 0.5f) {

                return a.x + a.y < b.x + b.y; // Simple tie-breaker
            }
            return distA < distB;
        });
    
    // ⚡ ULTRA-FAST PERFORMANCE: Much larger burst settings for smooth gameplay
    int chunksGenerated = 0;
    auto frameStartTime = std::chrono::high_resolution_clock::now();
    constexpr auto MAX_FRAME_TIME = std::chrono::microseconds(16000); // ⚡ 16ms - full frame budget
    constexpr int MAX_CHUNKS_BURST = 32; // ⚡ 32 chunks per frame for very smooth loading
    
    for (const glm::ivec2& chunkPos : neededChunks) {
        // Stop if we've hit our chunk limit or time limit
        if (chunksGenerated >= MAX_CHUNKS_BURST) {
            break;  // Hard limit on chunks per frame
        }
        
        // Check how much time we've used this frame
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - frameStartTime);
        if (frameTime > MAX_FRAME_TIME) {
            break;  // Time limit reached - but should be fast since no terrain generation
        }
        
        if (!isChunkLoaded(chunkPos)) {            
            // Create chunk container without auto-generation
            auto chunk = std::make_unique<Chunk>(chunkPos, m_terrainGenerator.get(), false);
            {
                std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
                m_chunks[chunkPos] = std::move(chunk);
            }
            chunksGenerated++;
            // Removed debug output for cleaner console
        }
    }
    
    // Queue newly created chunks for background terrain generation
    if (chunksGenerated > 0) {
        generateTerrainAsync();
        // Removed debug output for cleaner console
    }
}

void World::preloadChunksAhead(const glm::vec3& playerPosition, const glm::ivec2& currentChunk) {
    // Calculate player movement direction
    static glm::vec3 lastPlayerPos = playerPosition;
    glm::vec3 movement = playerPosition - lastPlayerPos;
    lastPlayerPos = playerPosition;
    
    // If player is moving, preload chunks in movement direction
    if (glm::length(movement) > 0.01f) {
        glm::vec2 direction = glm::normalize(glm::vec2(movement.x, movement.z));
        
        // Preload 2-3 chunks ahead in movement direction
        for (int distance = 1; distance <= 3; distance++) {
            glm::ivec2 preloadChunk = currentChunk + glm::ivec2(
                static_cast<int>(direction.x * distance),
                static_cast<int>(direction.y * distance)
            );
            
            // Only preload if within reasonable distance
            if (getChunkDistance(preloadChunk, currentChunk) <= m_renderDistance + 2) {
                if (!isChunkLoaded(preloadChunk)) {
                    auto chunk = std::make_unique<Chunk>(preloadChunk, m_terrainGenerator.get(), false);
                    {
                        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
                        m_chunks[preloadChunk] = std::move(chunk);
                    }
                }
            }
        }
        
        // Queue preloaded chunks for generation
        generateTerrainAsync();
    }
}

void World::unloadDistantChunks(const glm::vec3& playerPosition) {
    glm::ivec2 playerChunk = worldToChunkPosition(playerPosition);
    float unloadDistance = m_renderDistance * UNLOAD_DISTANCE_MULTIPLIER;
    
    std::vector<glm::ivec2> chunksToUnload;
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        for (const auto& pair : m_chunks) {
            const glm::ivec2& chunkPos = pair.first;
            if (getChunkDistance(playerChunk, chunkPos) > unloadDistance) {
                chunksToUnload.push_back(chunkPos);
            }
        }
    
        for (const glm::ivec2& chunkPos : chunksToUnload) {
            m_chunks.erase(chunkPos);
        }
    }
    
    if (!chunksToUnload.empty()) {
        std::cout << "Unloaded " << chunksToUnload.size() << " distant chunks" << std::endl;
    }
}

bool World::isChunkLoaded(const glm::ivec2& chunkPos) const {
    std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
    return m_chunks.find(chunkPos) != m_chunks.end();
}

std::vector<glm::ivec2> World::getChunksInRange(const glm::ivec2& center, int range) const {
    std::vector<glm::ivec2> chunks;
    
    // Generate chunks in a circular pattern for better loading priority
    for (int r = 0; r <= range; r++) {
        for (int dx = -r; dx <= r; dx++) {
            for (int dz = -r; dz <= r; dz++) {
                // Only include chunks at the current ring distance
                if (std::max(std::abs(dx), std::abs(dz)) == r) {
                    chunks.emplace_back(center.x + dx, center.y + dz);
                }
            }
        }
    }
    
    return chunks;
}

float World::getChunkDistance(const glm::ivec2& chunk1, const glm::ivec2& chunk2) const {
    glm::ivec2 diff = chunk1 - chunk2;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

float World::calculateChunkPriority(const glm::ivec2& chunkPos, const glm::ivec2& playerChunk) const {
    float distance = getChunkDistance(chunkPos, playerChunk);
    return 1.0f / (1.0f + distance);  // Closer chunks have higher priority
}

void World::generateTerrainAsync() {
    // ⚡ IMPROVED: Queue chunks that need terrain generation with priority sorting
    std::lock_guard<std::mutex> lock(m_generationQueueMutex);
    
    // Find chunks that need terrain generation and sort by priority
    std::vector<std::pair<float, glm::ivec2>> chunkPriorities;
    glm::ivec2 playerChunk = m_lastPlayerChunkPos;
    
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        for (auto& [pos, chunk] : m_chunks) {
            if (chunk && chunk->needsGeneration()) {
                float priority = calculateChunkPriority(pos, playerChunk);
                chunkPriorities.emplace_back(priority, pos);
            }
        }
    }
    
    // Sort by priority (highest first)
    std::sort(chunkPriorities.begin(), chunkPriorities.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Add high-priority chunks to generation queue (avoid duplicates)
    for (const auto& [priority, pos] : chunkPriorities) {
        if (m_chunksInGenerationQueue.find(pos) == m_chunksInGenerationQueue.end()) {
            m_chunksNeedingGeneration.push(pos);
            m_chunksInGenerationQueue.insert(pos);
        }
    }
    
    // Wake up the background worker thread
    m_generationCondition.notify_one();
}

void World::terrainGenerationWorker() {
    while (!m_stopGeneration) {
        std::unique_lock<std::mutex> lock(m_generationQueueMutex);
        
        // Wait for chunks to process or stop signal
        m_generationCondition.wait(lock, [this] {
            return !m_chunksNeedingGeneration.empty() || m_stopGeneration;
        });
        
        if (m_stopGeneration) {
            break;
        }
        
        // ⚡ MULTI-THREADED: Process optimized batches per thread for high render distance
        std::vector<glm::ivec2> chunksToProcess;
        while (!m_chunksNeedingGeneration.empty() && chunksToProcess.size() < 8) { // 8 per thread * 4 threads = 32 total per frame
            glm::ivec2 pos = m_chunksNeedingGeneration.front();
            chunksToProcess.push_back(pos);
            m_chunksNeedingGeneration.pop();
            m_chunksInGenerationQueue.erase(pos);  // Remove from tracking set
        }
        
        lock.unlock();
        
        // Generate terrain for chunks (outside of lock for performance)
        for (const auto& pos : chunksToProcess) {
            std::shared_ptr<Chunk> chunk;
            {
                std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
                auto it = m_chunks.find(pos);
                if (it != m_chunks.end() && it->second) {
                    // Create a safe shared_ptr that doesn't affect the unique_ptr ownership
                    chunk = std::shared_ptr<Chunk>(it->second.get(), [](Chunk*) {}); // Non-owning shared_ptr
                }
            }
            
            if (chunk && chunk->needsGeneration()) {
                auto startTime = std::chrono::high_resolution_clock::now();
                
                // ⚡ BACKGROUND THREAD: Only do CPU-intensive work here
                chunk->generateTerrainOnly();  // Generate terrain blocks only
                
                // Mark as ready for mesh building (which will happen on main thread)
                chunk->markReadyForUpload();   // Flag as ready for GPU upload
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
                
                // Removed debug output for cleaner console
            }
        }
    }
}

int World::getLoadedChunkCount() const {
    return static_cast<int>(m_chunks.size());
}

int World::getRequiredChunkCount(const glm::vec3& playerPosition) const {
    glm::ivec2 playerChunk = worldToChunkPosition(playerPosition);
    return static_cast<int>(getChunksInRange(playerChunk, m_renderDistance).size());
}

bool World::isInitialLoadingComplete(const glm::vec3& playerPosition) const {
    glm::ivec2 playerChunk = worldToChunkPosition(playerPosition);
    std::vector<glm::ivec2> requiredChunks = getChunksInRange(playerChunk, m_renderDistance);
    
    // Check if at least 75% of required chunks are loaded and generated
    int generatedCount = 0;
    {
        std::lock_guard<std::mutex> chunkLock(m_chunksMutex);
        for (const auto& pos : requiredChunks) {
            auto it = m_chunks.find(pos);
            if (it != m_chunks.end() && it->second && it->second->isGenerated()) {
                generatedCount++;
            }
        }
    }
    
    return generatedCount >= (requiredChunks.size() * 3) / 4; // 75% threshold
}
