#include "engine/graphics/Window.h"
#include "engine/graphics/OpenGL.h"
#include <stdexcept>
#include <iostream>

Window::Window(int width, int height, const char* title) {
    // Initialize GLFW if not already done
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 📏 Make window resizable for better user experience
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // For compatibility with macOS
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    // Create window
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Make the OpenGL context current
    glfwMakeContextCurrent(m_window);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW");
    }
    
    // Enable V-Sync
    glfwSwapInterval(1);
    
    std::cout << "Window created successfully: " << width << "x" << height << std::endl;
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setMouseCallback(void(*callback)(GLFWwindow*, double, double)) {
    glfwSetCursorPosCallback(m_window, callback);
}

void Window::setMouseButtonCallback(void(*callback)(GLFWwindow*, int, int, int)) {
    glfwSetMouseButtonCallback(m_window, callback);
}

void Window::enableMouseCapture() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::disableMouseCapture() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

// 📏 Window scalability methods
void Window::getFramebufferSize(int& width, int& height) const {
    glfwGetFramebufferSize(m_window, &width, &height);
}

float Window::getAspectRatio() const {
    int width, height;
    getFramebufferSize(width, height);
    return static_cast<float>(width) / static_cast<float>(height);
}

void Window::setFramebufferSizeCallback(void(*callback)(GLFWwindow*, int, int)) {
    glfwSetFramebufferSizeCallback(m_window, callback);
}
