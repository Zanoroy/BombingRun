#pragma once

#include "entities/Bomber.h"
#include "entities/FighterJet.h"
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
 * Handles bomber and fighter jet spawning, lifecycle, and interactions.
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
     * @brief Set runway position for fighter jets
     * @param runwayX Runway center X
     * @param runwayY Runway center Y
     */
    void setRunwayPosition(float runwayX, float runwayY);

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
     * @brief Spawn a fighter jet to defend the airfield
     */
    void spawnFighterJet();

    /**
     * @brief Update all aircraft
     * @param deltaTime Time since last update
     * @param weaponManager Optional weapon manager to drop bombs and fire bullets
     */
    void update(float deltaTime, WeaponManager* weaponManager = nullptr);

    /**
     * @brief Render all aircraft
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer);

    /**
     * @brief Get number of active bombers
     * @return Active bomber count
     */
    int getActiveBomberCount() const;

    /**
     * @brief Get number of active fighter jets
     * @return Active fighter jet count
     */
    int getActiveFighterCount() const;

    /**
     * @brief Check collision with point (for bullet hits on bombers)
     * @param x Point X
     * @param y Point Y
     * @return Pointer to hit bomber or nullptr
     */
    Bomber* checkBomberCollision(float x, float y);

    /**
     * @brief Get all active bombers
     * @return Vector of bomber pointers
     */
    std::vector<Bomber*> getActiveBombers();

    /**
     * @brief Get all active fighter jets
     * @return Vector of fighter jet pointers
     */
    std::vector<FighterJet*> getActiveFighters();

    /**
     * @brief Clear all aircraft
     */
    void clearAll();

private:
    float getSpeedForBombType(int bombType) const;
    float getRandomSpawnX() const;
    float getRandomTargetX() const;
    float getGroundY() const;
    
    /**
     * @brief Assign targets to fighters from available bombers
     */
    void assignFighterTargets();
    
    /**
     * @brief Check if automatic fighter spawning is needed
     */
    void checkAutoSpawn();

    std::vector<std::unique_ptr<Bomber>> m_bombers;
    std::vector<std::unique_ptr<FighterJet>> m_fighters;
    
    int m_screenWidth;
    int m_screenHeight;
    float m_runwayX;
    float m_runwayY;
    bool m_runwaySet;
    
    std::mt19937 m_randomEngine;
    
    float m_autoSpawnTimer;  // Timer for automatic fighter spawning
    
    // Bomb type speeds from specification
    static constexpr float BOMB_SPEEDS[7] = {6.0f, 5.5f, 5.0f, 4.0f, 3.0f, 2.5f, 2.0f};
};

} // namespace BombingRun
