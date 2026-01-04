#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>

namespace BombingRun {

/**
 * @brief Bomber aircraft that drops bombs on targets
 * 
 * Bombers fly from top of screen to target location, drop bombs,
 * and exit. They have 3 HP and show visual damage indicators.
 */
class Bomber : public GameObject {
public:
    /**
     * @brief Construct bomber with spawn position and target
     * @param spawnX Spawn X position
     * @param spawnY Spawn Y position (usually negative, above screen)
     * @param targetX Target X position on ground
     * @param targetY Target Y position on ground
     * @param speed Movement speed
     * @param bombType Which bomb type to carry (0-6)
     * @param spriteIndex Which plane sprite to use (0-4)
     */
    Bomber(float spawnX, float spawnY, float targetX, float targetY, float speed, int bombType = 2, int spriteIndex = 0);

    /**
     * @brief Update bomber position and logic
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render bomber to screen
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Deal damage to bomber
     * @param damage Damage amount (usually 1)
     */
    void takeDamage(int damage);

    /**
     * @brief Check if bomber reached target
     * @return true if at target position
     */
    bool hasReachedTarget() const { return m_reachedTarget; }

    /**
     * @brief Check if bomber has exited screen
     * @return true if off screen
     */
    bool hasExited() const { return m_exited; }

    // Health accessors
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    bool isDamaged() const { return m_health < m_maxHealth; }

    // Target accessors
    float getTargetX() const { return m_targetX; }
    float getTargetY() const { return m_targetY; }
    
    // Bomb type accessor
    int getBombType() const { return m_bombType; }
    bool hasBombsDropped() const { return m_bombsDropped; }
    void markBombsDropped() { m_bombsDropped = true; }

    // Rotation accessor
    float getAngle() const { return m_angle; }

    /**
     * @brief Load plane sprites into texture manager (call once at startup)
     * @return true if successful
     */
    static bool loadSprites();

private:
    void calculateVelocity();
    void calculateAngle();
    SDL_Color getDamageColor() const;

    float m_targetX;            // Target X position
    float m_targetY;            // Target Y position
    float m_speed;              // Movement speed
    float m_angle;              // Rotation angle in degrees
    int m_health;               // Current health
    int m_maxHealth;            // Maximum health (3)
    int m_spriteIndex;          // Which plane sprite (0-4)
    int m_bombType;             // Which bomb type this bomber carries (0-6)
    bool m_reachedTarget;       // Has reached target?
    bool m_bombsDropped;        // Has dropped bombs at target?
    bool m_exited;              // Has exited screen?
    float m_smokeTimer;         // Timer for smoke effects

    // Visual properties
    static constexpr float BOMBER_WIDTH = 40.0f;
    static constexpr float BOMBER_HEIGHT = 40.0f;
    static constexpr int MAX_BOMBER_HEALTH = 3;
    static constexpr float SMOKE_INTERVAL = 0.3f;  // Smoke puff every 0.3s when damaged
    static constexpr float TARGET_THRESHOLD = 5.0f; // Distance to consider "at target"
    static constexpr int SPRITE_SIZE = 175;  // Original sprite size
    static constexpr int RENDER_SIZE = 50;   // Scaled render size
};

} // namespace BombingRun
