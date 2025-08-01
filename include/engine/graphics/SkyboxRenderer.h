#pragma once

#include "engine/graphics/Shader.h"
#include <glm/glm.hpp>
#include <memory>

class SkyboxRenderer {
public:
    SkyboxRenderer();
    ~SkyboxRenderer();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection, float time);
    void cleanup();

private:
    void createSkyboxGeometry();
    
    std::shared_ptr<Shader> m_shader;
    unsigned int m_VAO;
    unsigned int m_VBO;
    bool m_initialized;
};
