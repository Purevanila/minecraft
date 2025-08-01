#include "entities/Entity.h"

Entity::Entity(const glm::vec3& position) 
    : m_position(position), m_velocity(0.0f), m_size(1.0f), m_onGround(false) {
}
