#include "engine/graphics/Camera.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position)
    , m_worldUp(up)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_front(glm::vec3(0.0f, 0.0f, -1.0f))
    , m_movementSpeed(15.0f)  // Base movement speed
    , m_mouseSensitivity(0.1f)
    , m_fov(45.0f)
    , m_isFlying(true)        // Start in flying mode for creative gameplay
    , m_flyingSpeedMultiplier(1.0f)  // Start at normal speed
    , m_minFlyingSpeed(0.5f)  // Minimum 0.5x speed
    , m_maxFlyingSpeed(10.0f) // Maximum 10x speed
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(m_fov), aspectRatio, 0.1f, 10000.0f);
}

void Camera::processKeyboard(Camera_Movement direction, float deltaTime) {
    // Calculate velocity based on flying mode
    float velocity;
    if (m_isFlying) {
        velocity = m_movementSpeed * m_flyingSpeedMultiplier * deltaTime;
    } else {
        velocity = m_movementSpeed * deltaTime;
    }
    
    switch (direction) {
        case FORWARD:
            m_position += m_front * velocity;
            break;
        case BACKWARD:
            m_position -= m_front * velocity;
            break;
        case LEFT:
            m_position -= m_right * velocity;
            break;
        case RIGHT:
            m_position += m_right * velocity;
            break;
        case UP:
            m_position += m_worldUp * velocity;
            break;
        case DOWN:
            m_position -= m_worldUp * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;
    
    m_yaw += xoffset;
    m_pitch += yoffset;
    
    if (constrainPitch) {
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    }
    
    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    // Use scroll wheel to adjust flying speed when in flying mode
    if (m_isFlying) {
        adjustFlyingSpeed(yoffset);
    } else {
        // If not flying, adjust FOV (zoom)
        m_fov -= yoffset;
        m_fov = std::clamp(m_fov, 1.0f, 90.0f);
    }
}

void Camera::adjustFlyingSpeed(float scrollOffset) {
    // Increase speed with scroll up, decrease with scroll down
    float speedIncrement = 0.5f;  // How much to change speed per scroll tick
    
    if (scrollOffset > 0) {
        // Scroll up - increase speed
        m_flyingSpeedMultiplier += speedIncrement;
    } else if (scrollOffset < 0) {
        // Scroll down - decrease speed
        m_flyingSpeedMultiplier -= speedIncrement;
    }
    
    // Clamp speed to reasonable limits
    m_flyingSpeedMultiplier = std::clamp(m_flyingSpeedMultiplier, m_minFlyingSpeed, m_maxFlyingSpeed);
    
    // Print speed change for user feedback
    std::cout << "Flying speed: " << std::fixed << std::setprecision(1) 
              << m_flyingSpeedMultiplier << "x (" 
              << (m_movementSpeed * m_flyingSpeedMultiplier) << " units/sec)" << std::endl;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
