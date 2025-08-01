#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Camera movement directions
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);
    
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getFront() const { return m_front; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getUp() const { return m_up; }
    
    void setPosition(const glm::vec3& position) { m_position = position; }
    void setYaw(float yaw) { m_yaw = yaw; updateCameraVectors(); }
    void setPitch(float pitch) { m_pitch = pitch; updateCameraVectors(); }
    
    // Camera movement
    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    
    // Flying controls
    void setFlying(bool flying) { m_isFlying = flying; }
    bool isFlying() const { return m_isFlying; }
    void adjustFlyingSpeed(float scrollOffset);
    float getFlyingSpeed() const { return m_movementSpeed * m_flyingSpeedMultiplier; }
    
private:
    // Camera attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // Camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_fov;
    
    // Flying system
    bool m_isFlying;
    float m_flyingSpeedMultiplier;  // Multiplier for flying speed
    float m_minFlyingSpeed;         // Minimum flying speed
    float m_maxFlyingSpeed;         // Maximum flying speed
    
    // Euler angles
    float m_yaw;
    float m_pitch;
    
    void updateCameraVectors();
};
