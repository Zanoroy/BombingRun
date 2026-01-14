#pragma once

#include "GameObject.h"
#include <SDL2/SDL.h>
#include <vector>

namespace BombingRun {

class Bomber;
class FighterJet;
class WeaponManager;

/**
 * @brief Anti-Aircraft Artillery Gun
 * 
 * Defends the runway by shooting at enemy aircraft with predictive targeting.
 * Fires bullets with lead calculation.
 */
class AAAGun : public GameObject {
public:
    /**
     * @brief Construct AAA gun at position
     * @param x X position
     * @param y Y position
     */
    AAAGun(float x, float y);
    ~AAAGun() = default;

    /**
     * @brief Update AAA gun - base implementation (unused)
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;
    
    /**
     * @brief Update AAA gun targeting and firing
     * @param deltaTime Time since last update
     * @param bombers List of active bombers to target
     * @param fighters List of active fighters to target
     * @param weaponManager Weapon manager to fire bullets
     */
    void updateTargeting(float deltaTime, const std::vector<Bomber*>& bombers, 
                        const std::vector<FighterJet*>& fighters,
                        WeaponManager* weaponManager);

    /**
     * @brief Render AAA gun and its tracer fire
     * @param renderer SDL renderer
     * @param font Font for text (unused)
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Get current target position (for visualization)
     */
    void getTargetPosition(float& x, float& y) const {
        x = m_targetX;
        y = m_targetY;
    }

    /**
     * @brief Check if currently firing
     */
    bool isFiring() const { return m_isFiring; }

    /**
     * @brief Get barrel angle
     */
    float getBarrelAngle() const { return m_barrelAngle; }

    /**
     * @brief Take damage from explosion
     * @param damage Amount of damage to take
     */
    void takeDamage(int damage);

    /**
     * @brief Get current health
     */
    int getHealth() const { return m_health; }

    /**
     * @brief Get maximum health
     */
    int getMaxHealth() const { return MAX_HEALTH; }

private:
    /**
     * @brief Calculate predicted intercept point for moving target
     * @param targetX Target current X
     * @param targetY Target current Y
     * @param targetVelX Target velocity X
     * @param targetVelY Target velocity Y
     * @param outX Output intercept X
     * @param outY Output intercept Y
     * @return true if intercept calculated, false if no solution
     */
    bool calculateInterceptPoint(float targetX, float targetY, 
                                  float targetVelX, float targetVelY,
                                  float& outX, float& outY);

    /**
     * @brief Fire bullet at current target
     * @param weaponManager Weapon manager to fire through
     */
    void fire(WeaponManager* weaponManager);

    float m_barrelAngle;         // Current barrel rotation (radians)
    float m_fireTimer;           // Time until next shot
    float m_fireRate;            // Shots per second
    float m_range;               // Maximum effective range
    float m_bulletSpeed;         // Bullet projectile speed
    float m_targetX;             // Current target position X
    float m_targetY;             // Current target position Y
    bool m_isFiring;             // Currently firing
    float m_muzzleFlashTimer;    // Timer for muzzle flash visual effect
    
    GameObject* m_currentTarget; // Currently targeted aircraft (bomber or fighter)
    int m_health;                // Current health points

    static constexpr float FIRE_RATE = 1.0f;        // 1 shot per second
    static constexpr float RANGE = 800.0f;          // 800 pixel range
    static constexpr float BULLET_SPEED = 1200.0f;  // Bullet speed in px/s
    static constexpr float TRACER_DURATION = 0.15f; // Tracer visible for 0.15 seconds
    static constexpr float ROTATION_SPEED = 3.0f;   // Radians per second
    static constexpr int DAMAGE = 1;                // Damage per hit
    static constexpr int MAX_HEALTH = 3;            // Takes 3 hits to destroy
};

} // namespace BombingRun
