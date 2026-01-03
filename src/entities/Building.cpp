#include "entities/Building.h"
#include <algorithm>

namespace BombingRun {

Building::Building(float x, float y, float width, float height, int maxHealth, Type type)
    : GameObject(x, y)
    , m_health(maxHealth)
    , m_maxHealth(maxHealth)
    , m_type(type)
    , m_baseColor({128, 128, 128, 255}) // Default gray color
{
    setDimensions(width, height);
}

void Building::update(float deltaTime) {
    // Buildings are static, but could add effects here
    if (m_health <= 0) {
        m_active = false;
    }
}

void Building::render(SDL_Renderer* renderer) {
    if (!m_active || m_health <= 0) {
        return;
    }

    // Get damage-based color
    SDL_Color color = getDamageColor();
    
    // Set render color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw filled rectangle for building
    SDL_Rect rect = getBounds();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw darker border
    SDL_SetRenderDrawColor(renderer, 
        static_cast<Uint8>(color.r * 0.6f),
        static_cast<Uint8>(color.g * 0.6f),
        static_cast<Uint8>(color.b * 0.6f),
        255);
    SDL_RenderDrawRect(renderer, &rect);
}

bool Building::takeDamage(int damage) {
    if (m_health <= 0) {
        return false; // Already destroyed
    }
    
    m_health -= damage;
    
    if (m_health < 0) {
        m_health = 0;
    }
    
    if (m_health == 0) {
        m_active = false;
        return true; // Building destroyed
    }
    
    return false;
}

SDL_Color Building::getDamageColor() const {
    float damageRatio = getDamageRatio();
    
    if (damageRatio < 0.33f) {
        // Healthy: base color
        return m_baseColor;
    } else if (damageRatio < 0.66f) {
        // Damaged: orange tint
        return SDL_Color{
            static_cast<Uint8>(std::min(255, m_baseColor.r + 60)),
            static_cast<Uint8>(std::max(0, m_baseColor.g - 20)),
            static_cast<Uint8>(std::max(0, m_baseColor.b - 40)),
            255
        };
    } else {
        // Heavily damaged: red tint
        return SDL_Color{
            static_cast<Uint8>(std::min(255, m_baseColor.r + 80)),
            static_cast<Uint8>(std::max(0, m_baseColor.g - 60)),
            static_cast<Uint8>(std::max(0, m_baseColor.b - 80)),
            255
        };
    }
}

} // namespace BombingRun
