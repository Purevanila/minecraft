#include "world/Chunk.h"
#include "world/ModularWorldGenerator.h"
#include "engine/graphics/Mesh.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <chrono>

Chunk::Chunk(const glm::ivec2& position, ModularWorldGenerator* terrainGen, bool autoGenerate) 
    : m_position(position)
    , m_needsRebuild(true)
    , m_terrainGenerator(terrainGen) {
    
    
    m_blockTypes.resize(CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE, BlockType::AIR);
    
    
    m_blocks.resize(CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE);
    
    
    m_mesh = std::make_unique<Mesh>();
    
    m_waterMesh = std::make_unique<Mesh>();
    
    m_oakMesh = std::make_unique<Mesh>();
    
    m_leavesMesh = std::make_unique<Mesh>();
    
    m_stoneMesh = std::make_unique<Mesh>();
    
    m_gravelMesh = std::make_unique<Mesh>();
    
    m_sandMesh = std::make_unique<Mesh>();
    
    // Initialize mesh building flag
    if (autoGenerate) {
        generate();
    }
}


void Chunk::drawWaterMesh() const {
    if (m_waterMesh) {
        m_waterMesh->render();
    }
}


void Chunk::drawOakMesh() const {
    if (m_oakMesh) {
        m_oakMesh->render();
    }
}

void Chunk::drawLeavesMesh() const {
    if (m_leavesMesh) {
        m_leavesMesh->render();
    }
}

void Chunk::drawStoneMesh() const {
    if (m_stoneMesh) {
        m_stoneMesh->render();
    }
}

void Chunk::drawGravelMesh() const {
    if (m_gravelMesh) {
        m_gravelMesh->render();
    }
}

void Chunk::drawSandMesh() const {
    if (m_sandMesh) {
        m_sandMesh->render();
    }
}


void Chunk::markReadyForUpload() {
    m_readyForUpload = true;
}


bool Chunk::needsUpload() const {
    return m_readyForUpload && m_generated;
}


void Chunk::generateTerrainOnly() {
    
    bool expected = false;
    if (!m_generated.compare_exchange_strong(expected, true)) {
        return;  
    }
    
    if (!m_terrainGenerator) {
        generateFlatTerrain();
        return;
    }
    
    
    m_terrainGenerator->generateChunk(*this);
    
    
    m_needsRebuild = true;
}


void Chunk::buildMeshData() {
    
    
    m_needsRebuild = true;
}


void Chunk::uploadMesh() {
    
    
    if (m_needsRebuild) {
        buildMesh();
    }
}

Chunk::~Chunk() {
    
    m_mesh.reset();
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    
    if (!isValidPosition(x, y, z)) {
        return;
    }
    
    
    int index = getIndex(x, y, z);
    
    
    m_blockTypes[index] = type;
    
    
    if (!m_blocks[index]) {
        m_blocks[index] = BlockRegistry::getInstance().createBlock(type);
    } else {
        
        m_blocks[index] = BlockRegistry::getInstance().createBlock(type);
    }
    
    
    m_needsRebuild = true;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!isValidPosition(x, y, z)) return BlockType::AIR;
    
    int index = getIndex(x, y, z);
    return m_blockTypes[index];  
}

const Block& Chunk::getBlockObject(int x, int y, int z) const {
    if (!isValidPosition(x, y, z)) {
        static Block airBlock(BlockType::AIR);
        return airBlock;
    }
    
    int index = getIndex(x, y, z);
    return *m_blocks[index];
}

void Chunk::generate() {
    if (m_generated) return;  
    
    //Create the basic terrain (grass on top, dirt below, stone at bottom)
    generateTerrain();
    
    //Build the 3D mesh that the graphics card will render
    buildMesh();
    
    m_generated.store(true);  //Mark as generated
}

void Chunk::buildMesh() {
    
    if (!m_needsRebuild) return;
    
    
    std::vector<Vertex> solidVertices;
    std::vector<unsigned int> solidIndices;
    std::vector<Vertex> waterVertices;
    std::vector<unsigned int> waterIndices;
    std::vector<Vertex> oakVertices;
    std::vector<unsigned int> oakIndices;
    std::vector<Vertex> leavesVertices;
    std::vector<unsigned int> leavesIndices;
    std::vector<Vertex> stoneVertices;
    std::vector<unsigned int> stoneIndices;
    std::vector<Vertex> gravelVertices;
    std::vector<unsigned int> gravelIndices;
    std::vector<Vertex> sandVertices;
    std::vector<unsigned int> sandIndices;
    
    // ⚡ PERFORMANCE: Reserve larger memory for fewer reallocations
    solidVertices.reserve(16384);    // Double the size to reduce reallocations
    solidIndices.reserve(24576);     
    waterVertices.reserve(4096);     // Increase water capacity
    waterIndices.reserve(6144);      
    oakVertices.reserve(2048);       // More space for trees
    oakIndices.reserve(3072);        
    leavesVertices.reserve(4096);    // Leaves are common
    leavesIndices.reserve(6144);     
    stoneVertices.reserve(8192);     // Stone is very common
    stoneIndices.reserve(12288);     
    gravelVertices.reserve(2048);    
    gravelIndices.reserve(3072);
    sandVertices.reserve(2048);      // Reserve space for sand blocks
    sandIndices.reserve(3072);
    unsigned int solidVertexIndex = 0;
    unsigned int waterVertexIndex = 0;
    unsigned int oakVertexIndex = 0;
    unsigned int leavesVertexIndex = 0;
    unsigned int stoneVertexIndex = 0;
    unsigned int gravelVertexIndex = 0;
    unsigned int sandVertexIndex = 0;
    
    // Static face direction and normal arrays for cube face generation
    static const glm::ivec3 faceDirections[6] = {
        { 0,  0,  1}, { 0,  0, -1}, {-1,  0,  0}, 
        { 1,  0,  0}, { 0,  1,  0}, { 0, -1,  0}
    };
    
    static const glm::vec3 faceNormals[6] = {
        { 0.0f,  0.0f,  1.0f}, { 0.0f,  0.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, 
        { 1.0f,  0.0f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f}
    };
    
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                
                BlockType blockType = getBlockFast(x, y, z);
                
                // Skip air blocks
                if (blockType == BlockType::AIR) continue;
                
                bool completelyHidden = true;
                
                
                if (x > 0 && x < CHUNK_SIZE - 1 && y > 1 && y < CHUNK_HEIGHT - 2 && z > 0 && z < CHUNK_SIZE - 1) {
                    
                    BlockType neighbors[6] = {
                        getBlockFast(x-1, y, z), getBlockFast(x+1, y, z),  
                        getBlockFast(x, y-1, z), getBlockFast(x, y+1, z),  
                        getBlockFast(x, y, z-1), getBlockFast(x, y, z+1)   
                    };
                    
                    
                    for (int i = 0; i < 6; ++i) {
                        BlockType neighborType = neighbors[i];
                        
                        
                        if (neighborType == BlockType::AIR || 
                            neighborType == BlockType::WATER || 
                            neighborType == BlockType::LEAVES) {
                            completelyHidden = false;
                            break;
                        }
                    }
                    
                    
                    if (blockType == BlockType::WATER || blockType == BlockType::LEAVES) {
                        completelyHidden = false;
                    }
                } else {
                    
                    completelyHidden = false;
                }
                
                
                if (completelyHidden) continue;
                
                
                glm::vec3 blockWorldPos = getWorldPosition() + glm::vec3(x, y, z);
                
                // Generate cube faces for this block
                for (int faceIndex = 0; faceIndex < 6; ++faceIndex) {
                    int nx = x + faceDirections[faceIndex].x;
                    int ny = y + faceDirections[faceIndex].y;
                    int nz = z + faceDirections[faceIndex].z;
                    
                    
                    BlockType neighborType = BlockType::AIR;
                    bool isChunkBoundary = false;
                    
                    if (nx >= 0 && nx < CHUNK_SIZE && ny >= 0 && ny < CHUNK_HEIGHT && nz >= 0 && nz < CHUNK_SIZE) {
                        
                        neighborType = getBlockFast(nx, ny, nz);
                    } else {
                        
                        isChunkBoundary = true;
                        
                        
                        
                        
                        neighborType = BlockType::AIR;
                        
                        
                        if (ny < 0) {
                            neighborType = BlockType::STONE;
                        }
                    }
                    
                    
                    bool shouldRenderFace = false;
                    
                    if (isChunkBoundary) {
                        
                        shouldRenderFace = true;
                    } else if (neighborType == BlockType::AIR) {
                        
                        shouldRenderFace = true;
                    } else if (blockType == neighborType) {
                        
                        shouldRenderFace = false;
                    } else if (blockType == BlockType::WATER) {
                        
                        shouldRenderFace = (neighborType != BlockType::WATER);
                    } else if (neighborType == BlockType::WATER) {
                        
                        shouldRenderFace = true;
                    } else if (blockType == BlockType::LEAVES || neighborType == BlockType::LEAVES) {
                        
                        shouldRenderFace = (blockType != neighborType);
                    } else {
                        
                        
                        
                        bool shouldRenderInteriorFace = true; 
                        
                        
                        if (y < 40 && !isChunkBoundary) { 
                            
                            bool similarMaterials = 
                                (blockType == BlockType::STONE && neighborType == BlockType::DIRT) ||
                                (blockType == BlockType::DIRT && neighborType == BlockType::STONE) ||
                                (blockType == BlockType::STONE && neighborType == BlockType::GRAVEL) ||
                                (blockType == BlockType::GRAVEL && neighborType == BlockType::STONE) ||
                                (blockType == BlockType::DIRT && neighborType == BlockType::GRAVEL) ||
                                (blockType == BlockType::GRAVEL && neighborType == BlockType::DIRT) ||
                                (blockType == BlockType::SAND && neighborType == BlockType::DIRT) ||
                                (blockType == BlockType::DIRT && neighborType == BlockType::SAND) ||
                                (blockType == BlockType::SAND && neighborType == BlockType::STONE) ||
                                (blockType == BlockType::STONE && neighborType == BlockType::SAND);
                            
                            if (similarMaterials) {
                                shouldRenderInteriorFace = false; 
                            }
                        }
                        
                        
                        if (blockType == BlockType::OAK_LOG || neighborType == BlockType::OAK_LOG ||
                            blockType == BlockType::GRASS || neighborType == BlockType::GRASS) {
                            shouldRenderInteriorFace = true; 
                        }
                        
                        shouldRenderFace = shouldRenderInteriorFace;
                    }
                    
                    
                    bool isEdgeBlock = (x == 0 || x == CHUNK_SIZE-1 || z == 0 || z == CHUNK_SIZE-1);
                    
                    
                    
                    if (shouldRenderFace && !isEdgeBlock) {
                        
                        glm::vec3 faceCenter = blockWorldPos + glm::vec3(faceDirections[faceIndex]) * 0.5f;
                        glm::vec3 faceNormal = faceNormals[faceIndex];
                        
                        
                        bool likelyBackface = false;
                        
                        
                        if (y < 30) { 
                            
                            if (faceIndex == 5 && y < 20) { 
                                likelyBackface = true;
                            }
                        }
                        
                        
                        if (y > 120) { 
                            
                            if (faceIndex == 4) { 
                                likelyBackface = true;
                            }
                        }
                        
                        
                        
                        
                        
                        
                        
                        if (!likelyBackface && y < 60 && !isEdgeBlock && 
                            x > 1 && x < CHUNK_SIZE-2 && z > 1 && z < CHUNK_SIZE-2) { 
                            
                            bool inSolidFormation = true;
                            for (int dx = -1; dx <= 1 && inSolidFormation; dx++) {
                                for (int dy = -1; dy <= 1 && inSolidFormation; dy++) {
                                    for (int dz = -1; dz <= 1 && inSolidFormation; dz++) {
                                        if (dx == 0 && dy == 0 && dz == 0) continue; 
                                        
                                        int checkX = x + dx, checkY = y + dy, checkZ = z + dz;
                                        if (checkX >= 0 && checkX < CHUNK_SIZE && 
                                            checkY >= 0 && checkY < CHUNK_HEIGHT && 
                                            checkZ >= 0 && checkZ < CHUNK_SIZE) {
                                            BlockType checkType = getBlockFast(checkX, checkY, checkZ);
                                            if (checkType == BlockType::AIR || checkType == BlockType::WATER) {
                                                inSolidFormation = false;
                                            }
                                        } else {
                                            
                                            inSolidFormation = false;
                                        }
                                    }
                                }
                            }
                            
                            
                            if (inSolidFormation) {
                                likelyBackface = true;
                            }
                        }
                        
                        
                        if (likelyBackface) {
                            shouldRenderFace = false;
                        }
                    }
                    
                    
                    if (!shouldRenderFace) continue;
                    
                    
                    if (blockType == BlockType::WATER) {
                        addFaceToMesh(waterVertices, waterIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], waterVertexIndex);
                    } else if (blockType == BlockType::OAK_LOG) {
                        addFaceToMesh(oakVertices, oakIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], oakVertexIndex);
                    } else if (blockType == BlockType::LEAVES) {
                        addFaceToMesh(leavesVertices, leavesIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], leavesVertexIndex);
                    } else if (blockType == BlockType::STONE) {
                        addFaceToMesh(stoneVertices, stoneIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], stoneVertexIndex);
                    } else if (blockType == BlockType::GRAVEL) {
                        addFaceToMesh(gravelVertices, gravelIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], gravelVertexIndex);
                    } else if (blockType == BlockType::SAND) {
                        addFaceToMesh(sandVertices, sandIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], sandVertexIndex);
                    } else {
                        
                        addFaceToMesh(solidVertices, solidIndices, blockWorldPos, faceIndex,
                                      faceNormals[faceIndex], solidVertexIndex);
                    }
                }
            }
        }
    }
    
    
    
    
    m_mesh->clear();
    if (!solidVertices.empty()) {
        m_mesh->setVertices(solidVertices);
        m_mesh->setIndices(solidIndices);
    }
    m_mesh->upload();
    
    
    m_waterMesh->clear();
    if (!waterVertices.empty()) {
        m_waterMesh->setVertices(waterVertices);
        m_waterMesh->setIndices(waterIndices);
    }
    m_waterMesh->upload();
    
    
    m_oakMesh->clear();
    if (!oakVertices.empty()) {
        m_oakMesh->setVertices(oakVertices);
        m_oakMesh->setIndices(oakIndices);
    }
    m_oakMesh->upload();
    
    
    m_leavesMesh->clear();
    if (!leavesVertices.empty()) {
        m_leavesMesh->setVertices(leavesVertices);
        m_leavesMesh->setIndices(leavesIndices);
    }
    m_leavesMesh->upload();
    
    
    m_stoneMesh->clear();
    if (!stoneVertices.empty()) {
        m_stoneMesh->setVertices(stoneVertices);
        m_stoneMesh->setIndices(stoneIndices);
    }
    m_stoneMesh->upload();
    
    
    m_gravelMesh->clear();
    if (!gravelVertices.empty()) {
        m_gravelMesh->setVertices(gravelVertices);
        m_gravelMesh->setIndices(gravelIndices);
    }
    m_gravelMesh->upload();
    
    
    m_sandMesh->clear();
    if (!sandVertices.empty()) {
        m_sandMesh->setVertices(sandVertices);
        m_sandMesh->setIndices(sandIndices);
    }
    m_sandMesh->upload();
    
    m_needsRebuild = false;
}

void Chunk::render(const glm::mat4& view, const glm::mat4& projection) {
    
    if (!m_generated) {
        return;  
    }
    
    
    if (needsUpload()) {
        uploadMesh();
        m_readyForUpload = false;  
    }
    
    
    if (m_needsRebuild) {
        buildMesh();
    }
    
    if (m_mesh) {
        
        
        glm::mat4 model = glm::mat4(1.0f); 
        
        
        
        m_mesh->render();
    }
}

bool Chunk::isValidPosition(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_SIZE && 
           y >= 0 && y < CHUNK_HEIGHT && 
           z >= 0 && z < CHUNK_SIZE;
}

int Chunk::getIndex(int x, int y, int z) const {
    return x + z * CHUNK_SIZE + y * CHUNK_SIZE * CHUNK_SIZE;
}

void Chunk::generateTerrain() {
    
    
    
    if (!m_terrainGenerator) {
        
        generateFlatTerrain();
        return;
    }
    
    
    m_terrainGenerator->generateChunk(*this);
    
    
    m_generated.store(true);
    m_needsRebuild = true;
}

void Chunk::generateFlatTerrain() {
    //Fallback flat terrain generation 
    const int GRASS_LEVEL = 4;
    const int DIRT_LAYERS = 3;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int y = 0; y <= GRASS_LEVEL && y < CHUNK_HEIGHT; ++y) {
                BlockType blockToPlace;
                
                if (y == GRASS_LEVEL) {
                    blockToPlace = BlockType::GRASS;
                } else if (y >= GRASS_LEVEL - DIRT_LAYERS) {
                    blockToPlace = BlockType::DIRT;
                } else {
                    blockToPlace = BlockType::STONE;
                }
                
                setBlock(x, y, z, blockToPlace);
            }
        }
    }
}

void Chunk::addTerrainVariation(int x, int z, int surfaceHeight) {
    
    

}

void Chunk::addFaceToMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                          const glm::vec3& blockPos, int faceIndex, const glm::vec3& normal, 
                          unsigned int& vertexIndex) {
    
    float size = 0.5f; 
    glm::vec3 pos = blockPos;
    
    
    std::vector<Vertex> faceVertices;
    
    switch(faceIndex) {
        case 0: 
            faceVertices = {
                Vertex(pos + glm::vec3(-size, -size,  size), glm::vec2(0.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size, -size,  size), glm::vec2(1.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size,  size,  size), glm::vec2(1.0f, 1.0f), normal),
                Vertex(pos + glm::vec3(-size,  size,  size), glm::vec2(0.0f, 1.0f), normal)
            };
            break;
        case 1: 
            faceVertices = {
                Vertex(pos + glm::vec3(-size, -size, -size), glm::vec2(1.0f, 0.0f), normal),
                Vertex(pos + glm::vec3(-size,  size, -size), glm::vec2(1.0f, 1.0f), normal),
                Vertex(pos + glm::vec3( size,  size, -size), glm::vec2(0.0f, 1.0f), normal),
                Vertex(pos + glm::vec3( size, -size, -size), glm::vec2(0.0f, 0.0f), normal)
            };
            break;
        case 2: 
            faceVertices = {
                Vertex(pos + glm::vec3(-size,  size,  size), glm::vec2(1.0f, 1.0f), normal),
                Vertex(pos + glm::vec3(-size,  size, -size), glm::vec2(0.0f, 1.0f), normal),
                Vertex(pos + glm::vec3(-size, -size, -size), glm::vec2(0.0f, 0.0f), normal),
                Vertex(pos + glm::vec3(-size, -size,  size), glm::vec2(1.0f, 0.0f), normal)
            };
            break;
        case 3: 
            faceVertices = {
                Vertex(pos + glm::vec3( size,  size,  size), glm::vec2(0.0f, 1.0f), normal),
                Vertex(pos + glm::vec3( size, -size,  size), glm::vec2(0.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size, -size, -size), glm::vec2(1.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size,  size, -size), glm::vec2(1.0f, 1.0f), normal)
            };
            break;
        case 4: 
            faceVertices = {
                Vertex(pos + glm::vec3(-size,  size, -size), glm::vec2(0.0f, 1.0f), normal),
                Vertex(pos + glm::vec3(-size,  size,  size), glm::vec2(0.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size,  size,  size), glm::vec2(1.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size,  size, -size), glm::vec2(1.0f, 1.0f), normal)
            };
            break;
        case 5: 
            faceVertices = {
                Vertex(pos + glm::vec3(-size, -size, -size), glm::vec2(0.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size, -size, -size), glm::vec2(1.0f, 0.0f), normal),
                Vertex(pos + glm::vec3( size, -size,  size), glm::vec2(1.0f, 1.0f), normal),
                Vertex(pos + glm::vec3(-size, -size,  size), glm::vec2(0.0f, 1.0f), normal)
            };
            break;
    }
    
    
    vertices.insert(vertices.end(), faceVertices.begin(), faceVertices.end());
    
    
    std::vector<unsigned int> faceIndices = {
        vertexIndex + 0, vertexIndex + 1, vertexIndex + 2,
        vertexIndex + 2, vertexIndex + 3, vertexIndex + 0
    };
    indices.insert(indices.end(), faceIndices.begin(), faceIndices.end());
    
    vertexIndex += 4; 
}
