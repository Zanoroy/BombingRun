#include "entities/PlayerControlledJet.h"
#include <cmath>
#include <iostream>

namespace BombingRun {

PlayerControlledJet::PlayerControlledJet(float x, float y)
    : GameObject(x, y)
    , m_thrust(0.5f)           // Start at 50% thrust
    , m_angleDegrees(0.0f)     // Start facing right
    , m_angleRadians(0.0f)
    , m_turnRate(TURN_RATE)
    , m_thrustChangeRate(THRUST_CHANGE_RATE)
    , m_fireTimer(0.0f)
    , m_fireRate(FIRE_RATE)
    , m_health(5)              // Start with 5 health
{
    // Jet dimensions
    m_width = 30.0f;
    m_height = 25.0f;
    
    m_active = true;
    
    // Set initial velocity based on starting thrust (50% = midway between MIN and NORMAL_MAX_SPEED)
    float speed = MIN_SPEED + (NORMAL_MAX_SPEED - MIN_SPEED) * ((m_thrust - MIN_THRUST) / (MAX_THRUST - MIN_THRUST));
    m_velocityX = speed * std::cos(m_angleRadians);
    m_velocityY = speed * std::sin(m_angleRadians);
}

void PlayerControlledJet::update(float deltaTime) {
    if (!m_active) return;
    
    updatePhysics(deltaTime);
}

void PlayerControlledJet::handleInput(bool wPressed, bool sPressed, bool aPressed, bool dPressed, bool spacePressed) {
    // Handle thrust changes
    if (wPressed && !sPressed) {
        if (m_thrust < MAX_THRUST) {
            // Normal acceleration in 1% intervals
            m_thrust += m_thrustChangeRate * 0.016667f;  // Assuming ~60fps, so 1% per frame
            if (m_thrust >= MAX_THRUST) {
                m_thrust = MAX_THRUST;
            }
        } else if (m_thrust >= MAX_THRUST) {
            // At 100%, snap to 110% afterburner
            m_thrust = AFTERBURNER_THRUST;
        }
    } else if (sPressed && !wPressed) {
        m_thrust -= m_thrustChangeRate * 0.016667f;  // Assuming ~60fps, so 1% per frame
        if (m_thrust < MIN_THRUST) m_thrust = MIN_THRUST;
    } else if (!wPressed && !sPressed) {
        // When not holding W, revert from afterburner (110%) back to 100%
        // But don't change thrust if it's 99% or below
        if (m_thrust > MAX_THRUST) {
            m_thrust = MAX_THRUST;
        }
    }
    
    // Handle rotation
    if (aPressed && !dPressed) {
        m_angleDegrees -= m_turnRate * 0.016667f;  // Turn left
        if (m_angleDegrees < 0.0f) m_angleDegrees += 360.0f;
    } else if (dPressed && !aPressed) {
        m_angleDegrees += m_turnRate * 0.016667f;  // Turn right
        if (m_angleDegrees >= 360.0f) m_angleDegrees -= 360.0f;
    }
    
    m_angleRadians = m_angleDegrees * M_PI / 180.0f;
    
    // Note: Shooting is handled in Game::updateJetFights() to properly create bullets
}

void PlayerControlledJet::updatePhysics(float deltaTime) {
    // Decrease fire timer
    if (m_fireTimer > 0.0f) {
        m_fireTimer -= deltaTime;
    }
    
    // Calculate current speed based on thrust
    float speed;
    if (m_thrust <= MAX_THRUST) {
        // Linear interpolation between MIN_SPEED and NORMAL_MAX_SPEED (20% to 100%)
        float thrustNormalized = (m_thrust - MIN_THRUST) / (MAX_THRUST - MIN_THRUST);
        speed = MIN_SPEED + (NORMAL_MAX_SPEED - MIN_SPEED) * thrustNormalized;
    } else {
        // Linear interpolation between NORMAL_MAX_SPEED and AFTERBURNER_SPEED (100% to 110%)
        float thrustNormalized = (m_thrust - MAX_THRUST) / (AFTERBURNER_THRUST - MAX_THRUST);
        speed = NORMAL_MAX_SPEED + (AFTERBURNER_SPEED - NORMAL_MAX_SPEED) * thrustNormalized;
    }
    
    // Update velocity based on angle and speed
    m_velocityX = speed * std::cos(m_angleRadians);
    m_velocityY = speed * std::sin(m_angleRadians);
    
    // Update position
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
}

bool PlayerControlledJet::fire() {
    if (m_fireTimer > 0.0f) {
        return false;  // Still on cooldown
    }
    
    // Set fire timer (10 shots per second = 0.1s cooldown)
    m_fireTimer = 1.0f / m_fireRate;
    
    // Note: Actual bullet creation will be handled by Game class
    // This method just tracks the fire timing
    return true;
}

void PlayerControlledJet::takeDamage(int damage) {
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_active = false;  // Destroy jet
        std::cout << "Player 1 (Blue) jet destroyed!" << std::endl;
    } else {
        std::cout << "Player 1 (Blue) hit! Health: " << m_health << "/5" << std::endl;
    }
}

void PlayerControlledJet::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) return;
    
    // Draw jet as a triangle pointing in the direction of travel
    // Calculate triangle points
    float noseX = m_x + std::cos(m_angleRadians) * 20.0f;
    float noseY = m_y + std::sin(m_angleRadians) * 20.0f;
    
    float leftWingAngle = m_angleRadians + 2.5f;
    float rightWingAngle = m_angleRadians - 2.5f;
    
    float leftWingX = m_x + std::cos(leftWingAngle) * 15.0f;
    float leftWingY = m_y + std::sin(leftWingAngle) * 15.0f;
    
    float rightWingX = m_x + std::cos(rightWingAngle) * 15.0f;
    float rightWingY = m_y + std::sin(rightWingAngle) * 15.0f;
    
    // Draw jet body (cyan color)
    SDL_SetRenderDrawColor(renderer, 0, 200, 255, 255);
    
    // Draw triangle lines
    SDL_RenderDrawLine(renderer, 
                      static_cast<int>(noseX), static_cast<int>(noseY),
                      static_cast<int>(leftWingX), static_cast<int>(leftWingY));
    SDL_RenderDrawLine(renderer, 
                      static_cast<int>(leftWingX), static_cast<int>(leftWingY),
                      static_cast<int>(rightWingX), static_cast<int>(rightWingY));
    SDL_RenderDrawLine(renderer, 
                      static_cast<int>(rightWingX), static_cast<int>(rightWingY),
                      static_cast<int>(noseX), static_cast<int>(noseY));
    
    // Draw center line for thickness
    SDL_RenderDrawLine(renderer,
                      static_cast<int>(m_x), static_cast<int>(m_y),
                      static_cast<int>(noseX), static_cast<int>(noseY));
    
    // Draw thrust indicator (engine glow)
    float thrustGlowIntensity = std::min(m_thrust, 1.0f);  // Cap visual at 100% for normal glow
    int glowR = static_cast<int>(255 * thrustGlowIntensity);
    int glowG = static_cast<int>(150 * thrustGlowIntensity);
    
    // Afterburner effect (blue glow when > 100%)
    if (m_thrust > 1.0f) {
        glowR = 100;
        glowG = 200;
        int glowB = 255;
        SDL_SetRenderDrawColor(renderer, glowR, glowG, glowB, 200);
    } else {
        SDL_SetRenderDrawColor(renderer, glowR, glowG, 0, 200);
    }
    
    float exhaustX = m_x - std::cos(m_angleRadians) * 12.0f;
    float exhaustY = m_y - std::sin(m_angleRadians) * 12.0f;
    
    // Draw exhaust glow (small circle, larger in afterburner)
    float glowScale = (m_thrust > 1.0f) ? 1.5f : 1.0f;
    int glowRadius = static_cast<int>(5.0f * thrustGlowIntensity * glowScale);
    for (int w = 0; w < glowRadius * 2; w++) {
        for (int h = 0; h < glowRadius * 2; h++) {
            int dx = glowRadius - w;
            int dy = glowRadius - h;
            if ((dx*dx + dy*dy) <= (glowRadius * glowRadius)) {
                SDL_RenderDrawPoint(renderer, 
                                  static_cast<int>(exhaustX) + dx, 
                                  static_cast<int>(exhaustY) + dy);
            }
        }
    }
    
    // Draw HUD info if font available
    if (font) {
        // Show thrust percentage
        int thrustPercent = static_cast<int>(m_thrust * 100.0f);
        std::string thrustText = "Thrust: " + std::to_string(thrustPercent) + "%";
        
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* thrustSurface = TTF_RenderText_Blended(font, thrustText.c_str(), white);
        if (thrustSurface) {
            SDL_Texture* thrustTexture = SDL_CreateTextureFromSurface(renderer, thrustSurface);
            SDL_Rect thrustRect = {20, 20, thrustSurface->w, thrustSurface->h};
            SDL_RenderCopy(renderer, thrustTexture, nullptr, &thrustRect);
            SDL_DestroyTexture(thrustTexture);
            SDL_FreeSurface(thrustSurface);
        }
        
        // Show speed
        float speed;
        if (m_thrust <= MAX_THRUST) {
            float thrustNormalized = (m_thrust - MIN_THRUST) / (MAX_THRUST - MIN_THRUST);
            speed = MIN_SPEED + (NORMAL_MAX_SPEED - MIN_SPEED) * thrustNormalized;
        } else {
            float thrustNormalized = (m_thrust - MAX_THRUST) / (AFTERBURNER_THRUST - MAX_THRUST);
            speed = NORMAL_MAX_SPEED + (AFTERBURNER_SPEED - NORMAL_MAX_SPEED) * thrustNormalized;
        }
        std::string speedText = "Speed: " + std::to_string(static_cast<int>(speed)) + " px/s";
        
        SDL_Surface* speedSurface = TTF_RenderText_Blended(font, speedText.c_str(), white);
        if (speedSurface) {
            SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(renderer, speedSurface);
            SDL_Rect speedRect = {20, 50, speedSurface->w, speedSurface->h};
            SDL_RenderCopy(renderer, speedTexture, nullptr, &speedRect);
            SDL_DestroyTexture(speedTexture);
            SDL_FreeSurface(speedSurface);
        }
    }
}

} // namespace BombingRun
