#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace BombingRun {

/**
 * @brief Second player-controlled jet for Jet Fights mode (RED TEAM)
 * 
 * Controls:
 * - Arrow Up: Increase thrust
 * - Arrow Down: Decrease thrust
 * - Arrow Left: Turn left
 * - Arrow Right: Turn right
 * - Right Ctrl: Shoot
 * 
 * Thrust system:
 * - Range: 20% to 110%
 * - At 20%: Minimum speed
 * - At 100%: Normal max speed
 * - At 110%: Afterburner mode
 */
class Player2Jet : public GameObject {
public:
    /**
     * @brief Construct player 2 jet
     * @param x Starting X position
     * @param y Starting Y position
     */
    Player2Jet(float x, float y);
    ~Player2Jet() = default;

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
     * @param upPressed Arrow Up key pressed
     * @param downPressed Arrow Down key pressed
     * @param leftPressed Arrow Left key pressed
     * @param rightPressed Arrow Right key pressed
     * @param rctrlPressed Right Ctrl key pressed
     */
    void handleInput(bool upPressed, bool downPressed, bool leftPressed, bool rightPressed, bool rctrlPressed);

    /**
     * @brief Fire a bullet
     * @return true if bullet was fired, false if on cooldown
     */
    bool fire();

    /**
     * @brief Check if ready to fire
     */
    bool canFire() const { return m_player2FireTimer <= 0.0f; }

    /**
     * @brief Get current thrust percentage
     * @return Thrust from 0.2 to 1.1
     */
    float getThrust() const { return m_player2Thrust; }

    /**
     * @brief Get current angle in degrees
     * @return Angle (0-360)
     */
    float getAngleDegrees() const { return m_player2AngleDegrees; }

    /**
     * @brief Get current rotation in radians
     * @return Angle in radians
     */
    float getRotation() const { return m_player2AngleRadians; }

    /**
     * @brief Take damage from bullet hit
     */
    void takeDamage(int damage = 1);

    /**
     * @brief Get current health
     */
    int getHealth() const { return m_player2Health; }

private:
    void updatePhysics(float deltaTime);
    void updateThrust(bool upPressed, bool downPressed, float deltaTime);
    void updateRotation(bool leftPressed, bool rightPressed, float deltaTime);

    // Player 2 specific variables (separate from Player 1)
    float m_player2Thrust;              // Current thrust (0.2 to 1.1)
    float m_player2AngleDegrees;        // Current rotation angle in degrees
    float m_player2AngleRadians;        // Current rotation angle in radians
    float m_player2TurnRate;            // Degrees per second turn rate
    float m_player2ThrustChangeRate;    // Thrust change per second
    float m_player2FireTimer;           // Time until next shot
    float m_player2FireRate;            // Shots per second
    int m_player2Health;                // Current health (5 = full health)

    static constexpr float P2_MIN_THRUST = 0.20f;           // 20% minimum
    static constexpr float P2_MAX_THRUST = 1.00f;           // 100% normal maximum
    static constexpr float P2_AFTERBURNER_THRUST = 1.10f;   // 110% afterburner
    static constexpr float P2_MIN_SPEED = 120.0f;           // Speed at 20% thrust
    static constexpr float P2_NORMAL_MAX_SPEED = 500.0f;    // Speed at 100% thrust
    static constexpr float P2_AFTERBURNER_SPEED = 600.0f;   // Speed at 110% thrust
    static constexpr float P2_TURN_RATE = 180.0f;           // 180 degrees per second
    static constexpr float P2_THRUST_CHANGE_RATE = 1.0f;    // 100% change per second
    static constexpr float P2_FIRE_RATE = 10.0f;            // 10 shots per second
};

} // namespace BombingRun
