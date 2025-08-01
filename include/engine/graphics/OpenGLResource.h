#pragma once
#include <GL/glew.h>
#include <memory>
#include <vector>

/**
 * RAII wrapper for OpenGL resources to prevent memory leaks
 */
namespace OpenGL {

    // RAII wrapper for VAO
    class VertexArray {
    public:
        VertexArray() { glGenVertexArrays(1, &m_id); }
        ~VertexArray() { if (m_id != 0) glDeleteVertexArrays(1, &m_id); }
        
        // Move semantics
        VertexArray(VertexArray&& other) noexcept : m_id(other.m_id) { other.m_id = 0; }
        VertexArray& operator=(VertexArray&& other) noexcept {
            if (this != &other) {
                if (m_id != 0) glDeleteVertexArrays(1, &m_id);
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }
        
        // Delete copy semantics
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        
        void bind() const { glBindVertexArray(m_id); }
        void unbind() const { glBindVertexArray(0); }
        GLuint id() const { return m_id; }
        
    private:
        GLuint m_id = 0;
    };

    // RAII wrapper for VBO/EBO
    class Buffer {
    public:
        Buffer() { glGenBuffers(1, &m_id); }
        ~Buffer() { if (m_id != 0) glDeleteBuffers(1, &m_id); }
        
        // Move semantics
        Buffer(Buffer&& other) noexcept : m_id(other.m_id) { other.m_id = 0; }
        Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                if (m_id != 0) glDeleteBuffers(1, &m_id);
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }
        
        // Delete copy semantics  
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        
        void bind(GLenum target) const { glBindBuffer(target, m_id); }
        void unbind(GLenum target) const { glBindBuffer(target, 0); }
        GLuint id() const { return m_id; }
        
        template<typename T>
        void setData(GLenum target, const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW) {
            bind(target);
            glBufferData(target, data.size() * sizeof(T), data.data(), usage);
        }
        
    private:
        GLuint m_id = 0;
    };

    // RAII wrapper for textures
    class Texture {
    public:
        Texture() { glGenTextures(1, &m_id); }
        ~Texture() { if (m_id != 0) glDeleteTextures(1, &m_id); }
        
        // Move semantics
        Texture(Texture&& other) noexcept : m_id(other.m_id) { other.m_id = 0; }
        Texture& operator=(Texture&& other) noexcept {
            if (this != &other) {
                if (m_id != 0) glDeleteTextures(1, &m_id);
                m_id = other.m_id;
                other.m_id = 0;
            }
            return *this;
        }
        
        // Delete copy semantics
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        
        void bind(GLenum target = GL_TEXTURE_2D) const { glBindTexture(target, m_id); }
        void unbind(GLenum target = GL_TEXTURE_2D) const { glBindTexture(target, 0); }
        GLuint id() const { return m_id; }
        
    private:
        GLuint m_id = 0;
    };

} // namespace OpenGL
