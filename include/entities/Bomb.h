#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>

namespace BombingRun {

// Bomb type enumeration
enum class BombType {
    BOMB_100LB = 0,
    BOMB_250LB = 1,
    BOMB_500LB = 2,
    BOMB_1000LB = 3,
    BOMB_2000LB = 4,
    BOMB_4000LB = 5,
    BOMB_8000LB = 6,
    AIRSTRIKE = 7
};

// Bomb configuration structure
struct BombConfig {
    int maxBombs;           // Maximum bombs per bomber
    int craterSize;         // Crater size in pixels
    int targetRadius;       // Target acquisition radius
    int damage;             // Damage value
    float speed;            // Fall speed multiplier
    
    BombConfig(int max, int crater, int radius, int dmg, float spd)
        : maxBombs(max), craterSize(crater), targetRadius(radius), 
          damage(dmg), speed(spd) {}
};

/**
 * Base Bomb class
 * Represents a falling bomb with physics and collision
 */
class Bomb : public GameObject {
public:
    Bomb(float x, float y, BombType type);
    virtual ~Bomb() = default;

    // Core methods
    virtual void update(float deltaTime) override;
    virtual void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    // Bomb-specific methods
    void drop(float dropX, float dropY, float bomberY = 0);
    void markExploded() { m_shouldExplode = false; m_active = false; }
    bool hasHitGround() const { return m_hitGround; }
    bool shouldExplode() const { return m_shouldExplode; }
    
    // Getters
    BombType getType() const { return m_type; }
    int getCraterSize() const;
    int getTargetRadius() const;
    int getDamage() const;
    float getRotation() const { return m_rotation; }
    
    // Configuration access
    static const BombConfig& getConfig(BombType type);
    static int getMaxBombs(BombType type);

protected:
    BombType m_type;
    float m_fallSpeed;
    float m_rotation;           // Rotation angle for visual effect
    float m_rotationSpeed;      // How fast it rotates while falling
    bool m_hitGround;
    bool m_shouldExplode;
    float m_groundLevel;        // Y coordinate of ground
    
    // Static configuration table (includes airstrike at index 7)
    static const BombConfig BOMB_CONFIGS[8];
};

} // namespace BombingRun
