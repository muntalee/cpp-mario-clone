#include "Entity.h"

#include <utility>
#include <cmath>

Entity::Entity(const size_t i, std::string t)
    : m_id(i), m_tag(std::move(t))
{
}

bool Entity::isActive() const
{
    return m_active;
}

const std::string &Entity::tag() const
{
    return m_tag;
}

size_t Entity::id() const
{
    return m_id;
}

bool Entity::collides(std::shared_ptr<Entity> entity) const
{
    float dx = entity->cTransform->pos.x - cTransform->pos.x;
    float dy = entity->cTransform->pos.y - cTransform->pos.y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < entity->cCollision->radius + cCollision->radius)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Entity::destroy()
{
    m_active = false;
}
