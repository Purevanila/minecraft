#include "ui/Crosshair.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/OpenGL.h"
#include <iostream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

Crosshair::Crosshair() 
    : m_VAO(0), m_VBO(0), m_EBO(0), m_color(1.0f, 1.0f, 1.0f), 
      m_size(12.0f), m_thickness(2.0f), 
      m_visible(true), m_initialized(false) {
}

Crosshair::~Crosshair() {
    cleanup();
}

bool Crosshair::initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // Create simple shader for crosshair rendering
        m_shader = std::make_unique<Shader>();
        
        // Simple vertex shader - just positions in screen space
        std::string vertexSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            
            uniform mat4 projection;
            
            void main() {
                gl_Position = projection * vec4(aPos, 0.0, 1.0);
            }
        )";
        
        // Simple fragment shader - solid color
        std::string fragmentSource = R"(
            #version 330 core
            out vec4 FragColor;
            
            uniform vec3 color;
            
            void main() {
                FragColor = vec4(color, 1.0);
            }
        )";
        
        if (!m_shader->loadFromString(vertexSource, fragmentSource)) {
            return false;
        }
        
        if (!setupGeometry()) {
            cleanup();
            return false;
        }
        
        m_initialized = true;
        return true;
    } catch (...) {
        cleanup();
        return false;
    }
}

bool Crosshair::setupGeometry() {
    try {
        // Create crosshair geometry as two simple rectangles forming a cross
        // Each rectangle defined by 4 vertices, total 8 vertices
        
        const float halfSize = m_size;
        const float halfThickness = m_thickness / 2.0f;
        
        const float vertices[] = {
            // Horizontal line rectangle (4 vertices)
            -halfSize, -halfThickness,  // Bottom left
             halfSize, -halfThickness,  // Bottom right  
             halfSize,  halfThickness,  // Top right
            -halfSize,  halfThickness,  // Top left
            
            // Vertical line rectangle (4 vertices)
            -halfThickness, -halfSize,  // Bottom left
             halfThickness, -halfSize,  // Bottom right
             halfThickness,  halfSize,  // Top right
            -halfThickness,  halfSize   // Top left
        };
        
        // Indices for drawing two rectangles using triangles
        const unsigned int indices[] = {
            // Horizontal rectangle
            0, 1, 2,   2, 3, 0,
            
            // Vertical rectangle
            4, 5, 6,   6, 7, 4
        };
        
        glGenVertexArrays(1, &m_VAO);
        if (m_VAO == 0) {
            return false;
        }
        
        glGenBuffers(1, &m_VBO);
        if (m_VBO == 0) {
            return false;
        }
        
        glGenBuffers(1, &m_EBO);
        if (m_EBO == 0) {
            return false;
        }
        
        glBindVertexArray(m_VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        // Check for OpenGL errors
        return glGetError() == GL_NO_ERROR;
    } catch (...) {
        return false;
    }
}

void Crosshair::render(int windowWidth, int windowHeight) {
    if (!m_initialized || !m_visible || !m_shader) {
        return;
    }
    
    // Save current OpenGL state
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint blendSrc, blendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);
    
    // Enable blending for crosshair transparency/anti-aliasing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use crosshair shader
    m_shader->use();
    
    // Create orthographic projection matrix for screen-space rendering
    // Map screen coordinates to normalized device coordinates
    const float left = -windowWidth / 2.0f;
    const float right = windowWidth / 2.0f;
    const float bottom = -windowHeight / 2.0f;
    const float top = windowHeight / 2.0f;
    
    glm::mat4 projection = glm::ortho(left, right, bottom, top);
    
    m_shader->setMat4("projection", projection);
    m_shader->setVec3("color", m_color);
    
    // Render crosshair
    glBindVertexArray(m_VAO);
    
    // Draw both rectangles (horizontal and vertical lines) using indices
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    
    // Restore previous OpenGL state
    if (!blendEnabled) {
        glDisable(GL_BLEND);
    } else {
        glBlendFunc(blendSrc, blendDst);
    }
    
    // Render complete
}

void Crosshair::cleanup() {
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
    m_initialized = false;
}
