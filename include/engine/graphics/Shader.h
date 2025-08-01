#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader();
    ~Shader();
    
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromString(const std::string& vertexSource, const std::string& fragmentSource);
    
    void use() const;
    unsigned int getProgram() const { return m_program; }
    
    // Uniform setters
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& value);
    
private:
    unsigned int m_program;
    mutable std::unordered_map<std::string, int> m_uniformCache;
    
    unsigned int compileShader(const std::string& source, unsigned int type);
    std::string loadShaderFile(const std::string& path);
    int getUniformLocation(const std::string& name) const;
};
