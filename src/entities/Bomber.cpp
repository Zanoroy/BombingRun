#include "entities/Bomber.h"
#include "systems/CollisionDetector.h"
#include "systems/TextureManager.h"
#include <cmath>
#include <iostream>

namespace BombingRun {

Bomber::Bomber(float spawnX, float spawnY, float targetX, float targetY, float speed, int bombType, int spriteIndex)
    : GameObject(spawnX, spawnY)
    , m_targetX(targetX)
    , m_targetY(targetY)
    , m_speed(speed)
    , m_angle(0.0f)
    , m_health(MAX_BOMBER_HEALTH)
    , m_maxHealth(MAX_BOMBER_HEALTH)
    , m_spriteIndex(spriteIndex % 5)  // Ensure 0-4 range
    , m_bombType(bombType)
    , m_reachedTarget(false)
    , m_bombsDropped(false)
    , m_exited(false)
    , m_smokeTimer(0.0f)
{
    m_width = BOMBER_WIDTH;
    m_height = BOMBER_HEIGHT;
    calculateVelocity();
    calculateAngle();
}

void Bomber::calculateVelocity() {
    // Calculate direction vector to target
    float dx = m_targetX - m_x;
    float dy = m_targetY - m_y;
    float distance = CollisionDetector::distance(m_x, m_y, m_targetX, m_targetY);
    
    if (distance > 0.0f) {
        // Normalize and apply speed
        m_velocityX = (dx / distance) * m_speed;
        m_velocityY = (dy / distance) * m_speed;
    }
}

void Bomber::calculateAngle() {
    // Calculate angle from velocity (atan2 returns radians)
    // Add 90 degrees because sprite faces up (0 degrees) by default
    m_angle = std::atan2(m_velocityY, m_velocityX) * 180.0f / M_PI + 90.0f;
}

void Bomber::update(float deltaTime) {
    if (!m_active) {
        return;
    }

    // Check if reached target
    float distanceToTarget = CollisionDetector::distance(m_x, m_y, m_targetX, m_targetY);
    if (!m_reachedTarget && distanceToTarget < TARGET_THRESHOLD) {
        m_reachedTarget = true;
        std::cout << "Bomber reached target at (" << m_targetX << ", " << m_targetY << ")" << std::endl;
        // After reaching target, continue moving in same direction to exit screen
    }

    // Update position
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;

    // Check if exited screen (above top or past sides)
    if (m_y < -200.0f || m_x < -200.0f || m_x > 4000.0f) {
        m_exited = true;
        m_active = false;
        std::cout << "Bomber exited screen" << std::endl;
    }

    // Update smoke timer if damaged
    if (isDamaged()) {
        m_smokeTimer += deltaTime;
        if (m_smokeTimer >= SMOKE_INTERVAL) {
            m_smokeTimer = 0.0f;
            // TODO: Spawn smoke particle effect in Phase 10
        }
    }
}

void Bomber::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) {
        return;
    }

    // Try to use sprite if available
    SDL_Texture* texture = TextureManager::getInstance().getTexture("planes");
    
    if (texture) {
        // Single sprite (175x175)
        SDL_Rect srcRect = {
            0,  // Always use sprite at position 0
            0,
            SPRITE_SIZE,
            SPRITE_SIZE
        };
        
        // Destination rectangle (centered on bomber position)
        SDL_Rect destRect = {
            static_cast<int>(m_x - RENDER_SIZE / 2),
            static_cast<int>(m_y - RENDER_SIZE / 2),
            RENDER_SIZE,
            RENDER_SIZE
        };
        
        // Apply damage tint if damaged
        if (isDamaged()) {
            SDL_Color color = getDamageColor();
            SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
        } else {
            SDL_SetTextureColorMod(texture, 255, 255, 255);
        }
        
        // Render with rotation
        TextureManager::getInstance().drawFrame("planes", srcRect, destRect, m_angle);
        
        // Reset color mod
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    } else {
        // Fallback to simple shape if sprite not loaded
        SDL_Color color = getDamageColor();
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

        SDL_Rect bodyRect = {
            static_cast<int>(m_x - m_width / 2),
            static_cast<int>(m_y - m_height / 2),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        SDL_RenderFillRect(renderer, &bodyRect);

        // Draw wings
        SDL_RenderDrawLine(renderer,
            static_cast<int>(m_x - m_width),
            static_cast<int>(m_y),
            static_cast<int>(m_x + m_width),
            static_cast<int>(m_y)
        );
    }

    // Draw health indicator (small bars above bomber)
    for (int i = 0; i < m_maxHealth; ++i) {
        SDL_Rect healthBar = {
            static_cast<int>(m_x - RENDER_SIZE / 2 + i * 18),
            static_cast<int>(m_y - RENDER_SIZE / 2 - 10),
            15,
            5
        };
        
        if (i < m_health) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green for remaining health
        } else {
            SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);  // Gray for lost health
        }
        SDL_RenderFillRect(renderer, &healthBar);
    }
}

void Bomber::takeDamage(int damage) {
    m_health -= damage;
    
    if (m_health <= 0) {
        m_health = 0;
        m_active = false;
        std::cout << "Bomber destroyed!" << std::endl;
    } else {
        std::cout << "Bomber hit! Health: " << m_health << "/" << m_maxHealth << std::endl;
    }
}

SDL_Color Bomber::getDamageColor() const {
    // Normal (full health) -> Orange (damaged) -> Red (critical)
    if (m_health == m_maxHealth) {
        return {255, 255, 255, 255};  // White (normal, sprite tint)
    } else if (m_health == 2) {
        return {255, 200, 0, 255};    // Yellow-Orange (damaged)
    } else {
        return {255, 100, 100, 255};  // Light Red (critical)
    }
}

bool Bomber::loadSprites() {
    return TextureManager::getInstance().loadTexture("planes", "assets/sprites/bomberandfighter.png");
}

} // namespace BombingRun
