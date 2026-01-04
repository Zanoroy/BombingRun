#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "systems/BuildingManager.h"

namespace BombingRun {

/**
 * @brief Structure types for map objects
 */
enum class StructureType {
    ROAD_HORIZONTAL,
    ROAD_VERTICAL,
    ROAD_INTERSECTION,
    HOUSE_SMALL,
    HOUSE_MEDIUM,
    HOUSE_LARGE,
    HOSPITAL,
    INDUSTRIAL,
    RIVER_HORIZONTAL,
    RIVER_VERTICAL,
    BRIDGE_HORIZONTAL,
    BRIDGE_VERTICAL,
    POWERLINE_HORIZONTAL,
    POWERLINE_VERTICAL,
    POWERLINE_TOWER
};

/**
 * @brief Map structure/object
 */
struct MapObject {
    StructureType type;
    SDL_Rect bounds;
    SDL_Color color;
    bool destructible;
    int health;
    
    MapObject(StructureType t, int x, int y, int w, int h, SDL_Color c, bool dest = false, int hp = 0)
        : type(t), bounds{x, y, w, h}, color(c), destructible(dest), health(hp) {}
};

/**
 * @brief Game map with structures and terrain
 * 
 * Handles rendering and management of map elements including
 * roads, buildings, rivers, and other structures.
 */
class Map {
public:
    Map(int width, int height);
    ~Map() = default;

    /**
     * @brief Load map from data
     * @param mapName Map identifier
     * @return true if successful
     */
    bool loadMap(const std::string& mapName);

    /**
     * @brief Render all map elements
     * @param renderer SDL renderer
     * @param font TTF font for text rendering (optional)
     */
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr);

    /**
     * @brief Check collision with map object
     * @param x Point X
     * @param y Point Y
     * @return Pointer to hit object or nullptr
     */
    MapObject* checkCollision(float x, float y);

    /**
     * @brief Damage buildings within explosion radius
     * @param x Explosion center X
     * @param y Explosion center Y
     * @param radius Explosion radius
     * @param damage Damage to apply
     * @return Number of buildings destroyed
     */
    int damageBuildings(float x, float y, float radius, int damage);

    /**
     * @brief Get count of destroyed buildings
     */
    int getDestroyedBuildingCount() const { return m_destroyedBuildings; }

    /**
     * @brief Get map dimensions
     */
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /**
     * @brief Get ground level Y coordinate
     */
    float getGroundY() const { return m_height * 0.66f; }

    /**
     * @brief Get building manager
     */
    BuildingManager& getBuildingManager() { return m_buildingManager; }
    
    /**
     * @brief Get runway pointer (for BattleGround map)
     * @return Pointer to runway or nullptr if not present
     */
    Runway* getRunway() { return m_buildingManager.getRunway(); }

private:
    void createMap1();  // City map with roads, houses, river, etc.
    void addRoad(int x, int y, int width, int height, bool horizontal);
    void addHouse(int x, int y, int size);
    void addRiver(int x, int y, int width, int height, bool horizontal);
    void addBridge(int x, int y, bool horizontal);
    void addHospital(int x, int y);
    void addIndustrial(int x, int y, int width, int height);
    void addPowerline(int x1, int y1, int x2, int y2);

    int m_width;
    int m_height;
    int m_destroyedBuildings;
    std::vector<std::unique_ptr<MapObject>> m_objects;
    SDL_Color m_grassColor;
    BuildingManager m_buildingManager;
};

} // namespace BombingRun
