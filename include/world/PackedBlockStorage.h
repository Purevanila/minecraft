#pragma once
#include "world/Block.h"
#include <vector>
#include <cstdint>

/**
 * Ultra-efficient block storage using bit packing
 * Reduces memory usage by ~75% and improves cache performance
 */
class PackedBlockStorage {
public:
    PackedBlockStorage();
    
    // Get/set block types efficiently
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);
    
    // Fast validity check
    bool isValidPosition(int x, int y, int z) const;
    
    // Memory usage info
    size_t getMemoryUsage() const;
    
private:
    // Use 4-bit per block (16 possible block types)
    std::vector<uint8_t> m_packedData;
    
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int CHUNK_HEIGHT = 256;
    static constexpr int TOTAL_BLOCKS = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
    static constexpr int PACKED_SIZE = (TOTAL_BLOCKS + 1) / 2; // 2 blocks per byte
    
    int getIndex(int x, int y, int z) const;
};

inline PackedBlockStorage::PackedBlockStorage() {
    m_packedData.resize(PACKED_SIZE, 0); // All air blocks initially
}

inline bool PackedBlockStorage::isValidPosition(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_SIZE && 
           y >= 0 && y < CHUNK_HEIGHT && 
           z >= 0 && z < CHUNK_SIZE;
}

inline int PackedBlockStorage::getIndex(int x, int y, int z) const {
    return x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
}

inline BlockType PackedBlockStorage::getBlock(int x, int y, int z) const {
    if (!isValidPosition(x, y, z)) return BlockType::AIR;
    
    int index = getIndex(x, y, z);
    int byteIndex = index / 2;
    bool isLowNibble = (index % 2) == 0;
    
    uint8_t byte = m_packedData[byteIndex];
    uint8_t blockValue = isLowNibble ? (byte & 0x0F) : ((byte & 0xF0) >> 4);
    
    return static_cast<BlockType>(blockValue);
}

inline void PackedBlockStorage::setBlock(int x, int y, int z, BlockType type) {
    if (!isValidPosition(x, y, z)) return;
    
    int index = getIndex(x, y, z);
    int byteIndex = index / 2;
    bool isLowNibble = (index % 2) == 0;
    
    uint8_t blockValue = static_cast<uint8_t>(type) & 0x0F; // Ensure 4-bit
    
    if (isLowNibble) {
        m_packedData[byteIndex] = (m_packedData[byteIndex] & 0xF0) | blockValue;
    } else {
        m_packedData[byteIndex] = (m_packedData[byteIndex] & 0x0F) | (blockValue << 4);
    }
}

inline size_t PackedBlockStorage::getMemoryUsage() const {
    return m_packedData.size();
}
