#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>

namespace BombingRun {

class Bomber;

/**
 * @brief Fighter Jet that defends the airfield from bombers
 * 
 * Properties:
 * - 4 hit points (more durable than bombers)
 * - 579 bullets maximum
 * - 80% hit rate with predictive targeting
 * - Spawns when bombers approach airfield
 * - Returns to base when out of ammo or no targets
 */
class FighterJet : public GameObject {
public:
    enum class FighterState {
        SPAWNING,       // Flying in from edge
        PATROLLING,     // Circling the airfield
        ENGAGING,       // Actively attacking a bomber
        RETURNING,      // Heading back to runway
        LANDED          // On the ground, inactive
    };

    /**
     * @brief Construct a fighter jet
     * @param x Starting X position
     * @param y Starting Y position
     * @param runwayX Runway center X for landing
     * @param runwayY Runway center Y for landing
     */
    FighterJet(float x, float y, float runwayX, float runwayY);

    /**
     * @brief Update fighter jet logic
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render fighter jet to screen
     * @param renderer SDL renderer
     * @param font Optional font for debug text
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Take damage from a bomb explosion
     * @param damage Damage amount
     * @return true if fighter was destroyed
     */
    bool takeDamage(int damage);

    /**
     * @brief Set target bomber to engage
     * @param target Pointer to target bomber
     */
    void setTarget(Bomber* target);

    /**
     * @brief Fire at current target
     * @return true if bullet was fired
     */
    bool fire();

    /**
     * @brief Check if fighter is destroyed
     * @return true if health is 0
     */
    bool isDestroyed() const { return m_health <= 0; }

    /**
     * @brief Get current health
     * @return Current health points
     */
    int getHealth() const { return m_health; }

    /**
     * @brief Get maximum health
     * @return Maximum health points
     */
    int getMaxHealth() const { return m_maxHealth; }

    /**
     * @brief Get remaining bullets
     * @return Bullet count
     */
    int getRemainingBullets() const { return m_bulletsRemaining; }

    /**
     * @brief Get current state
     * @return Fighter state
     */
    FighterState getState() const { return m_state; }

    /**
     * @brief Get current target
     * @return Pointer to target bomber or nullptr
     */
    Bomber* getTarget() const { return m_target; }
    
    /**
     * @brief Load fighter jet sprites
     * @return true if successful
     */
    static bool loadSprites();

private:
    /**
     * @brief Update spawning behavior
     * @param deltaTime Time delta
     */
    void updateSpawning(float deltaTime);

    /**
     * @brief Update patrol behavior
     * @param deltaTime Time delta
     */
    void updatePatrolling(float deltaTime);

    /**
     * @brief Update engagement behavior
     * @param deltaTime Time delta
     */
    void updateEngaging(float deltaTime);

    /**
     * @brief Update return to base behavior
     * @param deltaTime Time delta
     */
    void updateReturning(float deltaTime);

    /**
     * @brief Calculate predicted interception point
     * @param targetX Target X position
     * @param targetY Target Y position
     * @param targetVx Target X velocity
     * @param targetVy Target Y velocity
     * @return true if interception calculated
     */
    bool calculateInterception(float targetX, float targetY, 
                               float targetVx, float targetVy);

    /**
     * @brief Get damage color based on health
     * @return Color for rendering
     */
    SDL_Color getDamageColor() const;

    int m_health;               // Current health (max 4)
    int m_maxHealth;            // Maximum health
    int m_bulletsRemaining;     // Bullets left (max 579)
    float m_speed;              // Flight speed
    float m_fireRate;           // Shots per second
    float m_fireTimer;          // Time until next shot
    float m_accuracy;           // Hit accuracy (0.8 = 80%)
    
    FighterState m_state;       // Current behavior state
    Bomber* m_target;           // Current target bomber
    
    float m_runwayX;            // Runway center X
    float m_runwayY;            // Runway center Y
    
    float m_patrolCenterX;      // Patrol circle center X
    float m_patrolCenterY;      // Patrol circle center Y
    float m_patrolRadius;       // Patrol circle radius
    float m_patrolAngle;        // Current patrol angle
    
    float m_interceptX;         // Calculated intercept point X
    float m_interceptY;         // Calculated intercept point Y
    
    int m_hitCount;             // Times hit for visual feedback
    float m_angle;              // Current rotation angle
    
    // Sprite constants
    static constexpr int SPRITE_SIZE = 175;    // Source sprite size
    static constexpr int RENDER_SIZE = 44;     // Rendered size (0.25x scale)
};

} // namespace BombingRun
