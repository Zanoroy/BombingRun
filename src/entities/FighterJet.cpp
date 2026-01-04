#include "entities/FighterJet.h"
#include "entities/Bomber.h"
#include "systems/TextureManager.h"
#include <cmath>
#include <iostream>

namespace BombingRun {

FighterJet::FighterJet(float x, float y, float runwayX, float runwayY)
    : GameObject(x, y)
    , m_health(4)
    , m_maxHealth(4)
    , m_bulletsRemaining(579)
    , m_speed(400.0f)  // Faster than bombers
    , m_fireRate(8.0f)  // 8 shots per second (increased from 3)
    , m_fireTimer(0.0f)
    , m_accuracy(0.8f)  // 80% hit rate
    , m_state(FighterState::SPAWNING)
    , m_target(nullptr)
    , m_runwayX(runwayX)
    , m_runwayY(runwayY)
    , m_patrolRadius(300.0f)
    , m_patrolAngle(0.0f)
    , m_interceptX(0.0f)
    , m_interceptY(0.0f)
    , m_hitCount(0)
    , m_angle(0.0f)
{
    // Fighter jet is larger than bomber
    m_width = 25.0f;
    m_height = 30.0f;
    
    // Set patrol center around runway
    m_patrolCenterX = runwayX;
    m_patrolCenterY = runwayY;
    
    m_active = true;
    
    std::cout << "Fighter jet created at (" << x << ", " << y << ")" << std::endl;
}

bool FighterJet::loadSprites() {
    return TextureManager::getInstance().loadTexture("fighter", "assets/sprites/FighterJetEdited.png");
}

void FighterJet::update(float deltaTime) {
    if (!m_active || m_health <= 0) {
        return;
    }
    
    // Update fire timer
    if (m_fireTimer > 0.0f) {
        m_fireTimer -= deltaTime;
    }
    
    // Update based on state
    switch (m_state) {
        case FighterState::SPAWNING:
            updateSpawning(deltaTime);
            break;
        case FighterState::PATROLLING:
            updatePatrolling(deltaTime);
            break;
        case FighterState::ENGAGING:
            updateEngaging(deltaTime);
            break;
        case FighterState::RETURNING:
            updateReturning(deltaTime);
            break;
        case FighterState::LANDED:
            // Do nothing, inactive
            break;
    }
    
    // Update angle based on velocity with smooth interpolation
    if (m_velocityX != 0.0f || m_velocityY != 0.0f) {
        float targetAngle = std::atan2(m_velocityY, m_velocityX) * 180.0f / M_PI + 90.0f;
        
        // Calculate shortest angle difference
        float angleDiff = targetAngle - m_angle;
        while (angleDiff > 180.0f) angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;
        
        // Smoothly interpolate toward target angle
        float turnSpeed = 180.0f;  // degrees per second
        float maxTurn = turnSpeed * deltaTime;
        
        if (std::abs(angleDiff) < maxTurn) {
            m_angle = targetAngle;
        } else {
            m_angle += (angleDiff > 0 ? maxTurn : -maxTurn);
        }
        
        // Keep angle in 0-360 range
        while (m_angle >= 360.0f) m_angle -= 360.0f;
        while (m_angle < 0.0f) m_angle += 360.0f;
    }
}

void FighterJet::updateSpawning(float deltaTime) {
    // Move toward patrol center
    float dx = m_patrolCenterX - m_x;
    float dy = m_patrolCenterY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < m_patrolRadius) {
        // Reached patrol area - calculate current angle on circle
        float relativeX = m_x - m_patrolCenterX;
        float relativeY = m_y - m_patrolCenterY;
        m_patrolAngle = std::atan2(relativeY, relativeX);
        
        m_state = FighterState::PATROLLING;
        std::cout << "Fighter jet entering patrol mode at angle " << m_patrolAngle << std::endl;
        return;
    }
    
    // Move toward center
    m_velocityX = (dx / distance) * m_speed;
    m_velocityY = (dy / distance) * m_speed;
    
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
}

void FighterJet::updatePatrolling(float deltaTime) {
    // Circle around the patrol center
    float angularSpeed = 1.2f;  // radians per second (increased for faster circling)
    
    // Advance the patrol angle
    m_patrolAngle += deltaTime * angularSpeed;
    
    if (m_patrolAngle >= 2.0f * M_PI) {
        m_patrolAngle -= 2.0f * M_PI;
    }
    
    // Calculate ideal position on patrol circle
    float targetX = m_patrolCenterX + m_patrolRadius * std::cos(m_patrolAngle);
    float targetY = m_patrolCenterY + m_patrolRadius * std::sin(m_patrolAngle);
    
    // Smoothly interpolate to the circle (for smooth entry from spawning)
    float adjustSpeed = 5.0f;  // How quickly to converge to perfect circle
    m_x += (targetX - m_x) * adjustSpeed * deltaTime;
    m_y += (targetY - m_y) * adjustSpeed * deltaTime;
    
    // Calculate velocity tangent to the circle (perpendicular to radius)
    // For counterclockwise motion: vx = -sin(angle), vy = cos(angle)
    m_velocityX = -std::sin(m_patrolAngle) * m_patrolRadius * angularSpeed;
    m_velocityY = std::cos(m_patrolAngle) * m_patrolRadius * angularSpeed;
    
    // If out of ammo, return to base
    if (m_bulletsRemaining <= 0) {
        m_state = FighterState::RETURNING;
        m_target = nullptr;
        std::cout << "Fighter jet out of ammo, returning to base" << std::endl;
    }
}

void FighterJet::updateEngaging(float deltaTime) {
    // Check if target is still valid
    if (!m_target || !m_target->isActive()) {
        m_target = nullptr;
        m_state = FighterState::PATROLLING;
        return;
    }
    
    // Calculate interception point
    calculateInterception(
        m_target->getX(), m_target->getY(),
        m_target->getVelocityX(), m_target->getVelocityY()
    );
    
    // Move toward interception point
    float dx = m_interceptX - m_x;
    float dy = m_interceptY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 1.0f) {
        m_velocityX = (dx / distance) * m_speed;
        m_velocityY = (dy / distance) * m_speed;
        
        m_x += m_velocityX * deltaTime;
        m_y += m_velocityY * deltaTime;
    }
    
    // Fire if in range and timer ready
    if (distance < 500.0f && m_fireTimer <= 0.0f) {
        fire();
    }
    
    // If out of ammo, return to base
    if (m_bulletsRemaining <= 0) {
        m_state = FighterState::RETURNING;
        m_target = nullptr;
        std::cout << "Fighter jet out of ammo, returning to base" << std::endl;
    }
}

void FighterJet::updateReturning(float deltaTime) {
    // Move toward runway
    float dx = m_runwayX - m_x;
    float dy = m_runwayY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < 20.0f) {
        // Landed
        m_state = FighterState::LANDED;
        m_active = false;
        std::cout << "Fighter jet landed" << std::endl;
        return;
    }
    
    // Move toward runway
    m_velocityX = (dx / distance) * m_speed;
    m_velocityY = (dy / distance) * m_speed;
    
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
}

bool FighterJet::calculateInterception(float targetX, float targetY, 
                                       float targetVx, float targetVy) {
    // Simple lead targeting - aim ahead of target
    float leadTime = 0.5f;  // Half second lead
    
    m_interceptX = targetX + targetVx * leadTime;
    m_interceptY = targetY + targetVy * leadTime;
    
    return true;
}

void FighterJet::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) {
        return;
    }
    
    // Try to use sprite if available
    SDL_Texture* texture = TextureManager::getInstance().getTexture("fighter");
    
    if (texture) {
        // Single sprite (175x175)
        SDL_Rect srcRect = {
            0,  // Always use sprite at position 0
            0,
            SPRITE_SIZE,
            SPRITE_SIZE
        };
        
        // Destination rectangle (centered on fighter position)
        SDL_Rect destRect = {
            static_cast<int>(m_x - RENDER_SIZE / 2),
            static_cast<int>(m_y - RENDER_SIZE / 2),
            RENDER_SIZE,
            RENDER_SIZE
        };
        
        // Apply damage tint if damaged
        if (m_hitCount > 0) {
            SDL_Color color = getDamageColor();
            SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
        } else {
            SDL_SetTextureColorMod(texture, 255, 255, 255);
        }
        
        // Render with rotation
        TextureManager::getInstance().drawFrame("fighter", srcRect, destRect, m_angle);
        
        // Reset color mod
        SDL_SetTextureColorMod(texture, 255, 255, 255);
    } else {
        // Fallback to shape rendering if sprite not loaded
        SDL_Color color = getDamageColor();
        
        // Draw fighter jet as a triangle/arrow shape pointing forward
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        
        // Main body (rectangle)
        SDL_Rect body = {
            static_cast<int>(m_x - m_width / 2),
            static_cast<int>(m_y - m_height / 2),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        SDL_RenderFillRect(renderer, &body);
        
        // Wings (horizontal rectangles)
        SDL_Rect leftWing = {
            static_cast<int>(m_x - m_width),
            static_cast<int>(m_y - 2),
            static_cast<int>(m_width / 2),
            4
        };
        SDL_RenderFillRect(renderer, &leftWing);
        
        SDL_Rect rightWing = {
            static_cast<int>(m_x + m_width / 2),
            static_cast<int>(m_y - 2),
            static_cast<int>(m_width / 2),
            4
        };
        SDL_RenderFillRect(renderer, &rightWing);
        
        // Cockpit (darker spot)
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect cockpit = {
            static_cast<int>(m_x - 3),
            static_cast<int>(m_y - m_height / 2 + 3),
            6,
            6
        };
        SDL_RenderFillRect(renderer, &cockpit);
    }
    
    // Draw health bar if damaged
    if (m_hitCount > 0) {
        int barWidth = 30;
        int barHeight = 4;
        int barX = static_cast<int>(m_x - barWidth / 2);
        int barY = static_cast<int>(m_y - RENDER_SIZE / 2 - 10);
        
        // Background
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 200);
        SDL_Rect barBg = {barX, barY, barWidth, barHeight};
        SDL_RenderFillRect(renderer, &barBg);
        
        // Health fill
        float healthPercent = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
        SDL_Color healthColor = healthPercent > 0.5f ? 
            SDL_Color{0, 255, 0, 255} : SDL_Color{255, 0, 0, 255};
        
        SDL_SetRenderDrawColor(renderer, healthColor.r, healthColor.g, healthColor.b, 255);
        SDL_Rect healthRect = {
            barX + 1,
            barY + 1,
            static_cast<int>((barWidth - 2) * healthPercent),
            barHeight - 2
        };
        SDL_RenderFillRect(renderer, &healthRect);
    }
}

bool FighterJet::takeDamage(int damage) {
    if (m_health <= 0) {
        return false;
    }
    
    m_health -= damage;
    m_hitCount++;
    
    if (m_health < 0) {
        m_health = 0;
    }
    
    std::cout << "Fighter jet hit! Health: " << m_health << "/" << m_maxHealth << std::endl;
    
    if (m_health == 0) {
        m_active = false;
        std::cout << "Fighter jet destroyed!" << std::endl;
        return true;
    }
    
    return false;
}

void FighterJet::setTarget(Bomber* target) {
    if (target && target->isActive()) {
        m_target = target;
        m_state = FighterState::ENGAGING;
        std::cout << "Fighter jet engaging target" << std::endl;
    }
}

bool FighterJet::fire() {
    if (m_bulletsRemaining <= 0 || m_fireTimer > 0.0f) {
        return false;
    }
    
    m_bulletsRemaining--;
    m_fireTimer = 1.0f / m_fireRate;  // Reset fire timer
    
    // Will be handled by weapon manager to actually create bullet
    return true;
}

SDL_Color FighterJet::getDamageColor() const {
    float healthPercent = static_cast<float>(m_health) / static_cast<float>(m_maxHealth);
    
    // Fighter jets are dark gray/black
    uint8_t baseColor = 80;
    
    if (healthPercent > 0.66f) {
        // Healthy: dark gray
        return SDL_Color{baseColor, baseColor, baseColor, 255};
    } else if (healthPercent > 0.33f) {
        // Damaged: darker with orange tint
        return SDL_Color{static_cast<uint8_t>(baseColor + 20), 
                        static_cast<uint8_t>(baseColor > 10 ? baseColor - 10 : 0), 
                        0, 255};
    } else {
        // Heavily damaged: very dark with red tint
        return SDL_Color{static_cast<uint8_t>(baseColor + 40), 0, 0, 255};
    }
}

} // namespace BombingRun
