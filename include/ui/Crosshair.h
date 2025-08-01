#pragma once
#include <memory>
#include <glm/glm.hpp>

class Shader;

/**
 * Simple crosshair UI element for the center of the screen
 * Renders a Minecraft-style crosshair using OpenGL
 */
class Crosshair {
public:
    Crosshair();
    ~Crosshair();
    
    bool initialize();
    void render(int windowWidth, int windowHeight);
    void cleanup();
    
    // Configuration
    void setColor(const glm::vec3& color) { m_color = color; }
    void setSize(float size) { m_size = size; }
    void setThickness(float thickness) { m_thickness = thickness; }
    void setVisible(bool visible) { m_visible = visible; }
    
private:
    bool setupGeometry();
    
    unsigned int m_VAO, m_VBO, m_EBO;
    std::unique_ptr<Shader> m_shader;
    
    // Crosshair properties
    glm::vec3 m_color;
    float m_size;        // Half-length of crosshair arms
    float m_thickness;   // Thickness of crosshair lines
    bool m_visible;
    bool m_initialized;
};
