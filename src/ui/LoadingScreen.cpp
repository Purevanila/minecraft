#include "ui/LoadingScreen.h"
#include "engine/graphics/Shader.h"
#include "engine/graphics/OpenGL.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

LoadingScreen::LoadingScreen() {
}

LoadingScreen::~LoadingScreen() {
    // RAII handles cleanup automatically
}

bool LoadingScreen::initialize() {
    // Create shader for simple 2D rendering
    m_shader = std::make_unique<Shader>();
    
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    if (!m_shader->loadFromString(vertexShaderSource, fragmentShaderSource)) {
        std::cerr << "Failed to compile loading screen shader" << std::endl;
        return false;
    }
    
    // VAO and VBO are automatically generated in their constructors
    m_vao.bind();
    m_vbo.bind(GL_ARRAY_BUFFER);
    
    // Enable position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    m_vao.unbind();
    
    return true;
}

void LoadingScreen::render(int windowWidth, int windowHeight, int chunksLoaded, int totalChunks, const std::string& status) {
    // Set up orthographic projection for 2D rendering
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
    
    // Disable depth testing for overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_shader->use();
    m_shader->setMat4("projection", projection);
    
    // Draw semi-transparent dark background
    glm::vec3 backgroundColor(0.0f, 0.0f, 0.0f);
    m_shader->setVec3("color", backgroundColor);
    
    float vertices[] = {
        0.0f, 0.0f,
        (float)windowWidth, 0.0f,
        (float)windowWidth, (float)windowHeight,
        0.0f, (float)windowHeight
    };
    
    m_vao.bind();
    m_vbo.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Colors
    glm::vec3 whiteColor(1.0f, 1.0f, 1.0f);
    glm::vec3 greenColor(0.0f, 1.0f, 0.0f);
    glm::vec3 grayColor(0.3f, 0.3f, 0.3f);
    
    // Title
    float centerY = windowHeight * 0.3f;
    renderCenteredText("MINECRAFT CLONE", centerY, 4.0f, whiteColor, windowWidth, windowHeight);
    
    // Status text
    centerY += 80.0f;
    renderCenteredText(status, centerY, 2.0f, whiteColor, windowWidth, windowHeight);
    
    // Progress info
    centerY += 60.0f;
    std::string progressText = "Chunks: ";
    
    // Calculate center position for the progress text
    float textWidth = progressText.length() * 30.0f * 1.5f; // Approximate text width
    float numberWidth = 60.0f; // Space for two numbers
    float totalWidth = textWidth + numberWidth * 2 + 30.0f; // Space for " / "
    float startX = (windowWidth - totalWidth) / 2.0f;
    
    // Render "Chunks: "
    renderText(progressText, startX, centerY, 1.5f, whiteColor, windowWidth, windowHeight);
    startX += textWidth;
    
    // Render loaded count
    renderNumber(chunksLoaded, startX, centerY, 1.5f, greenColor, windowWidth, windowHeight);
    startX += numberWidth;
    
    // Render " / "
    renderText(" / ", startX, centerY, 1.5f, whiteColor, windowWidth, windowHeight);
    startX += 30.0f;
    
    // Render total count
    renderNumber(totalChunks, startX, centerY, 1.5f, whiteColor, windowWidth, windowHeight);
    
    // Progress bar
    centerY += 80.0f;
    float barWidth = windowWidth * 0.6f;
    float barHeight = 20.0f;
    float barX = (windowWidth - barWidth) / 2.0f;
    float progress = totalChunks > 0 ? (float)chunksLoaded / (float)totalChunks : 0.0f;
    
    renderProgressBar(barX, centerY, barWidth, barHeight, progress, windowWidth, windowHeight);
    
    // Percentage
    centerY += 50.0f;
    int percentage = (int)(progress * 100.0f);
    std::string percentText = std::to_string(percentage) + "%";
    renderCenteredText(percentText, centerY, 2.0f, greenColor, windowWidth, windowHeight);
    
    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void LoadingScreen::renderProgressBar(float x, float y, float width, float height, float progress, int windowWidth, int windowHeight) {
    // Background bar (gray)
    glm::vec3 grayColor(0.3f, 0.3f, 0.3f);
    m_shader->setVec3("color", grayColor);
    
    float bgVertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    
    m_vao.bind();
    m_vbo.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Progress bar (green)
    if (progress > 0.0f) {
        glm::vec3 greenColor(0.0f, 1.0f, 0.0f);
        m_shader->setVec3("color", greenColor);
        
        float progressWidth = width * progress;
        float fgVertices[] = {
            x, y,
            x + progressWidth, y,
            x + progressWidth, y + height,
            x, y + height
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(fgVertices), fgVertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}

void LoadingScreen::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight) {
    float currentX = x;
    for (char c : text) {
        if (c >= '0' && c <= '9') {
            renderDigit(c - '0', currentX, y, scale, color, windowWidth, windowHeight);
            currentX += 30.0f * scale;
        } else {
            // For non-digits, just advance the position
            currentX += 20.0f * scale;
        }
    }
}

void LoadingScreen::renderCenteredText(const std::string& text, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight) {
    float textWidth = text.length() * 20.0f * scale; // Approximate width
    float x = (windowWidth - textWidth) / 2.0f;
    renderText(text, x, y, scale, color, windowWidth, windowHeight);
}

void LoadingScreen::renderNumber(int number, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight) {
    if (number == 0) {
        renderDigit(0, x, y, scale, color, windowWidth, windowHeight);
        return;
    }
    
    // Count digits
    int temp = number;
    int digitCount = 0;
    while (temp > 0) {
        digitCount++;
        temp /= 10;
    }
    
    // Render digits from left to right
    temp = number;
    for (int i = digitCount - 1; i >= 0; i--) {
        int divisor = 1;
        for (int j = 0; j < i; j++) {
            divisor *= 10;
        }
        int digit = (temp / divisor) % 10;
        renderDigit(digit, x + (digitCount - 1 - i) * 30.0f * scale, y, scale, color, windowWidth, windowHeight);
    }
}

void LoadingScreen::renderDigit(int digit, float x, float y, float scale, const glm::vec3& color, int windowWidth, int windowHeight) {
    if (digit < 0 || digit > 9) return;
    
    m_shader->setVec3("color", color);
    
    m_vao.bind();
    
    // Render each pixel of the digit
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (UI::getDigitPixel(digit, col, row)) {
                float pixelX = x + col * 4.0f * scale;
                float pixelY = y + row * 4.0f * scale;
                float pixelSize = 3.0f * scale;
                
                float vertices[] = {
                    pixelX, pixelY,
                    pixelX + pixelSize, pixelY,
                    pixelX + pixelSize, pixelY + pixelSize,
                    pixelX, pixelY + pixelSize
                };
                
                m_vbo.bind(GL_ARRAY_BUFFER);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
        }
    }
}

void LoadingScreen::cleanup() {
    // RAII handles OpenGL resource cleanup automatically
    m_shader.reset();
}
