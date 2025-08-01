#pragma once
#include "entities/Entity.h"
#include "engine/graphics/Camera.h"

class Player : public Entity {
public:
    Player(const glm::vec3& position = glm::vec3(0.0f, 100.0f, 0.0f));
    
    void update(float deltaTime) override;
    void render() override;
    
    void processInput(float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }
    
private:
    Camera m_camera;
    float m_movementSpeed;
    float m_jumpVelocity;
    bool m_flying;
    
    void applyPhysics(float deltaTime);
    void checkCollisions();
};
