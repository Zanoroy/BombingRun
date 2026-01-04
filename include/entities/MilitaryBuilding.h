#pragma once

#include "entities/Building.h"
#include <SDL2/SDL.h>

namespace BombingRun {

// Forward declaration
class Runway;

/**
 * @brief Military building for BattleGround map
 * 
 * Hangars, fuel tanks, radar towers, and other military structures.
 * When destroyed, they reduce the runway's max health by 10 HP.
 */
class MilitaryBuilding : public Building {
public:
    /**
     * @brief Military structure type
     */
    enum class MilitaryType {
        HANGAR,         // Aircraft hangar
        FUEL_TANK,      // Fuel storage tank
        RADAR_TOWER,    // Radar/control tower
        BARRACKS,       // Personnel quarters
        AMMO_DEPOT      // Ammunition storage
    };

    /**
     * @brief Construct military building
     * @param x X position
     * @param y Y position
     * @param width Building width
     * @param height Building height
     * @param militaryType Type of military structure
     * @param runway Pointer to runway for health linkage
     */
    MilitaryBuilding(float x, float y, float width, float height, 
                     MilitaryType militaryType, Runway* runway = nullptr);

    /**
     * @brief Render building with military-specific appearance
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;

    /**
     * @brief Take damage and notify runway if destroyed
     * @param damage Damage amount
     * @return true if building was destroyed
     */
    bool takeDamage(int damage) override;

    /**
     * @brief Get military building type
     * @return Military type
     */
    MilitaryType getMilitaryType() const { return m_militaryType; }

    /**
     * @brief Set runway reference for health linkage
     * @param runway Pointer to runway
     */
    void setRunway(Runway* runway) { m_runway = runway; }

private:
    /**
     * @brief Get base color for military type
     * @return SDL_Color based on type
     */
    SDL_Color getMilitaryColor() const;

    /**
     * @brief Get health based on military type
     * @return Health points
     */
    int getMilitaryHealth() const;

    MilitaryType m_militaryType;
    Runway* m_runway;           // Pointer to runway for health linkage
    bool m_hasNotifiedRunway;   // Track if runway was notified of destruction
};

} // namespace BombingRun
