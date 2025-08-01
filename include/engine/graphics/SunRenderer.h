#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class Texture;

/**
 * SunRenderer - Renders a moving sun in the sky
 * Creates a billboard sun that moves across the sky in a circular path
 * Uses the sun.png texture for authentic Minecraft-style sun
 */
class SunRenderer {
public:
    SunRenderer();
    ~SunRenderer();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& cameraPos);
    void cleanup();
    
    // Configuration
    void setSunSize(float size) { m_sunSize = size; }
    void setSunDistance(float distance) { m_sunDistance = distance; }
    void setSunSpeed(float speed) { m_sunSpeed = speed; }
    void setSunHeight(float height) { m_sunHeight = height; }
    
    // Get sun position for lighting calculations
    glm::vec3 getSunPosition(float time) const;
    glm::vec3 getSunDirection(float time) const;
    float getSunIntensity(float time) const;
    
private:
    void createSunGeometry();
    glm::vec3 calculateSunPosition(float time) const;
    
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Texture> m_sunTexture;
    
    // OpenGL objects
    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;
    
    // Sun parameters
    float m_sunSize = 60.0f;          // Size of the sun billboard (3x bigger)
    float m_sunDistance = 1000.0f;    // Much farther distance to appear like skybox
    float m_sunSpeed = 0.02f;         // Revolution speed (much slower for realistic day/night)
    float m_sunHeight = 200.0f;       // Height offset for sun path
    
    bool m_initialized;
};
