#pragma once

#include "world/Block.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

class Shader;
class Texture;

/**
 * GPU Instanced Rendering System
 * Renders multiple identical objects (like grass, flowers, debris) in a single draw call
 * Massive performance improvement for repeated geometry
 */
class InstancedRenderer {
public:
    struct InstanceData {
        glm::mat4 modelMatrix;
        glm::vec4 color;        // For tinting/variation
        glm::vec2 texOffset;    // For texture atlas variation
    };
    
    InstancedRenderer();
    ~InstancedRenderer();
    
    bool initialize();
    void cleanup();
    
    // Add an instance to be rendered
    void addInstance(BlockType blockType, const glm::mat4& transform, 
                    const glm::vec4& color = glm::vec4(1.0f),
                    const glm::vec2& texOffset = glm::vec2(0.0f));
    
    // Render all instances of all types
    void renderAll(const glm::mat4& view, const glm::mat4& projection);
    
    // Render instances of a specific block type
    void renderBlockType(BlockType blockType, const glm::mat4& view, const glm::mat4& projection);
    
    // Clear all instances (call each frame)
    void clear();
    
    // Update instance data on GPU (call after adding all instances)
    void updateInstanceData();
    
    // Performance statistics
    int getTotalInstances() const;
    int getDrawCalls() const { return m_drawCalls; }

private:
    struct InstanceGroup {
        std::vector<InstanceData> instances;
        unsigned int instanceVBO;
        unsigned int VAO, VBO, EBO;
        std::shared_ptr<Texture> texture;
        bool needsUpdate;
        
        InstanceGroup() : instanceVBO(0), VAO(0), VBO(0), EBO(0), needsUpdate(true) {}
    };
    
    std::unordered_map<BlockType, std::unique_ptr<InstanceGroup>> m_instanceGroups;
    std::unique_ptr<Shader> m_instancedShader;
    
    int m_drawCalls;
    bool m_initialized;
    
    // Create base geometry for a block type
    void createBlockGeometry(BlockType blockType, InstanceGroup& group);
    
    // Setup instance data attributes
    void setupInstanceAttributes(InstanceGroup& group);
    
    // Update a specific instance group's GPU data
    void updateInstanceGroup(InstanceGroup& group);
};
