#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <type_traits>

/**
 * High-Performance Memory Pool Allocator
 * Pre-allocates large blocks of memory to avoid frequent malloc/free calls
 * Dramatically reduces memory fragmentation and allocation overhead
 */
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    MemoryPool() = default;
    ~MemoryPool() { cleanup(); }
    
    // Non-copyable, non-movable for safety
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    
    // Allocate an object from the pool
    template<typename... Args>
    T* allocate(Args&&... args) {
        if (m_freeList.empty()) {
            allocateNewBlock();
        }
        
        T* ptr = m_freeList.back();
        m_freeList.pop_back();
        
        // Use placement new to construct the object
        return new(ptr) T(std::forward<Args>(args)...);
    }
    
    // Return an object to the pool
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        // Call destructor
        ptr->~T();
        
        // Add back to free list
        m_freeList.push_back(ptr);
    }
    
    // Get memory usage statistics
    size_t getTotalBlocks() const { return m_blocks.size(); }
    size_t getFreeObjects() const { return m_freeList.size(); }
    size_t getAllocatedObjects() const { 
        return getTotalBlocks() * BlockSize - getFreeObjects(); 
    }
    size_t getMemoryUsage() const { 
        return getTotalBlocks() * BlockSize * sizeof(T); 
    }

private:
    struct Block {
        alignas(T) char data[BlockSize * sizeof(T)];
    };
    
    std::vector<std::unique_ptr<Block>> m_blocks;
    std::vector<T*> m_freeList;
    
    void allocateNewBlock() {
        auto block = std::make_unique<Block>();
        T* blockStart = reinterpret_cast<T*>(block->data);
        
        // Add all objects in this block to the free list
        for (size_t i = 0; i < BlockSize; ++i) {
            m_freeList.push_back(blockStart + i);
        }
        
        m_blocks.push_back(std::move(block));
    }
    
    void cleanup() {
        m_freeList.clear();
        m_blocks.clear();
    }
};

/**
 * Global memory pools for common game objects
 * Use these instead of new/delete for better performance
 */
class GameMemoryPools {
public:
    static MemoryPool<Chunk>& getChunkPool() {
        static MemoryPool<Chunk, 128> pool;  // 128 chunks per block
        return pool;
    }
    
    static MemoryPool<Block>& getBlockPool() {
        static MemoryPool<Block, 4096> pool;  // 4096 blocks per block
        return pool;
    }
    
    // Pool for vertices during mesh building
    static MemoryPool<std::vector<Vertex>>& getVertexPool() {
        static MemoryPool<std::vector<Vertex>, 64> pool;
        return pool;
    }
    
    // Print memory usage statistics
    static void printStatistics();
};
