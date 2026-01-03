#pragma once

#include "entities/Bomber.h"
#include "utils/ObjectPool.h"
#include <SDL2/SDL.h>
#include <random>
#include <memory>
#include <vector>

namespace BombingRun {

// Forward declaration
class WeaponManager;

/**
 * @brief Manages all aircraft in the game
 * 
 * Handles bomber spawning, lifecycle, and interactions.
 * Uses object pooling for efficient memory management.
 */
class AircraftManager {
public:
    AircraftManager();
    ~AircraftManager() = default;

    /**
     * @brief Initialize aircraft manager with screen dimensions
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     */
    void initialize(int screenWidth, int screenHeight);

    /**
     * @brief Load aircraft sprites (must be called after TextureManager is ready)
     * @return true if successful
     */
    bool loadSprites();

    /**
     * @brief Spawn a bomber with specific bomb type
     * @param targetX Target X position (if negative, random)
     * @param targetY Target Y position (if negative, random ground position)
     * @param bombType Bomb type determines speed (0-6 for 100lb-8000lb)
     */
    void spawnBomber(float targetX = -1.0f, float targetY = -1.0f, int bombType = 0);

    /**
     * @brief Update all bombers
     * @param deltaTime Time since last update
     * @param weaponManager Optional weapon manager to drop bombs
     */
    void update(float deltaTime, WeaponManager* weaponManager = nullptr);

    /**
     * @brief Render all bombers
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer);

    /**
     * @brief Get number of active bombers
     * @return Active bomber count
     */
    int getActiveBomberCount() const;

    /**
     * @brief Check collision with point (for bullet hits)
     * @param x Point X
     * @param y Point Y
     * @return Pointer to hit bomber or nullptr
     */
    Bomber* checkCollision(float x, float y);

    /**
     * @brief Clear all bombers
     */
    void clearAll();

private:
    float getSpeedForBombType(int bombType) const;
    float getRandomSpawnX() const;
    float getRandomTargetX() const;
    float getGroundY() const;

    std::vector<std::unique_ptr<Bomber>> m_bombers;
    int m_screenWidth;
    int m_screenHeight;
    std::mt19937 m_randomEngine;
    
    // Bomb type speeds from specification
    static constexpr float BOMB_SPEEDS[7] = {6.0f, 5.5f, 5.0f, 4.0f, 3.0f, 2.5f, 2.0f};
};

} // namespace BombingRun
