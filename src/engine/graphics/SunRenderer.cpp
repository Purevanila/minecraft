#include "engine/graphics/SunRenderer.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/Texture.h"
#include "engine/AssetManager.h"
#include "engine/graphics/OpenGL.h"
#include <iostream>
#include <cmath>

SunRenderer::SunRenderer() 
    : m_VAO(0), m_VBO(0), m_EBO(0), m_initialized(false) {
}

SunRenderer::~SunRenderer() {
    cleanup();
}

bool SunRenderer::initialize() {
    if (m_initialized) {
        return true;
    }
    
    std::cout << "Initializing sun renderer..." << std::endl;
    
    // Load sun shader - we'll create new shaders for the sun
    m_shader = AssetManager::getInstance().loadShader(
        "assets/shaders/sun.vert", 
        "assets/shaders/sun.frag"
    );
    
    if (!m_shader) {
        std::cerr << "Failed to load sun shader!" << std::endl;
        return false;
    }
    
    // Load sun texture
    m_sunTexture = AssetManager::getInstance().loadTexture("assets/textures/sun.png");
    if (!m_sunTexture) {
        std::cerr << "Failed to load sun texture!" << std::endl;
        return false;
    }
    
    // Create sun geometry (billboard quad)
    createSunGeometry();
    
    m_initialized = true;
    std::cout << "Sun renderer initialized successfully" << std::endl;
    return true;
}

void SunRenderer::createSunGeometry() {
    // Create a simple quad for the sun billboard
    // The quad will always face the camera
    float vertices[] = {
        // positions        // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Generate and bind VAO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    // Fill VBO with vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Fill EBO with index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Set vertex attribute pointers
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

glm::vec3 SunRenderer::calculateSunPosition(float time) const {
    // Create a circular path for the sun
    float angle = time * m_sunSpeed;
    
    // Sun moves in a large circle, creating day/night cycle
    float x = cos(angle) * m_sunDistance;
    float y = sin(angle) * m_sunDistance * 0.5f + m_sunHeight; // Flattened circle for realistic sun path
    float z = sin(angle) * m_sunDistance * 0.3f; // Slight depth variation
    
    return glm::vec3(x, y, z);
}

glm::vec3 SunRenderer::getSunPosition(float time) const {
    return calculateSunPosition(time);
}

glm::vec3 SunRenderer::getSunDirection(float time) const {
    // Direction from origin to sun (for lighting calculations)
    glm::vec3 sunPos = calculateSunPosition(time);
    return glm::normalize(sunPos);
}

float SunRenderer::getSunIntensity(float time) const {
    // Sun intensity based on height (dimmer when below horizon)
    glm::vec3 sunPos = calculateSunPosition(time);
    float height = sunPos.y;
    
    // Fade out when below horizon level
    if (height < 0) {
        return 0.1f; // Minimal light during "night"
    }
    
    // Scale intensity based on height
    return glm::clamp(height / m_sunHeight, 0.1f, 1.0f);
}

void SunRenderer::render(const glm::mat4& view, const glm::mat4& projection, float time, const glm::vec3& cameraPos) {
    if (!m_initialized || !m_shader || !m_sunTexture) {
        return;
    }
    
    // Enable blending for sun transparency/glow effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth writing so sun doesn't interfere with other objects
    glDepthMask(GL_FALSE);
    
    m_shader->use();
    
    // Calculate sun direction (relative to sky, not world position)
    glm::vec3 sunDirection = getSunDirection(time);
    
    // Position sun relative to camera (like skybox) - always at far distance
    glm::vec3 sunPos = cameraPos + sunDirection * m_sunDistance;
    
    // Create billboard matrix - sun always faces camera
    glm::vec3 toCamera = -sunDirection; // Sun always faces toward camera
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, toCamera));
    up = glm::cross(toCamera, right);
    
    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sunPos);
    
    // Billboard rotation matrix
    glm::mat4 billboardMatrix = glm::mat4(1.0f);
    billboardMatrix[0] = glm::vec4(right * m_sunSize, 0.0f);
    billboardMatrix[1] = glm::vec4(up * m_sunSize, 0.0f);
    billboardMatrix[2] = glm::vec4(toCamera, 0.0f);
    
    model = model * billboardMatrix;
    
    // Set uniforms
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    m_shader->setFloat("time", time);
    m_shader->setFloat("sunIntensity", getSunIntensity(time));
    
    // Bind sun texture
    m_sunTexture->bind(0);
    m_shader->setInt("sunTexture", 0);
    
    // Render sun quad
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Restore render state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void SunRenderer::cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    
    m_shader.reset();
    m_sunTexture.reset();
    m_initialized = false;
}
