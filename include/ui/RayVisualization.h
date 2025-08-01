#pragma once

#include <glm/glm.hpp>
#include <memory>

class Shader;
class Camera;
class World;

/**
 * Visualizes raycast lines for debugging purposes
 * Renders a line from the camera position to the raycast hit point
 */
class RayVisualization {
public:
    RayVisualization();
    ~RayVisualization();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();
    
    // Update the ray visualization with new raycast data
    void updateRay(const glm::vec3& rayStart, const glm::vec3& rayDirection, float maxDistance, bool hit, const glm::vec3& hitPoint = glm::vec3(0));
    
    // Configuration
    void setVisible(bool visible) { m_visible = visible; }
    void setRayColor(const glm::vec3& color) { m_rayColor = color; }
    void setHitColor(const glm::vec3& color) { m_hitColor = color; }
    void setLineWidth(float width) { m_lineWidth = width; }
    
    bool isVisible() const { return m_visible; }
    
private:
    void setupGeometry();
    void updateLineGeometry();
    
    unsigned int m_VAO, m_VBO;
    std::unique_ptr<Shader> m_shader;
    
    // Ray data
    glm::vec3 m_rayStart;
    glm::vec3 m_rayEnd;
    bool m_hasRay;
    bool m_rayHit;
    
    // Rendering properties
    bool m_visible;
    bool m_initialized;
    glm::vec3 m_rayColor;   // Color for ray line
    glm::vec3 m_hitColor;   // Color for hit point
    float m_lineWidth;
    
    // Geometry data
    static constexpr int MAX_VERTICES = 6; // 2 vertices for line + 4 for hit point cross
};
