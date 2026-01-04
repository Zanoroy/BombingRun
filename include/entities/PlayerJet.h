#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace BombingRun {

/**
 * @brief Player-controlled jet for PVP dogfight mode
 * 
 * Features:
 * - Acceleration/deceleration physics
 * - Rotation controls
 * - Momentum-based movement
 * - Health system
 */
class PlayerJet : public GameObject {
public:
    enum class Player {
        PLAYER1,  // WASD controls
        PLAYER2   // Arrow key controls
    };

    PlayerJet(float x, float y, Player player);
    ~PlayerJet() override = default;

    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;
    
    // Render with camera offset for PVP mode
    void render(SDL_Renderer* renderer, int cameraX, int cameraY);

    // Input handling
    void increaseThrottle();
    void decreaseThrottle();
    void rotateLeft(float deltaTime);
    void rotateRight(float deltaTime);
    void shoot();

    // Physics
    float getVelocityX() const { return m_velocityX; }
    float getVelocityY() const { return m_velocityY; }
    float getAngle() const { return m_angle; }
    float getSpeed() const;
    int getThrottle() const { return m_throttlePercent; }
    float getTurnRateMultiplier() const;

    // Combat
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    bool takeDamage(int damage);
    void reset(float x, float y);

    // Collision
    float getRadius() const { return 20.0f; }  // Collision radius

    Player getPlayerType() const { return m_player; }

private:
    Player m_player;
    
    // Physics properties
    float m_velocityX;
    float m_velocityY;
    float m_angle;  // Rotation angle in radians
    int m_throttlePercent;  // 20-100%
    float m_rotationSpeed;

    // Health
    int m_health;
    int m_maxHealth;
    
    // Shooting
    float m_shootCooldown;
    float m_timeSinceLastShot;
    float m_throttleChangeTimer;  // Cooldown for throttle changes

    // Rendering
    void renderJet(SDL_Renderer* renderer, int cameraX = 0, int cameraY = 0);
    void renderHealthBar(SDL_Renderer* renderer, int cameraX = 0, int cameraY = 0);

    // Constants
    static constexpr float MIN_SPEED = 120.0f;           // 20% throttle (8000lb bomber speed ~120px/s)
    static constexpr float MAX_SPEED = 600.0f;           // 100% throttle (faster than fighter jet ~450px/s)
    static constexpr float ROTATION_SPEED = 3.5f;        // radians/s (slightly faster)
    static constexpr int MAX_HEALTH_VALUE = 100;
    static constexpr int MIN_THROTTLE = 20;
    static constexpr int MAX_THROTTLE = 100;
    static constexpr int THROTTLE_STEP = 10;             // 10% increments
    static constexpr float THROTTLE_CHANGE_COOLDOWN = 0.08f;  // seconds between throttle changes
    static constexpr float SHOOT_COOLDOWN = 0.15f;       // seconds between shots
    static constexpr float BULLET_SPEED = 600.0f;        // px/s
};

} // namespace BombingRun
