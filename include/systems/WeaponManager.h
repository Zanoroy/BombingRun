#pragma once

#include "entities/Bomb.h"
#include "entities/Bullet.h"
#include <vector>
#include <memory>
#include <SDL2/SDL.h>

namespace BombingRun {

/**
 * WeaponManager
 * Manages all active bombs and weapon systems
 */
class WeaponManager {
public:
    WeaponManager();
    ~WeaponManager() = default;

    void initialize(int screenWidth, int screenHeight);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    // Bomb management
    void dropBomb(float x, float y, BombType type, float bomberY = 0);
    void clearAllBombs();
    void cleanupInactiveBombs();  // Call after processing explosions
    
    // Bullet management
    void fireBullet(float x, float y, float velocityX, float velocityY, void* owner = nullptr, float size = 6.0f);
    void clearAllBullets();
    
    // Getters
    size_t getActiveBombCount() const { return m_bombs.size(); }
    const std::vector<std::unique_ptr<Bomb>>& getBombs() const { return m_bombs; }
    size_t getActiveBulletCount() const { return m_bullets.size(); }
    const std::vector<std::unique_ptr<Bullet>>& getBullets() const { return m_bullets; }
    
    // Current bomb type selection
    void setSelectedBombType(BombType type) { m_selectedBombType = type; }
    BombType getSelectedBombType() const { return m_selectedBombType; }
    
    // Bomb availability
    int getRemainingBombs(BombType type) const;
    bool canDropBomb(BombType type) const;
    void resetBombCounts();

private:
    std::vector<std::unique_ptr<Bomb>> m_bombs;
    std::vector<std::unique_ptr<Bullet>> m_bullets;
    BombType m_selectedBombType;
    int m_screenWidth;
    int m_screenHeight;
    
    // Track bomb usage per type
    int m_bombsUsed[7];  // One for each bomb type
    
    void removeInactiveBombs();
    void removeInactiveBullets();
};

} // namespace BombingRun
