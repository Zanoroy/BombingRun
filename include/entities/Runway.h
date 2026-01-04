#pragma once

#include "entities/GameObject.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace BombingRun {

/**
 * @brief Military runway that can be damaged and regenerates
 * 
 * Central structure for the BattleGround map. Has 1000 HP,
 * regenerates over time, and its max health is linked to
 * surrounding military buildings.
 */
class Runway : public GameObject {
public:
    /**
     * @brief Construct a runway
     * @param x X position (center top of map)
     * @param y Y position
     * @param width Runway width
     * @param height Runway height (vertical runway)
     */
    Runway(float x, float y, float width, float height);

    /**
     * @brief Update runway state (regeneration)
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render runway to screen
     * @param renderer SDL renderer
     * @param font TTF font for rendering text (optional)
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Deal damage to runway
     * @param damage Damage amount
     * @return true if runway was destroyed
     */
    bool takeDamage(int damage);

    /**
     * @brief Reduce max health (when military building destroyed)
     * @param reduction Amount to reduce max HP (usually 10)
     */
    void reduceMaxHealth(int reduction);

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
     * @brief Check if runway is destroyed
     * @return true if health is 0
     */
    bool isDestroyed() const { return m_health <= 0; }

    /**
     * @brief Get damage percentage (0.0 = no damage, 1.0 = destroyed)
     * @return Damage ratio
     */
    float getDamageRatio() const {
        if (m_maxHealth == 0) return 1.0f;
        return 1.0f - (static_cast<float>(m_health) / static_cast<float>(m_maxHealth));
    }

private:
    /**
     * @brief Get color based on damage state
     * @return SDL_Color for rendering
     */
    SDL_Color getDamageColor() const;

    int m_health;               // Current health
    int m_maxHealth;            // Maximum health (reduces when buildings destroyed)
    int m_baseMaxHealth;        // Base maximum health (1000)
    float m_regenRate;          // Health regeneration per second
    float m_regenTimer;         // Timer for regeneration
    SDL_Color m_baseColor;      // Base color for rendering
};

} // namespace BombingRun
