#pragma once

#include "entities/Building.h"
#include "entities/CivilianBuilding.h"
#include "entities/MilitaryBuilding.h"
#include "entities/Runway.h"
#include <vector>
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace BombingRun {

/**
 * @brief Manages all buildings in the game
 * 
 * Handles building lifecycle, procedural generation,
 * collision detection, and rendering.
 */
class BuildingManager {
public:
    BuildingManager();
    ~BuildingManager() = default;

    /**
     * @brief Generate buildings for City map
     * @param mapWidth Map width in pixels
     * @param mapHeight Map height in pixels
     * @param count Number of buildings to generate
     */
    void generateCityBuildings(int mapWidth, int mapHeight, int count = 50);

    /**
     * @brief Generate military base with runway
     * @param mapWidth Map width in pixels
     * @param mapHeight Map height in pixels
     * @return Pointer to the created runway
     */
    Runway* generateMilitaryBase(int mapWidth, int mapHeight);

    /**
     * @brief Get runway pointer
     * @return Pointer to runway or nullptr
     */
    Runway* getRunway() { return m_runway.get(); }

    /**
     * @brief Clear all buildings
     */
    void clear();

    /**
     * @brief Update all buildings
     * @param deltaTime Time since last update
     */
    void update(float deltaTime);

    /**
     * @brief Render all buildings
     * @param renderer SDL renderer
     * @param font TTF font for text rendering (optional)
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr);

    /**
     * @brief Check collision with a point (bomb impact)
     * @param x X position
     * @param y Y position
     * @param radius Explosion radius
     * @param damage Damage amount
     * @return Number of buildings damaged
     */
    int checkCollision(float x, float y, float radius, int damage);

    /**
     * @brief Get all active buildings
     * @return Vector of building pointers
     */
    const std::vector<std::unique_ptr<Building>>& getBuildings() const {
        return m_buildings;
    }

    /**
     * @brief Get count of active (not destroyed) buildings
     * @return Number of active buildings
     */
    int getActiveCount() const;

    /**
     * @brief Get count of destroyed buildings
     * @return Number of destroyed buildings
     */
    int getDestroyedCount() const;

private:
    /**
     * @brief Check if position overlaps with existing building
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param margin Safety margin
     * @return true if overlapping
     */
    bool isOverlapping(float x, float y, float width, float height, float margin = 10.0f) const;

    std::vector<std::unique_ptr<Building>> m_buildings;
    std::unique_ptr<Runway> m_runway;
    SDL_Rect m_fenceRect;
    bool m_hasFence = false;
};

} // namespace BombingRun
