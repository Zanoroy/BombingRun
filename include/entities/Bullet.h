#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>

namespace BombingRun {

class FighterJet;  // Forward declaration

/**
 * @brief Bullet fired by fighter jets at bombers
 * 
 * Fast-moving projectile with simple physics
 */
class Bullet : public GameObject {
public:
    /**
     * @brief Construct a bullet
     * @param x Starting X position
     * @param y Starting Y position
     * @param targetX Target X position
     * @param targetY Target Y position
     * @param speed Bullet speed
     * @param owner Pointer to the fighter that fired this bullet (optional)
     * @param size Bullet size (width and height)
     */
    Bullet(float x, float y, float targetX, float targetY, float speed = 800.0f, void* owner = nullptr, float size = 6.0f);

    /**
     * @brief Update bullet position
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render bullet to screen
     * @param renderer SDL renderer
     * @param font Optional font (unused)
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Check if bullet should be removed
     * @return true if off-screen or hit target
     */
    bool shouldRemove() const { return !m_active; }

    /**
     * @brief Mark bullet as hit
     */
    void markHit() { m_active = false; }
    
    /**
     * @brief Set runway Y position for boundary checking
     */
    void setRunwayY(float runwayY) { m_runwayY = runwayY; }
    
    /**
     * @brief Get the owner of this bullet (fighter that fired it)
     * @return Pointer to owner fighter, or nullptr if no owner
     */
    void* getOwner() const { return m_owner; }
    
    /**
     * @brief Check if bullet can hit the specified fighter
     * @param fighter Pointer to fighter to check
     * @return true if bullet can damage this fighter
     */
    bool canHit(void* fighter) const { return fighter != m_owner || m_lifetime > 0.1f; }

private:
    float m_speed;              // Bullet speed
    float m_directionX;         // Normalized direction X
    float m_directionY;         // Normalized direction Y
    float m_lifetime;           // Time alive (for cleanup)
    float m_maxLifetime;        // Max time before auto-removal
    float m_runwayY;            // Runway Y position for boundary checking
    void* m_owner;              // Pointer to fighter that fired this bullet (prevents self-hit)
};

} // namespace BombingRun
