#include "engine/AssetManager.h"
#include "engine/graphics/Texture.h"
#include "engine/graphics/Shader.h"
#include <iostream>
#include <filesystem>
#include <vector>

AssetManager& AssetManager::getInstance() {
    static AssetManager instance;
    return instance;
}

std::shared_ptr<Texture> AssetManager::loadTexture(const std::string& path) {
    // Check if already loaded
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Load new texture
    auto texture = std::make_shared<Texture>();
    if (texture->loadFromFile(path)) {
        m_textures[path] = texture;
        std::cout << "AssetManager: Loaded texture " << path << std::endl;
        return texture;
    } else {
        std::cout << "AssetManager: Failed to load texture " << path << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Shader> AssetManager::loadShader(const std::string& vertPath, const std::string& fragPath) {
    std::string key = makeShaderKey(vertPath, fragPath);
    
    // Check if already loaded
    auto it = m_shaders.find(key);
    if (it != m_shaders.end()) {
        return it->second;
    }
    
    // Load new shader
    auto shader = std::make_shared<Shader>();
    if (shader->loadFromFiles(vertPath, fragPath)) {
        m_shaders[key] = shader;
        std::cout << "AssetManager: Loaded shader " << key << std::endl;
        return shader;
    } else {
        std::cout << "AssetManager: Failed to load shader " << key << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Texture> AssetManager::getTexture(const std::string& path) {
    auto it = m_textures.find(path);
    return (it != m_textures.end()) ? it->second : nullptr;
}

std::shared_ptr<Shader> AssetManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    return (it != m_shaders.end()) ? it->second : nullptr;
}

void AssetManager::preloadAssets() {
    std::cout << "AssetManager: Preloading common assets..." << std::endl;
    
    // Preload common textures
    std::vector<std::string> commonTextures = {
        "assets/textures/grass.png",
        "assets/textures/water.webp",
        "assets/textures/oak.png",
        "assets/textures/oakleave.png"
    };
    
    for (const auto& path : commonTextures) {
        if (std::filesystem::exists(path)) {
            loadTexture(path);
        }
    }
    
    // Preload common shaders
    loadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    
    std::cout << "AssetManager: Preloaded " << m_textures.size() 
              << " textures and " << m_shaders.size() << " shaders" << std::endl;
}

void AssetManager::clearCache() {
    m_textures.clear();
    m_shaders.clear();
    std::cout << "AssetManager: Cleared asset cache" << std::endl;
}

std::string AssetManager::makeShaderKey(const std::string& vertPath, const std::string& fragPath) {
    return vertPath + "|" + fragPath;
}
