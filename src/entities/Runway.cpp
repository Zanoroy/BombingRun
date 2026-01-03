#include "entities/Runway.h"
#include <algorithm>

namespace BombingRun {

Runway::Runway(float x, float y, float width, float height)
    : GameObject(x, y)
    , m_health(1000)
    , m_maxHealth(1000)
    , m_baseMaxHealth(1000)
    , m_regenRate(1.0f)  // Regenerate 1 HP per second
    , m_regenTimer(0.0f)
    , m_baseColor({60, 60, 60, 255})  // Dark gray asphalt
{
    setDimensions(width, height);
}

void Runway::update(float deltaTime) {
    if (!m_active || m_health <= 0) {
        return;
    }

    // Gradual health regeneration
    if (m_health < m_maxHealth) {
        m_regenTimer += deltaTime;
        
        // Regenerate 1 HP per second
        if (m_regenTimer >= 1.0f) {
            m_health += static_cast<int>(m_regenRate);
            m_regenTimer = 0.0f;
            
            if (m_health > m_maxHealth) {
                m_health = m_maxHealth;
            }
        }
    }
}

void Runway::render(SDL_Renderer* renderer) {
    if (!m_active) {
        return;
    }

    // Get damage-based color
    SDL_Color color = getDamageColor();
    
    // Draw main runway surface
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = getBounds();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw runway center line (dashed white line)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int dashLength = 40;
    int dashGap = 20;
    int centerX = static_cast<int>(m_x + m_width / 2);
    
    for (int y = static_cast<int>(m_y); y < m_y + m_height; y += dashLength + dashGap) {
        SDL_RenderDrawLine(renderer, 
            centerX - 2, y, 
            centerX - 2, std::min(static_cast<int>(m_y + m_height), y + dashLength));
        SDL_RenderDrawLine(renderer, 
            centerX + 2, y, 
            centerX + 2, std::min(static_cast<int>(m_y + m_height), y + dashLength));
    }
    
    // Draw edge lines
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    int edgeOffset = 5;
    SDL_RenderDrawLine(renderer, 
        static_cast<int>(m_x) + edgeOffset, static_cast<int>(m_y),
        static_cast<int>(m_x) + edgeOffset, static_cast<int>(m_y + m_height));
    SDL_RenderDrawLine(renderer, 
        static_cast<int>(m_x + m_width) - edgeOffset, static_cast<int>(m_y),
        static_cast<int>(m_x + m_width) - edgeOffset, static_cast<int>(m_y + m_height));
    
    // Draw health bar above runway
    int barWidth = 200;
    int barHeight = 15;
    int barX = centerX - barWidth / 2;
    int barY = static_cast<int>(m_y) - 30;
    
    // Background
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
    SDL_Rect barBg = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &barBg);
    
    // Health fill
    float healthPercent = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
    SDL_Color healthColor;
    if (healthPercent > 0.66f) {
        healthColor = {0, 255, 0, 255};  // Green
    } else if (healthPercent > 0.33f) {
        healthColor = {255, 165, 0, 255};  // Orange
    } else {
        healthColor = {255, 0, 0, 255};  // Red
    }
    
    SDL_SetRenderDrawColor(renderer, healthColor.r, healthColor.g, healthColor.b, 255);
    SDL_Rect healthRect = {
        barX + 2,
        barY + 2,
        static_cast<int>((barWidth - 4) * healthPercent),
        barHeight - 4
    };
    SDL_RenderFillRect(renderer, &healthRect);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &barBg);
}

bool Runway::takeDamage(int damage) {
    if (m_health <= 0) {
        return false; // Already destroyed
    }
    
    m_health -= damage;
    
    if (m_health < 0) {
        m_health = 0;
    }
    
    if (m_health == 0) {
        m_active = false;
        return true; // Runway destroyed
    }
    
    return false;
}

void Runway::reduceMaxHealth(int reduction) {
    m_maxHealth -= reduction;
    
    if (m_maxHealth < 100) {
        m_maxHealth = 100; // Minimum max health
    }
    
    // If current health exceeds new max, reduce it
    if (m_health > m_maxHealth) {
        m_health = m_maxHealth;
    }
}

SDL_Color Runway::getDamageColor() const {
    float damageRatio = getDamageRatio();
    
    if (damageRatio < 0.33f) {
        // Healthy: base color
        return m_baseColor;
    } else if (damageRatio < 0.66f) {
        // Damaged: darker
        return SDL_Color{
            static_cast<Uint8>(m_baseColor.r * 0.7f),
            static_cast<Uint8>(m_baseColor.g * 0.7f),
            static_cast<Uint8>(m_baseColor.b * 0.7f),
            255
        };
    } else {
        // Heavily damaged: very dark with cracks
        return SDL_Color{
            static_cast<Uint8>(m_baseColor.r * 0.4f),
            static_cast<Uint8>(m_baseColor.g * 0.4f),
            static_cast<Uint8>(m_baseColor.b * 0.4f),
            255
        };
    }
}

} // namespace BombingRun
