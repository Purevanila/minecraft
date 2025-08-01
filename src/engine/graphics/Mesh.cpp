#include "engine/graphics/Mesh.h"
#include "engine/graphics/OpenGL.h"

Mesh::Mesh() : m_VAO(0), m_VBO(0), m_EBO(0), m_uploaded(false) {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
}

Mesh::~Mesh() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

void Mesh::setVertices(const std::vector<Vertex>& vertices) {
    m_vertices = vertices;
    m_uploaded = false;
}

void Mesh::setIndices(const std::vector<unsigned int>& indices) {
    m_indices = indices;
    m_uploaded = false;
}

void Mesh::upload() {
    if (m_vertices.empty()) return;
    
    glBindVertexArray(m_VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data if available
    if (!m_indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
    }
    
    setupMesh();
    m_uploaded = true;
    
    glBindVertexArray(0);
}

void Mesh::render() const {
    if (!m_uploaded || m_vertices.empty()) return;
    
    glBindVertexArray(m_VAO);
    
    if (!m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
    }
    
    glBindVertexArray(0);
}

void Mesh::clear() {
    m_vertices.clear();
    m_indices.clear();
    m_uploaded = false;
}

void Mesh::setupMesh() {
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(1);
    
    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
}

std::vector<Vertex> Mesh::generateCubeVertices(const glm::vec3& position, float size) {
    float half = size * 0.5f;
    glm::vec3 pos = position;
    
    return {
        // Front face
        Vertex(pos + glm::vec3(-half, -half,  half), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        Vertex(pos + glm::vec3( half, -half,  half), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        Vertex(pos + glm::vec3( half,  half,  half), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        Vertex(pos + glm::vec3(-half,  half,  half), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        
        // Back face
        Vertex(pos + glm::vec3(-half, -half, -half), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        Vertex(pos + glm::vec3(-half,  half, -half), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        Vertex(pos + glm::vec3( half,  half, -half), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        Vertex(pos + glm::vec3( half, -half, -half), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        
        // Left face
        Vertex(pos + glm::vec3(-half,  half,  half), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3(-half,  half, -half), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3(-half, -half, -half), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3(-half, -half,  half), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
        
        // Right face
        Vertex(pos + glm::vec3( half,  half,  half), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3( half, -half,  half), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3( half, -half, -half), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        Vertex(pos + glm::vec3( half,  half, -half), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
        
        // Top face
        Vertex(pos + glm::vec3(-half,  half, -half), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(pos + glm::vec3(-half,  half,  half), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(pos + glm::vec3( half,  half,  half), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(pos + glm::vec3( half,  half, -half), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        
        // Bottom face
        Vertex(pos + glm::vec3(-half, -half, -half), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        Vertex(pos + glm::vec3( half, -half, -half), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        Vertex(pos + glm::vec3( half, -half,  half), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        Vertex(pos + glm::vec3(-half, -half,  half), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
    };
}

std::vector<unsigned int> Mesh::generateCubeIndices(unsigned int baseIndex) {
    return {
        // Front face
        baseIndex + 0, baseIndex + 1, baseIndex + 2, baseIndex + 2, baseIndex + 3, baseIndex + 0,
        // Back face
        baseIndex + 4, baseIndex + 5, baseIndex + 6, baseIndex + 6, baseIndex + 7, baseIndex + 4,
        // Left face
        baseIndex + 8, baseIndex + 9, baseIndex + 10, baseIndex + 10, baseIndex + 11, baseIndex + 8,
        // Right face
        baseIndex + 12, baseIndex + 13, baseIndex + 14, baseIndex + 14, baseIndex + 15, baseIndex + 12,
        // Top face
        baseIndex + 16, baseIndex + 17, baseIndex + 18, baseIndex + 18, baseIndex + 19, baseIndex + 16,
        // Bottom face
        baseIndex + 20, baseIndex + 21, baseIndex + 22, baseIndex + 22, baseIndex + 23, baseIndex + 20
    };
}
