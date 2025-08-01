#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>

// Forward declarations
class Texture;
class Shader;

/**
 * Centralized Asset Management System
 * Handles loading, caching, and lifetime of all game assets
 */
class AssetManager {
public:
    static AssetManager& getInstance();
    
    // Load and cache a texture
    std::shared_ptr<Texture> loadTexture(const std::string& path);
    
    // Load and cache a shader
    std::shared_ptr<Shader> loadShader(const std::string& vertPath, const std::string& fragPath);
    
    // Get cached texture by path
    std::shared_ptr<Texture> getTexture(const std::string& path);
    
    // Get cached shader by name
    std::shared_ptr<Shader> getShader(const std::string& name);
    
    // Preload commonly used assets
    void preloadAssets();
    
    // Clear all cached assets
    void clearCache();
    
    // Get memory usage info
    size_t getCachedTextureCount() const { return m_textures.size(); }
    size_t getCachedShaderCount() const { return m_shaders.size(); }
    
private:
    AssetManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    
    // Helper to generate shader keys
    std::string makeShaderKey(const std::string& vertPath, const std::string& fragPath);
};
