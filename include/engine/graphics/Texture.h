#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class Texture {
public:
    Texture();
    ~Texture();
    
    bool loadFromFile(const std::string& filePath);
    void bind(unsigned int textureUnit = 0) const;
    void unbind() const;
    
    unsigned int getID() const { return m_textureID; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    
private:
    unsigned int m_textureID;
    int m_width;
    int m_height;
    int m_channels;
};

class TextureManager {
public:
    static TextureManager& getInstance();
    
    std::shared_ptr<Texture> loadTexture(const std::string& filePath);
    std::shared_ptr<Texture> getTexture(const std::string& filePath);
    void clear();
    
private:
    TextureManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
