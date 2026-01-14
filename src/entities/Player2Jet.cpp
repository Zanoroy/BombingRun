#include "entities/Player2Jet.h"
#include <cmath>
#include <iostream>

namespace BombingRun {

Player2Jet::Player2Jet(float x, float y)
    : GameObject(x, y)
    , m_player2Thrust(0.5f)           // Start at 50% thrust
    , m_player2AngleDegrees(180.0f)   // Start facing left (opposite of player 1)
    , m_player2AngleRadians(M_PI)
    , m_player2TurnRate(P2_TURN_RATE)
    , m_player2ThrustChangeRate(P2_THRUST_CHANGE_RATE)
    , m_player2FireTimer(0.0f)
    , m_player2FireRate(P2_FIRE_RATE)
    , m_player2Health(5)              // Start with 5 health
{
    // Jet dimensions
    m_width = 30.0f;
    m_height = 25.0f;
    
    m_active = true;
    
    // Set initial velocity based on starting thrust (50% = midway between MIN and NORMAL_MAX_SPEED)
    float p2Speed = P2_MIN_SPEED + (P2_NORMAL_MAX_SPEED - P2_MIN_SPEED) * ((m_player2Thrust - P2_MIN_THRUST) / (P2_MAX_THRUST - P2_MIN_THRUST));
    m_velocityX = p2Speed * std::cos(m_player2AngleRadians);
    m_velocityY = p2Speed * std::sin(m_player2AngleRadians);
}

void Player2Jet::update(float deltaTime) {
    if (!m_active) return;
    
    updatePhysics(deltaTime);
    
    // Update position
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
}

void Player2Jet::handleInput(bool upPressed, bool downPressed, bool leftPressed, bool rightPressed, bool rctrlPressed) {
    // Handle thrust changes
    if (upPressed && !downPressed) {
        if (m_player2Thrust < P2_MAX_THRUST) {
            // Normal acceleration in 1% intervals
            m_player2Thrust += m_player2ThrustChangeRate * 0.016667f;  // Assuming ~60fps, so 1% per frame
            if (m_player2Thrust >= P2_MAX_THRUST) {
                m_player2Thrust = P2_MAX_THRUST;
            }
        } else if (m_player2Thrust >= P2_MAX_THRUST) {
            // At 100%, snap to 110% afterburner
            m_player2Thrust = P2_AFTERBURNER_THRUST;
        }
    } else if (downPressed && !upPressed) {
        m_player2Thrust -= m_player2ThrustChangeRate * 0.016667f;  // Assuming ~60fps, so 1% per frame
        if (m_player2Thrust < P2_MIN_THRUST) m_player2Thrust = P2_MIN_THRUST;
    } else if (!upPressed && !downPressed) {
        // When not holding Up, revert from afterburner (110%) back to 100%
        // But don't change thrust if it's 99% or below
        if (m_player2Thrust > P2_MAX_THRUST) {
            m_player2Thrust = P2_MAX_THRUST;
        }
    }
    
    // Handle rotation
    if (leftPressed && !rightPressed) {
        m_player2AngleDegrees -= m_player2TurnRate * 0.016667f;  // Turn left
        if (m_player2AngleDegrees < 0.0f) m_player2AngleDegrees += 360.0f;
    } else if (rightPressed && !leftPressed) {
        m_player2AngleDegrees += m_player2TurnRate * 0.016667f;  // Turn right
        if (m_player2AngleDegrees >= 360.0f) m_player2AngleDegrees -= 360.0f;
    }
    
    m_player2AngleRadians = m_player2AngleDegrees * M_PI / 180.0f;
    
    // Note: Shooting is handled in Game::updateJetFights() to properly create bullets
}

void Player2Jet::updatePhysics(float deltaTime) {
    // Decrease fire timer
    if (m_player2FireTimer > 0.0f) {
        m_player2FireTimer -= deltaTime;
    }
    
    // Calculate current speed based on thrust
    float p2Speed;
    if (m_player2Thrust <= P2_MAX_THRUST) {
        // Linear interpolation between MIN_SPEED and NORMAL_MAX_SPEED (20% to 100%)
        float p2ThrustNormalized = (m_player2Thrust - P2_MIN_THRUST) / (P2_MAX_THRUST - P2_MIN_THRUST);
        p2Speed = P2_MIN_SPEED + (P2_NORMAL_MAX_SPEED - P2_MIN_SPEED) * p2ThrustNormalized;
    } else {
        // Afterburner range (100% to 110%)
        float p2ThrustNormalized = (m_player2Thrust - P2_MAX_THRUST) / (P2_AFTERBURNER_THRUST - P2_MAX_THRUST);
        p2Speed = P2_NORMAL_MAX_SPEED + (P2_AFTERBURNER_SPEED - P2_NORMAL_MAX_SPEED) * p2ThrustNormalized;
    }
    
    // Update velocity based on current angle and speed
    m_velocityX = p2Speed * std::cos(m_player2AngleRadians);
    m_velocityY = p2Speed * std::sin(m_player2AngleRadians);
}

bool Player2Jet::fire() {
    if (m_player2FireTimer <= 0.0f) {
        m_player2FireTimer = 1.0f / m_player2FireRate;  // Set cooldown
        return true;
    }
    return false;
}

void Player2Jet::takeDamage(int damage) {
    m_player2Health -= damage;
    if (m_player2Health <= 0) {
        m_player2Health = 0;
        m_active = false;  // Destroy jet
        std::cout << "Player 2 (Red) jet destroyed!" << std::endl;
    } else {
        std::cout << "Player 2 (Red) hit! Health: " << m_player2Health << "/5" << std::endl;
    }
}

void Player2Jet::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) return;
    
    // Draw jet as a triangle pointing in the direction of travel
    // Calculate triangle points
    float noseX = m_x + std::cos(m_player2AngleRadians) * 20.0f;
    float noseY = m_y + std::sin(m_player2AngleRadians) * 20.0f;
    
    float leftWingAngle = m_player2AngleRadians + 2.5f;
    float rightWingAngle = m_player2AngleRadians - 2.5f;
    
    float leftWingX = m_x + std::cos(leftWingAngle) * 15.0f;
    float leftWingY = m_y + std::sin(leftWingAngle) * 15.0f;
    
    float rightWingX = m_x + std::cos(rightWingAngle) * 15.0f;
    float rightWingY = m_y + std::sin(rightWingAngle) * 15.0f;
    
    // Draw jet body (red color for player 2)
    SDL_SetRenderDrawColor(renderer, 255, 60, 60, 255);
    
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
    float thrustGlowIntensity = std::min(m_player2Thrust, 1.0f);  // Cap visual at 100% for normal glow
    int glowR = 255;  // Red for player 2
    int glowG = static_cast<int>(100 * thrustGlowIntensity);
    
    // Afterburner effect (brighter red glow when > 100%)
    if (m_player2Thrust > P2_MAX_THRUST) {
        glowR = 255;
        glowG = 100;
        int glowB = 100;
        SDL_SetRenderDrawColor(renderer, glowR, glowG, glowB, 200);
    } else {
        SDL_SetRenderDrawColor(renderer, glowR, glowG, 0, 200);
    }
    
    float exhaustX = m_x - std::cos(m_player2AngleRadians) * 12.0f;
    float exhaustY = m_y - std::sin(m_player2AngleRadians) * 12.0f;
    
    // Draw exhaust glow (small circle, larger in afterburner)
    float glowScale = (m_player2Thrust > P2_MAX_THRUST) ? 1.5f : 1.0f;
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
    
    // Draw HUD info if font available (right side of screen for player 2)
    if (font) {
        // Show thrust percentage
        int p2ThrustPercent = static_cast<int>(m_player2Thrust * 100.0f);
        std::string p2ThrustText = "P2 Thrust: " + std::to_string(p2ThrustPercent) + "%";
        
        SDL_Color red = {255, 100, 100, 255};
        SDL_Surface* thrustSurface = TTF_RenderText_Blended(font, p2ThrustText.c_str(), red);
        if (thrustSurface) {
            SDL_Texture* thrustTexture = SDL_CreateTextureFromSurface(renderer, thrustSurface);
            SDL_Rect thrustRect = {1920 - 220, 20, thrustSurface->w, thrustSurface->h};  // Right side
            SDL_RenderCopy(renderer, thrustTexture, nullptr, &thrustRect);
            SDL_DestroyTexture(thrustTexture);
            SDL_FreeSurface(thrustSurface);
        }
        
        // Show speed
        float p2Speed;
        if (m_player2Thrust <= P2_MAX_THRUST) {
            float p2ThrustNormalized = (m_player2Thrust - P2_MIN_THRUST) / (P2_MAX_THRUST - P2_MIN_THRUST);
            p2Speed = P2_MIN_SPEED + (P2_NORMAL_MAX_SPEED - P2_MIN_SPEED) * p2ThrustNormalized;
        } else {
            float p2ThrustNormalized = (m_player2Thrust - P2_MAX_THRUST) / (P2_AFTERBURNER_THRUST - P2_MAX_THRUST);
            p2Speed = P2_NORMAL_MAX_SPEED + (P2_AFTERBURNER_SPEED - P2_NORMAL_MAX_SPEED) * p2ThrustNormalized;
        }
        std::string p2SpeedText = "P2 Speed: " + std::to_string(static_cast<int>(p2Speed)) + " px/s";
        
        SDL_Surface* speedSurface = TTF_RenderText_Blended(font, p2SpeedText.c_str(), red);
        if (speedSurface) {
            SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(renderer, speedSurface);
            SDL_Rect speedRect = {1920 - 220, 50, speedSurface->w, speedSurface->h};  // Right side
            SDL_RenderCopy(renderer, speedTexture, nullptr, &speedRect);
            SDL_DestroyTexture(speedTexture);
            SDL_FreeSurface(speedSurface);
        }
    }
}

} // namespace BombingRun
