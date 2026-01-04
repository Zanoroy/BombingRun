#include "entities/Runway.h"
#include <algorithm>
#include <string>

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
    // Don't regenerate if destroyed
    if (m_health <= 0) {
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

void Runway::render(SDL_Renderer* renderer, TTF_Font* font) {
    // Always render runway, even when destroyed (don't check m_active)
    
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
    int barWidth = 240;
    int barHeight = 25;  // Increased height for larger text
    int barX = centerX - barWidth / 2;
    int barY = static_cast<int>(m_y) - 40;
    
    // Background
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 220);
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
    
    // Draw HP text if font is available - larger and with shadow for visibility
    if (font) {
        std::string hpText = std::to_string(m_health) + "/" + std::to_string(m_maxHealth) + " HP";
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, hpText.c_str(), textColor);
        
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                // Draw shadow first (black text slightly offset)
                SDL_Color shadowColor = {0, 0, 0, 255};
                SDL_Surface* shadowSurface = TTF_RenderText_Blended(font, hpText.c_str(), shadowColor);
                if (shadowSurface) {
                    SDL_Texture* shadowTexture = SDL_CreateTextureFromSurface(renderer, shadowSurface);
                    if (shadowTexture) {
                        SDL_Rect shadowRect = {
                            barX + (barWidth - textSurface->w) / 2 + 1,
                            barY + (barHeight - textSurface->h) / 2 + 1,
                            textSurface->w,
                            textSurface->h
                        };
                        SDL_RenderCopy(renderer, shadowTexture, nullptr, &shadowRect);
                        SDL_DestroyTexture(shadowTexture);
                    }
                    SDL_FreeSurface(shadowSurface);
                }
                
                // Center text in health bar
                SDL_Rect textRect = {
                    barX + (barWidth - textSurface->w) / 2,
                    barY + (barHeight - textSurface->h) / 2,
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
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
        // Keep m_active true so runway remains visible
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
