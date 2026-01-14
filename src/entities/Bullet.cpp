#include "entities/Bullet.h"
#include <cmath>

namespace BombingRun {

Bullet::Bullet(float x, float y, float targetX, float targetY, float speed, void* owner, float size)
    : GameObject(x, y)
    , m_speed(speed)
    , m_lifetime(0.0f)
    , m_maxLifetime(3.0f)  // 3 seconds max lifetime
    , m_runwayY(10000.0f)  // Default far away
    , m_owner(owner)  // Track who fired this bullet
{
    // Calculate direction to target
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0.0f) {
        m_directionX = dx / distance;
        m_directionY = dy / distance;
    } else {
        m_directionX = 0.0f;
        m_directionY = 1.0f;  // Default downward
    }
    
    // Set velocity
    m_velocityX = m_directionX * m_speed;
    m_velocityY = m_directionY * m_speed;
    
    // Bullet size
    m_width = size;
    m_height = size;
    
    m_active = true;
}

void Bullet::update(float deltaTime) {
    if (!m_active) {
        return;
    }
    
    // Update position
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
    
    // Update lifetime
    m_lifetime += deltaTime;
    
    // Only deactivate if way off-screen or lifetime expired
    // Let collision detection handle hits - don't remove bullets prematurely
    if (m_lifetime >= m_maxLifetime || 
        m_x < -1000 || m_x > 3000 ||
        m_y < -1000 || m_y > 2000) {
        m_active = false;
    }
}

void Bullet::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) {
        return;
    }
    
    // Draw bullet as a bright, visible projectile (smaller size)
    // Bright white core (6x6)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect bulletCore = {
        static_cast<int>(m_x - 3),
        static_cast<int>(m_y - 3),
        6,
        6
    };
    SDL_RenderFillRect(renderer, &bulletCore);
    
    // Outer glow (bright red/orange for visibility)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Bright red
    for (int i = -5; i <= 5; i++) {
        for (int j = -5; j <= 5; j++) {
            if (abs(i) + abs(j) > 3 && abs(i) + abs(j) <= 8) {
                SDL_RenderDrawPoint(renderer, 
                    static_cast<int>(m_x) + i, 
                    static_cast<int>(m_y) + j);
            }
        }
    }
    
    // Draw visible motion trail
    SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);  // Bright orange trail
    for (int i = 1; i <= 8; i++) {
        float trailX = m_x - m_directionX * i * 2.5f;
        float trailY = m_y - m_directionY * i * 2.5f;
        SDL_Rect trailRect = {
            static_cast<int>(trailX - 2),
            static_cast<int>(trailY - 2),
            4,
            4
        };
        SDL_RenderFillRect(renderer, &trailRect);
    }
}

} // namespace BombingRun
