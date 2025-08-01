#pragma once

// Forward declaration
struct GLFWwindow;

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();
    
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    GLFWwindow* getHandle() const { return m_window; }
    
    // 📏 Window dimensions and scalability
    void getFramebufferSize(int& width, int& height) const;
    float getAspectRatio() const;
    void setFramebufferSizeCallback(void(*callback)(GLFWwindow*, int, int));
    
    // Mouse control
    void setMouseCallback(void(*callback)(GLFWwindow*, double, double));
    void setMouseButtonCallback(void(*callback)(GLFWwindow*, int, int, int));
    void enableMouseCapture();
    void disableMouseCapture();
    
private:
    GLFWwindow* m_window;
};
