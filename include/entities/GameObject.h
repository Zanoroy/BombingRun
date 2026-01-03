#pragma once

#include <SDL2/SDL.h>
#include <memory>

namespace BombingRun {

/**
 * @brief Base class for all game objects
 * 
 * Provides common interface for position, rendering, updating,
 * and collision detection. All game entities inherit from this.
 */
class GameObject {
public:
    GameObject(float x = 0.0f, float y = 0.0f);
    virtual ~GameObject() = default;

    /**
     * @brief Update object logic
     * @param deltaTime Time since last update in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Render object to screen
     * @param renderer SDL renderer to draw with
     */
    virtual void render(SDL_Renderer* renderer) = 0;

    /**
     * @brief Check if object is active/alive
     * @return true if active, false if should be removed
     */
    virtual bool isActive() const { return m_active; }

    /**
     * @brief Mark object for removal
     */
    virtual void destroy() { m_active = false; }

    // Position accessors
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setPosition(float x, float y) { m_x = x; m_y = y; }

    // Velocity accessors
    float getVelocityX() const { return m_velocityX; }
    float getVelocityY() const { return m_velocityY; }
    void setVelocity(float vx, float vy) { m_velocityX = vx; m_velocityY = vy; }

    // Dimension accessors
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    void setDimensions(float width, float height) { m_width = width; m_height = height; }

    /**
     * @brief Get bounding box for collision detection
     * @return SDL_Rect representing object bounds
     */
    SDL_Rect getBounds() const {
        return SDL_Rect{
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
    }

protected:
    float m_x;              // X position
    float m_y;              // Y position
    float m_velocityX;      // X velocity
    float m_velocityY;      // Y velocity
    float m_width;          // Width in pixels
    float m_height;         // Height in pixels
    bool m_active;          // Is object active?
};

} // namespace BombingRun
