#include "systems/WeaponManager.h"
#include <iostream>
#include <algorithm>

namespace BombingRun {

WeaponManager::WeaponManager()
    : m_selectedBombType(BombType::BOMB_500LB)  // Default to 500lb
    , m_screenWidth(0)
    , m_screenHeight(0)
{
    // Initialize bomb usage counters
    for (int i = 0; i < 7; i++) {
        m_bombsUsed[i] = 0;
    }
}

void WeaponManager::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    std::cout << "WeaponManager initialized for " << screenWidth << "x" << screenHeight << std::endl;
}

void WeaponManager::update(float deltaTime) {
    // Update all active bombs
    for (auto& bomb : m_bombs) {
        if (bomb) {
            bomb->update(deltaTime);
        }
    }
    
    // Update all active bullets
    for (auto& bullet : m_bullets) {
        if (bullet) {
            bullet->update(deltaTime);
        }
    }
    
    // Note: Don't remove inactive bombs here
    // Let the game loop check for explosions first
    // Then call removeInactiveBombs() from Game after processing explosions
}

void WeaponManager::render(SDL_Renderer* renderer) {
    // Render all bombs (even those that just hit ground for one frame)
    for (const auto& bomb : m_bombs) {
        if (bomb) {
            bomb->render(renderer);
        }
    }
    
    // Render all bullets
    for (const auto& bullet : m_bullets) {
        if (bullet) {
            bullet->render(renderer);
        }
    }
}

void WeaponManager::dropBomb(float x, float y, BombType type, float bomberY) {
    // Create and drop the bomb
    auto bomb = std::make_unique<Bomb>(x, y, type);
    bomb->drop(x, y, bomberY);
    
    std::cout << "Dropped bomb type " << static_cast<int>(type) 
              << " at (" << x << ", " << y << ")" << std::endl;
    
    m_bombs.push_back(std::move(bomb));
}

void WeaponManager::clearAllBombs() {
    m_bombs.clear();
    resetBombCounts();
}

void WeaponManager::fireBullet(float x, float y, float velocityX, float velocityY, void* owner, float size) {
    // Calculate target position from velocity (project 1 second ahead)
    float targetX = x + velocityX;
    float targetY = y + velocityY;
    
    auto bullet = std::make_unique<Bullet>(x, y, targetX, targetY, 800.0f, owner, size);
    // Note: Runway Y will be set by Game after creation if available
    m_bullets.push_back(std::move(bullet));
    
    std::cout << "Fired bullet from (" << x << ", " << y << ") toward (" << targetX << ", " << targetY << ")" << std::endl;
}

void WeaponManager::clearAllBullets() {
    m_bullets.clear();
}

void WeaponManager::cleanupInactiveBombs() {
    removeInactiveBombs();
    removeInactiveBullets();
}

void WeaponManager::resetBombCounts() {
    for (int i = 0; i < 7; i++) {
        m_bombsUsed[i] = 0;
    }
    std::cout << "Bomb counts reset" << std::endl;
}

int WeaponManager::getRemainingBombs(BombType type) const {
    int typeIndex = static_cast<int>(type);
    int maxBombs = Bomb::getMaxBombs(type);
    int used = m_bombsUsed[typeIndex];
    return maxBombs - used;
}

bool WeaponManager::canDropBomb(BombType type) const {
    return getRemainingBombs(type) > 0;
}

void WeaponManager::removeInactiveBombs() {
    // Remove bombs that have exploded or are inactive
    m_bombs.erase(
        std::remove_if(m_bombs.begin(), m_bombs.end(),
            [](const std::unique_ptr<Bomb>& bomb) {
                return !bomb || !bomb->isActive();
            }),
        m_bombs.end()
    );
}

void WeaponManager::removeInactiveBullets() {
    // Remove bullets that have expired or hit something
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const std::unique_ptr<Bullet>& bullet) {
                return !bullet || !bullet->isActive();
            }),
        m_bullets.end()
    );
}

} // namespace BombingRun
