#pragma once

#include "world/Block.h"
#include "engine/graphics/Mesh.h"
#include <glm/glm.hpp>
#include <vector>

/**
 * Greedy Meshing Algorithm for Minecraft-style voxel terrain
 * Reduces triangle count by 70-90% compared to naive cube generation
 * 
 * Instead of generating 6 faces per block, this algorithm:
 * 1. Groups adjacent identical faces into larger rectangles
 * 2. Eliminates hidden faces between solid blocks
 * 3. Creates optimized meshes with minimal overdraw
 */
class GreedyMeshing {
public:
    struct Face {
        glm::ivec3 position;
        glm::ivec2 size;        // Width and height of the merged face
        BlockType blockType;
        int direction;          // 0-5 for cube faces
        
        Face(const glm::ivec3& pos, const glm::ivec2& sz, BlockType type, int dir)
            : position(pos), size(sz), blockType(type), direction(dir) {}
    };
    
    // Generate optimized mesh using greedy meshing algorithm
    static void generateMesh(
        const std::vector<BlockType>& blocks,
        int chunkSize, int chunkHeight,
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices
    );
    
    // Generate mesh for specific block type (for separate material rendering)
    static void generateMeshForBlockType(
        const std::vector<BlockType>& blocks,
        int chunkSize, int chunkHeight,
        BlockType targetType,
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices
    );

private:
    // Core greedy meshing algorithm - merges faces in 2D slices
    static std::vector<Face> greedyMesh(
        const std::vector<BlockType>& blocks,
        int chunkSize, int chunkHeight,
        BlockType targetType = BlockType::AIR  // AIR means all block types
    );
    
    // Check if a block face should be rendered
    static bool shouldRenderFace(
        const std::vector<BlockType>& blocks,
        int x, int y, int z, int direction,
        int chunkSize, int chunkHeight
    );
    
    // Get block at position with bounds checking
    static BlockType getBlock(
        const std::vector<BlockType>& blocks,
        int x, int y, int z,
        int chunkSize, int chunkHeight
    );
    
    // Convert faces to actual mesh geometry
    static void facesToMesh(
        const std::vector<Face>& faces,
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices
    );
    
    // Direction vectors for cube faces
    static const glm::ivec3 FACE_DIRECTIONS[6];
    static const glm::vec3 FACE_NORMALS[6];
};
