#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "ui/DigitRenderer.h"
#include "engine/graphics/OpenGLResource.h"

class Shader;

class LoadingScreen {
public:
    LoadingScreen();
    ~LoadingScreen();
    
    bool initialize();
    void render(int windowWidth, int windowHeight, int chunksLoaded, int totalChunks, const std::string& status = "Loading...");
    void cleanup();
    
private:
    void renderProgressBar(float x, float y, float width, float height, float progress, int windowWidth, int windowHeight);
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight);
    void renderCenteredText(const std::string& text, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight);
    void renderNumber(int number, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight);
    void renderDigit(int digit, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight);
    
    OpenGL::VertexArray m_vao;
    OpenGL::Buffer m_vbo;
    std::unique_ptr<Shader> m_shader;
};
