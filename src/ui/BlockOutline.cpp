#include "ui/BlockOutline.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/OpenGL.h"
#include "engine/graphics/Camera.h"
#include "world/World.h"
#include "world/Chunk.h"
#include "world/Block.h"
#include "utils/RaycastUtil.h"
#include <iostream>
#include <cmath>

BlockOutline::BlockOutline() 
    : m_VAO(0), m_VBO(0), m_EBO(0), m_visible(true), m_initialized(false),
      m_hasTarget(false), m_lineWidth(2.0f), m_targetBlock(0, 0, 0) {
}

BlockOutline::~BlockOutline() {
    cleanup();
}

bool BlockOutline::initialize() {
    if (m_initialized) {
        return true;
    }
    
    m_shader = std::make_unique<Shader>();
    
    std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    std::string fragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 outlineColor;
        uniform float alpha;
        
        void main() {
            FragColor = vec4(outlineColor, alpha);
        }
    )";
    
    if (!m_shader->loadFromString(vertexSource, fragmentSource)) {
        std::cout << "Failed to create block outline shader!" << std::endl;
        return false;
    }
    
    setupGeometry();
    m_initialized = true;
    
    std::cout << "BlockOutline initialized successfully" << std::endl;
    return true;
}

void BlockOutline::setupGeometry() {
    float vertices[] = {
        // Bottom face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        
        // Top face
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };
    
    unsigned int indices[] = {
        // Bottom face edges
        0, 1,  1, 2,  2, 3,  3, 0,
        // Top face edges  
        4, 5,  5, 6,  6, 7,  7, 4,
        // Vertical edges
        0, 4,  1, 5,  2, 6,  3, 7
    };
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BlockOutline::updateTargetBlock(const Camera& camera, const World& world, float maxDistance) {
    glm::vec3 rayOrigin = camera.getPosition();
    glm::vec3 rayDirection = camera.getFront();
    
    RaycastUtil::RaycastResult result = RaycastUtil::raycast(rayOrigin, rayDirection, world, maxDistance);
    
    updateFromRaycast(result);
}

void BlockOutline::updateFromRaycast(const RaycastUtil::RaycastResult& result) {
    if (result.hit) {
        m_targetBlock = result.blockPos;
        m_hitPoint = result.hitPoint;  // Store the actual hit point
        m_hasTarget = true;
    } else {
        m_hasTarget = false;
    }
}

void BlockOutline::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& outlineColor) {
    if (!m_initialized || !m_visible || !m_hasTarget || !m_shader) {
        return;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(m_lineWidth);
    
    m_shader->use();
    
    // Position outline at block center using the hit point to determine the exact center
    glm::vec3 renderPos;
    
    // DIAGNOSTIC APPROACH: Position exactly at the target block coordinates (no offset)
    // This should align with the corner of the block mesh if that's where it's positioned
    renderPos = glm::vec3(m_targetBlock);
    
    // Position outline at block center
    #ifdef DEBUG_OUTLINE_POSITIONING
    static int outlineDebugCounter = 0;
    if (outlineDebugCounter++ % 60 == 0) {
        std::cout << "[DEBUG OUTLINE] Target Block: (" << m_targetBlock.x << ", " << m_targetBlock.y << ", " << m_targetBlock.z << ")" << std::endl;
        std::cout << "[DEBUG OUTLINE] Hit Point: (" << m_hitPoint.x << ", " << m_hitPoint.y << ", " << m_hitPoint.z << ")" << std::endl;
        std::cout << "[DEBUG OUTLINE] Render Position: (" << renderPos.x << ", " << renderPos.y << ", " << renderPos.z << ")" << std::endl;
    }
    #endif
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, renderPos);
    model = glm::scale(model, glm::vec3(1.01f)); // Slightly larger to avoid z-fighting
    
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    m_shader->setVec3("outlineColor", outlineColor);
    m_shader->setFloat("alpha", 0.8f);
    
    glBindVertexArray(m_VAO);
    glDrawElements(GL_LINES, INDEX_COUNT, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void BlockOutline::cleanup() {
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