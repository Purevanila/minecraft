#include "engine/graphics/Texture.h"
#include "engine/graphics/OpenGL.h"

// Using stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

Texture::Texture() 
    : m_textureID(0), m_width(0), m_height(0), m_channels(0) {
    glGenTextures(1, &m_textureID);
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

bool Texture::loadFromFile(const std::string& filePath) {
    // Set stb_image to flip loaded texture's on the y-axis (since OpenGL expects 0.0 coordinate on the bottom)
    stbi_set_flip_vertically_on_load(true);
    
    unsigned char* data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, 0);
    if (!data) {
        std::cout << "Failed to load texture: " << filePath << std::endl;
        std::cout << "STB Image error: " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    bind();
    
    // Determine format
    GLenum format = GL_RGB;
    if (m_channels == 1) {
        format = GL_RED;
    } else if (m_channels == 3) {
        format = GL_RGB;
    } else if (m_channels == 4) {
        format = GL_RGBA;
    }
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    unbind();
    
    // Free image data
    stbi_image_free(data);
    
    std::cout << "Loaded texture: " << filePath << " (" << m_width << "x" << m_height << ", " << m_channels << " channels)" << std::endl;
    return true;
}

void Texture::bind(unsigned int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TextureManager implementation
TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::loadTexture(const std::string& filePath) {
    // Check if texture is already loaded
    auto it = m_textures.find(filePath);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Create and load new texture
    auto texture = std::make_shared<Texture>();
    if (texture->loadFromFile(filePath)) {
        m_textures[filePath] = texture;
        return texture;
    }
    
    return nullptr;
}

std::shared_ptr<Texture> TextureManager::getTexture(const std::string& filePath) {
    auto it = m_textures.find(filePath);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Try to load texture if not found
    return loadTexture(filePath);
}

void TextureManager::clear() {
    m_textures.clear();
}
