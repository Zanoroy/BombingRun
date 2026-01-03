#include "entities/GameObject.h"

namespace BombingRun {

GameObject::GameObject(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_velocityX(0.0f)
    , m_velocityY(0.0f)
    , m_width(0.0f)
    , m_height(0.0f)
    , m_active(true)
{
}

} // namespace BombingRun
