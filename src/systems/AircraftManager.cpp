#include "systems/AircraftManager.h"
#include "systems/CollisionDetector.h"
#include "systems/WeaponManager.h"
#include "entities/Bomb.h"
#include <iostream>
#include <cmath>

namespace BombingRun {

AircraftManager::AircraftManager()
    : m_screenWidth(800)
    , m_screenHeight(600)
    , m_randomEngine(std::random_device{}())
{
}

void AircraftManager::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    std::cout << "AircraftManager initialized for " << screenWidth << "x" << screenHeight << std::endl;
}

bool AircraftManager::loadSprites() {
    return Bomber::loadSprites();
}

void AircraftManager::spawnBomber(float targetX, float targetY, int bombType) {
    // Clamp bomb type to valid range
    if (bombType < 0) bombType = 0;
    if (bombType > 6) bombType = 6;

    // Spawn at bottom of screen, directly below target X position
    float spawnX = targetX;
    float spawnY = m_screenHeight + 50.0f;  // Below screen

    // If target not specified, use random position near top
    if (targetX < 0.0f) {
        spawnX = getRandomSpawnX();
        targetX = spawnX;  // Fly straight up
    }
    if (targetY < 0.0f) {
        targetY = 100.0f;  // Target near top of screen
    }

    // Get speed based on bomb type
    float speed = getSpeedForBombType(bombType);

    // Use single sprite (always 0)
    int spriteIndex = 0;

    // Create and add bomber
    auto bomber = std::make_unique<Bomber>(spawnX, spawnY, targetX, targetY, speed, bombType, spriteIndex);
    m_bombers.push_back(std::move(bomber));

    std::cout << "Spawned bomber at (" << spawnX << ", " << spawnY 
              << ") targeting (" << targetX << ", " << targetY 
              << ") with speed " << speed 
              << " bombType " << bombType
              << " sprite " << spriteIndex << std::endl;
}

void AircraftManager::update(float deltaTime, WeaponManager* weaponManager) {
    // Update all bombers
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            bomber->update(deltaTime);
            
            // If bomber reached target and hasn't dropped bombs yet, drop all bombs!
            if (weaponManager && bomber->hasReachedTarget() && !bomber->hasBombsDropped()) {
                BombType bombType = static_cast<BombType>(bomber->getBombType());
                
                // Get number of bombs this type carries
                int bombCount = Bomb::getMaxBombs(bombType);
                
                // Get target radius for this bomb type
                int targetRadius = Bomb::getConfig(bombType).targetRadius;
                
                std::cout << "Bomber dropping " << bombCount << " bombs of type " << bomber->getBombType() << std::endl;
                
                // Drop all bombs in random pattern within target circle
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
                std::uniform_real_distribution<float> radiusDist(0.0f, static_cast<float>(targetRadius));
                
                float bomberY = bomber->getY();  // Get bomber's current Y position
                
                for (int i = 0; i < bombCount; ++i) {
                    float angle = angleDist(m_randomEngine);
                    float radius = radiusDist(m_randomEngine);
                    
                    float offsetX = radius * cos(angle);
                    float offsetY = radius * sin(angle);
                    
                    float bombX = bomber->getTargetX() + offsetX;
                    float bombY = bomber->getTargetY() + offsetY;
                    
                    weaponManager->dropBomb(bombX, bombY, bombType, bomberY);
                }
                
                bomber->markBombsDropped();  // Mark as dropped
            }
        }
    }

    // Remove inactive bombers (destroyed or exited)
    m_bombers.erase(
        std::remove_if(m_bombers.begin(), m_bombers.end(),
            [](const std::unique_ptr<Bomber>& bomber) {
                return !bomber->isActive();
            }),
        m_bombers.end()
    );
}

void AircraftManager::render(SDL_Renderer* renderer) {
    for (auto& bomber : m_bombers) {
        if (bomber->isActive()) {
            bomber->render(renderer);
        }
    }
}

int AircraftManager::getActiveBomberCount() const {
    int count = 0;
    for (const auto& bomber : m_bombers) {
        if (bomber->isActive()) {
            ++count;
        }
    }
    return count;
}

Bomber* AircraftManager::checkCollision(float x, float y) {
    for (auto& bomber : m_bombers) {
        if (bomber->isActive()) {
            SDL_Rect bounds = bomber->getBounds();
            if (CollisionDetector::checkPointRect(x, y, bounds)) {
                return bomber.get();
            }
        }
    }
    return nullptr;
}

void AircraftManager::clearAll() {
    m_bombers.clear();
    std::cout << "Cleared all bombers" << std::endl;
}

float AircraftManager::getSpeedForBombType(int bombType) const {
    // Speed affects how fast bomber moves (pixels per frame at 60 FPS)
    // Multiply by 60 to convert to pixels per second
    return BOMB_SPEEDS[bombType] * 60.0f;
}

float AircraftManager::getRandomSpawnX() const {
    std::uniform_real_distribution<float> dist(100.0f, m_screenWidth - 100.0f);
    return dist(const_cast<std::mt19937&>(m_randomEngine));
}

float AircraftManager::getRandomTargetX() const {
    std::uniform_real_distribution<float> dist(50.0f, m_screenWidth - 50.0f);
    return dist(const_cast<std::mt19937&>(m_randomEngine));
}

float AircraftManager::getGroundY() const {
    // Target is at 2/3 down the screen (ground level)
    return m_screenHeight * 0.66f;
}

} // namespace BombingRun
