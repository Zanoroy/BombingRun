#include "entities/PlayerJet.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace BombingRun {

PlayerJet::PlayerJet(float x, float y, Player player)
    : GameObject(x, y)
    , m_player(player)
    , m_velocityX(0.0f)
    , m_velocityY(0.0f)
    , m_angle(player == Player::PLAYER1 ? 0.0f : M_PI)  // P1 faces right, P2 faces left
    , m_throttlePercent(50)  // Start at 50% throttle
    , m_rotationSpeed(ROTATION_SPEED)
    , m_health(MAX_HEALTH_VALUE)
    , m_maxHealth(MAX_HEALTH_VALUE)
    , m_shootCooldown(SHOOT_COOLDOWN)
    , m_timeSinceLastShot(0.0f)
    , m_throttleChangeTimer(0.0f)
{
    m_active = true;
    // Start with 50% throttle speed in facing direction
    float startSpeed = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * (m_throttlePercent - MIN_THROTTLE) / (MAX_THROTTLE - MIN_THROTTLE);
    m_velocityX = std::cos(m_angle) * startSpeed;
    m_velocityY = std::sin(m_angle) * startSpeed;
}

void PlayerJet::update(float deltaTime) {
    if (!m_active) return;

    // Update cooldown timers
    m_timeSinceLastShot += deltaTime;
    m_throttleChangeTimer += deltaTime;

    // Calculate target speed based on throttle percentage
    float targetSpeed = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * (m_throttlePercent - MIN_THROTTLE) / (MAX_THROTTLE - MIN_THROTTLE);
    
    // Update velocity to match throttle speed in facing direction
    m_velocityX = std::cos(m_angle) * targetSpeed;
    m_velocityY = std::sin(m_angle) * targetSpeed;

    // Update position based on velocity
    m_x += m_velocityX * deltaTime;
    m_y += m_velocityY * deltaTime;
}

void PlayerJet::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) return;
    renderJet(renderer, 0, 0);
    renderHealthBar(renderer, 0, 0);
}

void PlayerJet::render(SDL_Renderer* renderer, int cameraX, int cameraY) {
    if (!m_active) return;
    renderJet(renderer, cameraX, cameraY);
    renderHealthBar(renderer, cameraX, cameraY);
}

void PlayerJet::renderJet(SDL_Renderer* renderer, int cameraX, int cameraY) {
    // Draw jet as a triangle pointing in direction of movement
    const float size = 25.0f;
    
    // Calculate triangle points relative to center
    float points[3][2] = {
        {size, 0},           // Nose
        {-size, size * 0.5f},   // Left wing
        {-size, -size * 0.5f}   // Right wing
    };

    // Rotate and translate points
    SDL_Point trianglePoints[4];
    for (int i = 0; i < 3; i++) {
        float rotatedX = points[i][0] * std::cos(m_angle) - points[i][1] * std::sin(m_angle);
        float rotatedY = points[i][0] * std::sin(m_angle) + points[i][1] * std::cos(m_angle);
        trianglePoints[i].x = static_cast<int>(m_x + rotatedX - cameraX);
        trianglePoints[i].y = static_cast<int>(m_y + rotatedY - cameraY);
    }
    trianglePoints[3] = trianglePoints[0];  // Close the triangle

    // Color based on player and health
    uint8_t r, g, b;
    float healthPercent = static_cast<float>(m_health) / m_maxHealth;
    
    if (m_player == Player::PLAYER1) {
        // Player 1: Blue with damage red tint
        r = static_cast<uint8_t>((1.0f - healthPercent) * 255);
        g = static_cast<uint8_t>(100 * healthPercent);
        b = 255;
    } else {
        // Player 2: Green with damage red tint
        r = static_cast<uint8_t>((1.0f - healthPercent) * 255);
        g = 255;
        b = static_cast<uint8_t>(100 * healthPercent);
    }

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    // Draw the triangle
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawLine(renderer, 
            trianglePoints[i].x, trianglePoints[i].y,
            trianglePoints[i + 1].x, trianglePoints[i + 1].y);
    }

    // Fill the triangle (simple fill)
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            SDL_RenderDrawLine(renderer,
                trianglePoints[i].x, trianglePoints[i].y,
                trianglePoints[j].x, trianglePoints[j].y);
        }
    }
}

void PlayerJet::renderHealthBar(SDL_Renderer* renderer, int cameraX, int cameraY) {
    const int barWidth = 50;
    const int barHeight = 5;
    const int barOffsetY = -35;

    // Background (red)
    SDL_Rect bgRect = {
        static_cast<int>(m_x - barWidth / 2 - cameraX),
        static_cast<int>(m_y + barOffsetY - cameraY),
        barWidth,
        barHeight
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // Health (green)
    float healthPercent = static_cast<float>(m_health) / m_maxHealth;
    SDL_Rect healthRect = {
        static_cast<int>(m_x - barWidth / 2 - cameraX),
        static_cast<int>(m_y + barOffsetY - cameraY),
        static_cast<int>(barWidth * healthPercent),
        barHeight
    };
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &healthRect);

    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
}

void PlayerJet::increaseThrottle() {
    if (!m_active) return;
    if (m_throttlePercent >= MAX_THROTTLE) return;  // Already at max
    if (m_throttleChangeTimer < THROTTLE_CHANGE_COOLDOWN) return;  // Cooldown not ready
    
    m_throttleChangeTimer = 0.0f;  // Reset timer
    int oldThrottle = m_throttlePercent;
    m_throttlePercent = std::min(m_throttlePercent + 1, MAX_THROTTLE);
    // Only log at 10% intervals to avoid spam
    if (m_throttlePercent % 10 == 0 && oldThrottle != m_throttlePercent) {
        std::cout << "Player " << (m_player == Player::PLAYER1 ? "1" : "2") 
                  << " throttle: " << m_throttlePercent << "%" << std::endl;
    }
}

void PlayerJet::decreaseThrottle() {
    if (!m_active) return;
    if (m_throttlePercent <= MIN_THROTTLE) return;  // Already at min
    if (m_throttleChangeTimer < THROTTLE_CHANGE_COOLDOWN) return;  // Cooldown not ready
    
    m_throttleChangeTimer = 0.0f;  // Reset timer
    int oldThrottle = m_throttlePercent;
    m_throttlePercent = std::max(m_throttlePercent - 1, MIN_THROTTLE);
    // Only log at 10% intervals to avoid spam
    if (m_throttlePercent % 10 == 0 && oldThrottle != m_throttlePercent) {
        std::cout << "Player " << (m_player == Player::PLAYER1 ? "1" : "2") 
                  << " throttle: " << m_throttlePercent << "%" << std::endl;
    }
}

void PlayerJet::shoot() {
    if (!m_active) return;
    
    // Check cooldown
    if (m_timeSinceLastShot < m_shootCooldown) {
        return;
    }
    
    m_timeSinceLastShot = 0.0f;
    
    std::cout << "Player " << (m_player == Player::PLAYER1 ? "1" : "2") 
              << " fired!" << std::endl;
    // TODO: Actually spawn bullet - will be handled by Game class
}

void PlayerJet::rotateLeft(float deltaTime) {
    if (!m_active) return;
    float turnMultiplier = getTurnRateMultiplier();
    m_angle -= m_rotationSpeed * turnMultiplier * deltaTime;
}

void PlayerJet::rotateRight(float deltaTime) {
    if (!m_active) return;
    float turnMultiplier = getTurnRateMultiplier();
    m_angle += m_rotationSpeed * turnMultiplier * deltaTime;
}

float PlayerJet::getTurnRateMultiplier() const {
    // Slower turn rate at higher speeds
    // At 20% throttle (min): 1.5x turn rate
    // At 100% throttle (max): 0.4x turn rate
    float speedPercent = (m_throttlePercent - MIN_THROTTLE) / static_cast<float>(MAX_THROTTLE - MIN_THROTTLE);
    return 1.5f - (speedPercent * 1.1f);
}

float PlayerJet::getSpeed() const {
    return std::sqrt(m_velocityX * m_velocityX + m_velocityY * m_velocityY);
}

bool PlayerJet::takeDamage(int damage) {
    if (!m_active) return false;

    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_active = false;
        std::cout << "Player " << (m_player == Player::PLAYER1 ? "1" : "2") 
                  << " destroyed!" << std::endl;
        return true;  // Destroyed
    }
    return false;  // Still alive
}

void PlayerJet::reset(float x, float y) {
    m_x = x;
    m_y = y;
    m_health = m_maxHealth;
    m_active = true;
    m_angle = m_player == Player::PLAYER1 ? 0.0f : M_PI;  // P1 faces right, P2 faces left
    m_throttlePercent = 50;  // Reset to 50% throttle
    float startSpeed = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * (m_throttlePercent - MIN_THROTTLE) / (MAX_THROTTLE - MIN_THROTTLE);
    m_velocityX = std::cos(m_angle) * startSpeed;
    m_velocityY = std::sin(m_angle) * startSpeed;
}

} // namespace BombingRun
