#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace BombingRun {

/**
 * @brief Player-controlled jet for Jet Fights mode
 * 
 * Controls:
 * - W: Increase thrust
 * - S: Decrease thrust
 * - A: Turn left
 * - D: Turn right
 * 
 * Thrust system:
 * - Range: 20% to 100%
 * - At 20%: Same speed as 8000lb bomber (2.0 px/frame)
 * - At 100%: Faster than fighter jet (>450 px/s)
 */
class PlayerControlledJet : public GameObject {
public:
    /**
     * @brief Construct player-controlled jet
     * @param x Starting X position
     * @param y Starting Y position
     */
    PlayerControlledJet(float x, float y);
    ~PlayerControlledJet() = default;

    /**
     * @brief Update jet physics and controls
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render jet to screen
     * @param renderer SDL renderer
     * @param font Optional font for HUD
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Handle control inputs
     * @param wPressed W key pressed
     * @param sPressed S key pressed
     * @param aPressed A key pressed
     * @param dPressed D key pressed
     * @param spacePressed Space key pressed
     */
    void handleInput(bool wPressed, bool sPressed, bool aPressed, bool dPressed, bool spacePressed);

    /**
     * @brief Fire a bullet
     * @return true if bullet was fired, false if on cooldown
     */
    bool fire();

    /**
     * @brief Check if ready to fire
     */
    bool canFire() const { return m_fireTimer <= 0.0f; }

    /**
     * @brief Get current thrust percentage
     * @return Thrust from 0.2 to 1.0
     */
    float getThrust() const { return m_thrust; }

    /**
     * @brief Get current angle in degrees
     * @return Angle (0-360)
     */
    float getAngleDegrees() const { return m_angleDegrees; }

    /**
     * @brief Get current rotation in radians
     * @return Angle in radians
     */
    float getRotation() const { return m_angleRadians; }

    /**
     * @brief Take damage from bullet hit
     */
    void takeDamage(int damage = 1);

    /**
     * @brief Get current health
     */
    int getHealth() const { return m_health; }

private:
    void updatePhysics(float deltaTime);
    void updateThrust(bool wPressed, bool sPressed, float deltaTime);
    void updateRotation(bool aPressed, bool dPressed, float deltaTime);

    float m_thrust;              // Current thrust (0.2 to 1.1)
    float m_angleDegrees;        // Current rotation angle in degrees
    float m_angleRadians;        // Current rotation angle in radians
    float m_turnRate;            // Degrees per second turn rate
    float m_thrustChangeRate;    // Thrust change per second
    float m_fireTimer;           // Time until next shot
    float m_fireRate;            // Shots per second
    int m_health;                // Current health (5 = full health)

    static constexpr float MIN_THRUST = 0.20f;           // 20% minimum
    static constexpr float MAX_THRUST = 1.00f;           // 100% normal maximum
    static constexpr float AFTERBURNER_THRUST = 1.10f;   // 110% afterburner
    static constexpr float MIN_SPEED = 120.0f;           // Speed at 20% thrust (8000lb bomber speed * 60fps)
    static constexpr float NORMAL_MAX_SPEED = 500.0f;    // Speed at 100% thrust (faster than fighter)
    static constexpr float AFTERBURNER_SPEED = 600.0f;   // Speed at 110% thrust
    static constexpr float TURN_RATE = 180.0f;           // 180 degrees per second
    static constexpr float THRUST_CHANGE_RATE = 1.0f;    // 100% change per second (1% per 0.01s)
    static constexpr float FIRE_RATE = 10.0f;            // 10 shots per second
};

} // namespace BombingRun
