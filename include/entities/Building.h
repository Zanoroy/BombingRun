#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>

namespace BombingRun {

/**
 * @brief Base class for all building structures
 * 
 * Buildings can be destroyed by bombs and show progressive damage states.
 * They are static objects placed on the map.
 */
class Building : public GameObject {
public:
    /**
     * @brief Building type enumeration
     */
    enum class Type {
        CIVILIAN,    // City buildings
        MILITARY     // Base structures (hangars, fuel tanks, etc.)
    };

    /**
     * @brief Construct a building at position
     * @param x X position
     * @param y Y position
     * @param width Building width in pixels
     * @param height Building height in pixels
     * @param maxHealth Maximum health points
     * @param type Building type
     */
    Building(float x, float y, float width, float height, int maxHealth, Type type);

    /**
     * @brief Update building state
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render building to screen
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer) override;

    /**
     * @brief Deal damage to building
     * @param damage Damage amount
     * @return true if building was destroyed by this hit
     */
    virtual bool takeDamage(int damage);

    /**
     * @brief Check if building is destroyed
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
     * @brief Get building type
     * @return Building type
     */
    Type getType() const { return m_type; }

    /**
     * @brief Get damage percentage (0.0 = no damage, 1.0 = destroyed)
     * @return Damage ratio
     */
    float getDamageRatio() const {
        if (m_maxHealth == 0) return 1.0f;
        return 1.0f - (static_cast<float>(m_health) / static_cast<float>(m_maxHealth));
    }

protected:
    /**
     * @brief Get color based on damage state
     * @return SDL_Color for rendering
     */
    SDL_Color getDamageColor() const;

    int m_health;           // Current health
    int m_maxHealth;        // Maximum health
    Type m_type;            // Building type
    SDL_Color m_baseColor;  // Base color for rendering
};

} // namespace BombingRun
