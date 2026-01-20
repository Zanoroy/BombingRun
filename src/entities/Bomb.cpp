#include "entities/Bomb.h"
#include <cmath>
#include <iostream>

namespace BombingRun {

// Bomb configuration table from specification
// Format: {maxBombs, craterSize, targetRadius, damage, speed}
const BombConfig Bomb::BOMB_CONFIGS[9] = {
    BombConfig(10, 15, 60, 3, 6.0f),    // 100lb - increased damage
    BombConfig(8, 20, 75, 5, 5.5f),     // 250lb - increased damage
    BombConfig(6, 25, 90, 8, 5.0f),     // 500lb - increased damage
    BombConfig(3, 40, 120, 15, 4.0f),   // 1000lb - increased damage
    BombConfig(1, 80, 90, 30, 3.0f),    // 2000lb - increased damage
    BombConfig(1, 160, 60, 50, 2.5f),   // 4000lb - increased damage
    BombConfig(1, 320, 60, 100, 2.0f),  // 8000lb - increased damage
    BombConfig(15, 15, 90, 1, 5.0f),    // Airstrike - 15 small bombs per plane
    BombConfig(1, 1600, 200, 500, 1.0f) // NUKE - 5x larger crater than 8000lb, massive damage
};

Bomb::Bomb(float x, float y, BombType type)
    : GameObject(x, y)
    , m_type(type)
    , m_fallSpeed(200.0f)  // Will be adjusted based on bomb type
    , m_rotation(0.0f)
    , m_rotationSpeed(360.0f)  // 360 degrees per second for spinning effect
    , m_hitGround(false)
    , m_shouldExplode(false)
    , m_groundLevel(y)  // Will be set when dropped
{
    // Adjust fall speed based on bomb weight - heavier bombs fall slower (more stable/accurate)
    switch(type) {
        case BombType::BOMB_100LB:  m_fallSpeed = 250.0f; break;  // Lightest - faster
        case BombType::BOMB_250LB:  m_fallSpeed = 220.0f; break;
        case BombType::BOMB_500LB:  m_fallSpeed = 200.0f; break;
        case BombType::BOMB_1000LB: m_fallSpeed = 180.0f; break;
        case BombType::BOMB_2000LB: m_fallSpeed = 150.0f; break;
        case BombType::BOMB_4000LB: m_fallSpeed = 120.0f; break;
        case BombType::BOMB_8000LB: m_fallSpeed = 100.0f; break; // Heaviest - slowest
        case BombType::AIRSTRIKE:   m_fallSpeed = 250.0f; break; // Same as 100lb
    }
    
    m_width = 8.0f;   // Small size for bomb
    m_height = 12.0f;
    m_velocityY = m_fallSpeed;  // Start falling immediately
    m_active = false;  // Will be activated when dropped
    
    std::cout << "Created bomb type " << static_cast<int>(type) 
              << " at (" << x << ", " << y << ")" << std::endl;
}

void Bomb::drop(float dropX, float dropY, float bomberY) {
    m_x = dropX;
    m_y = bomberY;  // Start at bomber's height
    m_groundLevel = dropY;  // Fall to this target position
    m_active = true;
    m_velocityY = m_fallSpeed;
    
    std::cout << "Dropped bomb from Y=" << bomberY << " falling to Y=" << dropY << std::endl;
}

void Bomb::update(float deltaTime) {
    if (!m_active) {
        return;
    }
    
    // If already hit ground, stay inactive (explosion already happened)
    if (m_hitGround) {
        return;
    }

    // Apply gravity/fall speed
    m_y += m_velocityY * deltaTime;
    
    // Rotate while falling for visual effect
    m_rotation += m_rotationSpeed * deltaTime;
    if (m_rotation >= 360.0f) {
        m_rotation -= 360.0f;
    }

    // Check if hit ground
    if (m_y >= m_groundLevel) {
        m_y = m_groundLevel;
        m_hitGround = true;
        m_shouldExplode = true;
        
        std::cout << "Bomb hit ground at (" << m_x << ", " << m_y << ")" << std::endl;
    }
}

void Bomb::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active && !m_hitGround) {
        return;
    }

    // Render bomb as a small falling object
    // Color varies by bomb type (heavier = darker)
    int typeIndex = static_cast<int>(m_type);
    uint8_t colorValue = 255 - (typeIndex * 30);  // Darker for heavier bombs
    
    SDL_SetRenderDrawColor(renderer, colorValue, colorValue, colorValue, 255);
    
    // Simple rectangular bomb shape
    SDL_Rect bombRect = {
        static_cast<int>(m_x - m_width / 2),
        static_cast<int>(m_y - m_height / 2),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    
    SDL_RenderFillRect(renderer, &bombRect);
    
    // Add a small tail fin indicator (red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect finRect = {
        static_cast<int>(m_x - 2),
        static_cast<int>(m_y - m_height / 2 - 2),
        4,
        4
    };
    SDL_RenderFillRect(renderer, &finRect);
}

int Bomb::getCraterSize() const {
    return BOMB_CONFIGS[static_cast<int>(m_type)].craterSize;
}

int Bomb::getTargetRadius() const {
    return BOMB_CONFIGS[static_cast<int>(m_type)].targetRadius;
}

int Bomb::getDamage() const {
    return BOMB_CONFIGS[static_cast<int>(m_type)].damage;
}

const BombConfig& Bomb::getConfig(BombType type) {
    return BOMB_CONFIGS[static_cast<int>(type)];
}

int Bomb::getMaxBombs(BombType type) {
    return BOMB_CONFIGS[static_cast<int>(type)].maxBombs;
}

} // namespace BombingRun
