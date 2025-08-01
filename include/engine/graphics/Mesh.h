#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    
    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& norm)
        : position(pos), texCoords(tex), normal(norm) {}
};

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void setVertices(const std::vector<Vertex>& vertices);
    void setIndices(const std::vector<unsigned int>& indices);
    void upload();
    void render() const;
    void clear();
    
    // Utility methods for block rendering
    static std::vector<Vertex> generateCubeVertices(const glm::vec3& position, float size = 1.0f);
    static std::vector<unsigned int> generateCubeIndices(unsigned int baseIndex = 0);
    
private:
    unsigned int m_VAO, m_VBO, m_EBO;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    bool m_uploaded;
    
    void setupMesh();
};
