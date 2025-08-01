#pragma once
#include "entities/Entity.h"
#include "world/Block.h"
#include <memory>

class Texture;
class Shader;
class World;

class ItemEntity : public Entity {
public:
    ItemEntity(const glm::vec3& position, BlockType blockType, World* world);
    
    void update(float deltaTime) override;
    void render() override;
    
    BlockType getBlockType() const { return m_blockType; }
    
    // For collection by player
    bool canBeCollected() const;
    void setCollected() { m_collected = true; }
    bool isCollected() const { return m_collected; }
    
    // Physics properties
    float getCollectionRadius() const { return m_collectionRadius; }
    
private:
    BlockType m_blockType;
    float m_rotationY;
    float m_bobOffset;
    float m_bobSpeed;
    float m_timeAlive;
    float m_collectionRadius;
    bool m_collected;
    bool m_onGround;
    
    // World reference for collision detection
    World* m_world;
    
    // Rendering
    std::shared_ptr<Texture> m_texture;
    std::shared_ptr<Shader> m_shader;
    unsigned int m_VAO, m_VBO, m_EBO;
    
    void initializeRenderData();
    void applyPhysics(float deltaTime);
    std::shared_ptr<Texture> getTextureForBlockType(BlockType blockType);
};
