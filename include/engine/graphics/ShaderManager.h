#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>
#include "utils/ModernCpp.h"

class Shader;

/**
 * High-performance shader management system
 * Features automatic hot-reloading, caching, and error recovery
 */
class ShaderManager {
public:
    static ShaderManager& getInstance();
    
    // Load or get cached shader
    std::shared_ptr<Shader> getShader(std::string_view name, 
                                     std::string_view vertexPath, 
                                     std::string_view fragmentPath);
    
    // Precompile commonly used shaders
    void precompileCommonShaders();
    
    // Hot-reload all shaders (for development)
    void reloadAllShaders();
    
    // Clear shader cache
    void clearCache();
    
    // Get compilation statistics
    struct Stats {
        size_t totalShaders = 0;
        size_t compilationErrors = 0;
        size_t memoryUsage = 0;
        double totalCompileTime = 0.0;
    };
    
    Stats getStats() const { return m_stats; }
    
private:
    ShaderManager() = default;
    
    struct ShaderEntry {
        std::shared_ptr<Shader> shader;
        std::string vertexPath;
        std::string fragmentPath;
        uint64_t lastModified = 0;
    };
    
    std::unordered_map<std::string, ShaderEntry, Utils::StringHash, std::equal_to<>> m_shaders;
    mutable Stats m_stats;
    
    // Utility functions
    uint64_t getFileModificationTime(std::string_view path) const;
    bool needsRecompilation(const ShaderEntry& entry) const;
    std::shared_ptr<Shader> compileShader(std::string_view name, 
                                         std::string_view vertexPath, 
                                         std::string_view fragmentPath);
};
