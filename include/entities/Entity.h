#pragma once
#include <glm/glm.hpp>

class Entity {
public:
    Entity(const glm::vec3& position = glm::vec3(0.0f));
    virtual ~Entity() = default;
    
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    
    const glm::vec3& getPosition() const { return m_position; }
    void setPosition(const glm::vec3& position) { m_position = position; }
    
    const glm::vec3& getVelocity() const { return m_velocity; }
    void setVelocity(const glm::vec3& velocity) { m_velocity = velocity; }
    
protected:
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_size;
    bool m_onGround;
};
