#include "ui/RayVisualization.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/OpenGL.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

RayVisualization::RayVisualization() 
    : m_VAO(0), m_VBO(0), m_visible(true), m_initialized(false),
      m_hasRay(false), m_rayHit(false), m_lineWidth(2.0f),
      m_rayColor(1.0f, 0.0f, 0.0f), m_hitColor(0.0f, 1.0f, 0.0f),
      m_rayStart(0.0f), m_rayEnd(0.0f) {
}

RayVisualization::~RayVisualization() {
    cleanup();
}

bool RayVisualization::initialize() {
    if (m_initialized) {
        return true;
    }
    
    m_shader = std::make_unique<Shader>();
    
    std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 fragColor;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            fragColor = aColor;
        }
    )";
    
    std::string fragmentSource = R"(
        #version 330 core
        in vec3 fragColor;
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(fragColor, 1.0);
        }
    )";
    
    if (!m_shader->loadFromString(vertexSource, fragmentSource)) {
        std::cout << "Failed to create ray visualization shader!" << std::endl;
        return false;
    }
    
    setupGeometry();
    m_initialized = true;
    std::cout << "RayVisualization initialized successfully" << std::endl;
    return true;
}

void RayVisualization::setupGeometry() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    // Allocate space for vertices (position + color = 6 floats per vertex)
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RayVisualization::updateRay(const glm::vec3& rayStart, const glm::vec3& rayDirection, float maxDistance, bool hit, const glm::vec3& hitPoint) {
    m_rayStart = rayStart;
    m_rayHit = hit;
    
    if (hit) {
        m_rayEnd = hitPoint;
    } else {
        m_rayEnd = rayStart + rayDirection * maxDistance;
    }
    
    m_hasRay = true;
    updateLineGeometry();
}

void RayVisualization::updateLineGeometry() {
    if (!m_hasRay) return;
    
    // Create vertex data: position (3 floats) + color (3 floats) = 6 floats per vertex
    float vertices[12]; // 2 vertices * 6 floats each
    
    // Ray start vertex
    vertices[0] = m_rayStart.x;
    vertices[1] = m_rayStart.y;
    vertices[2] = m_rayStart.z;
    vertices[3] = m_rayColor.r;
    vertices[4] = m_rayColor.g;
    vertices[5] = m_rayColor.b;
    
    // Ray end vertex (use hit color if it hit something, ray color otherwise)
    glm::vec3 endColor = m_rayHit ? m_hitColor : m_rayColor;
    vertices[6] = m_rayEnd.x;
    vertices[7] = m_rayEnd.y;
    vertices[8] = m_rayEnd.z;
    vertices[9] = endColor.r;
    vertices[10] = endColor.g;
    vertices[11] = endColor.b;
    
    // Update VBO with new geometry
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RayVisualization::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_initialized || !m_visible || !m_hasRay || !m_shader) {
        return;
    }
    
    // Save previous line width
    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
    
    glLineWidth(m_lineWidth);
    glDisable(GL_DEPTH_TEST); // Draw ray on top of everything for debugging
    
    m_shader->use();
    
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    
    // Restore previous line width and depth test
    glLineWidth(prevLineWidth);
    glEnable(GL_DEPTH_TEST);
}

void RayVisualization::cleanup() {
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    m_shader.reset();
    m_initialized = false;
}
