#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "utils/RaycastUtil.h"

class Shader;
class Camera;
class World;

class BlockOutline {
public:
    BlockOutline();
    ~BlockOutline();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& outlineColor = glm::vec3(1.0f, 1.0f, 1.0f));
    void cleanup();
    
    void updateTargetBlock(const Camera& camera, const World& world, float maxDistance = 5.0f);
    void updateFromRaycast(const RaycastUtil::RaycastResult& result);
    
    void setVisible(bool visible) { m_visible = visible; }
    void setLineWidth(float width) { m_lineWidth = width; }
    
    bool hasTarget() const { return m_hasTarget; }
    glm::ivec3 getTargetBlock() const { return m_targetBlock; }
    
private:
    void setupGeometry();
    
    unsigned int m_VAO, m_VBO, m_EBO;
    std::unique_ptr<Shader> m_shader;
    
    bool m_visible = true;
    bool m_initialized = false;
    bool m_hasTarget = false;
    float m_lineWidth = 2.0f;
    glm::ivec3 m_targetBlock;
    glm::vec3 m_hitPoint;  // Store the actual raycast hit point
    
    static constexpr int VERTEX_COUNT = 8;
    static constexpr int INDEX_COUNT = 24;
};