#pragma once

#include "entities/Building.h"
#include <SDL2/SDL.h>

namespace BombingRun {

/**
 * @brief Civilian building for City map
 * 
 * Residential and commercial structures with varied appearances.
 * Different sizes and colors for visual variety.
 */
class CivilianBuilding : public Building {
public:
    /**
     * @brief Building style/appearance
     */
    enum class Style {
        RESIDENTIAL,    // Houses and apartments
        COMMERCIAL,     // Shops and offices
        INDUSTRIAL,     // Factories and warehouses
        TOWER           // Tall buildings
    };

    /**
     * @brief Construct civilian building
     * @param x X position
     * @param y Y position
     * @param width Building width
     * @param height Building height
     * @param style Visual style
     */
    CivilianBuilding(float x, float y, float width, float height, Style style = Style::RESIDENTIAL);

    /**
     * @brief Render building with style-specific appearance
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer) override;

    /**
     * @brief Get building style
     * @return Building style
     */
    Style getStyle() const { return m_style; }

private:
    /**
     * @brief Get base color for style
     * @return SDL_Color based on style
     */
    SDL_Color getStyleColor() const;

    Style m_style;          // Visual style
};

} // namespace BombingRun
