#include "engine/graphics/SkyboxRenderer.h"
#include "engine/graphics/OpenGL.h"
#include "engine/AssetManager.h"
#include <iostream>

SkyboxRenderer::SkyboxRenderer() 
    : m_VAO(0), m_VBO(0), m_initialized(false) {
}

SkyboxRenderer::~SkyboxRenderer() {
    cleanup();
}

bool SkyboxRenderer::initialize() {
    if (m_initialized) {
        return true;
    }
    
    std::cout << "Initializing skybox renderer..." << std::endl;
    
    // Load skybox shader using AssetManager
    m_shader = AssetManager::getInstance().loadShader(
        "assets/shaders/skybox.vert", 
        "assets/shaders/skybox.frag"
    );
    
    if (!m_shader) {
        std::cerr << "Failed to load skybox shader!" << std::endl;
        return false;
    }
    
    // Create skybox geometry
    createSkyboxGeometry();
    
    m_initialized = true;
    std::cout << "Skybox renderer initialized successfully" << std::endl;
    return true;
}

void SkyboxRenderer::createSkyboxGeometry() {
    // Skybox vertices - a simple cube
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // Generate and bind VAO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    
    // Fill VBO with vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    // Set vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

void SkyboxRenderer::render(const glm::mat4& view, const glm::mat4& projection, float time) {
    if (!m_initialized) {
        return;
    }
    
    // Change depth function so depth test passes when values are equal to depth buffer's content
    glDepthFunc(GL_LEQUAL);
    
    m_shader->use();
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    m_shader->setFloat("time", time);
    
    // Render skybox cube
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Set depth function back to default
    glDepthFunc(GL_LESS);
}

void SkyboxRenderer::cleanup() {
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
