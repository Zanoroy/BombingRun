#include "entities/CivilianBuilding.h"
#include <algorithm>

namespace BombingRun {

CivilianBuilding::CivilianBuilding(float x, float y, float width, float height, Style style)
    : Building(x, y, width, height, 3, Type::CIVILIAN) // 3 HP for civilian buildings
    , m_style(style)
{
    m_baseColor = getStyleColor();
}

void CivilianBuilding::render(SDL_Renderer* renderer) {
    if (!m_active || m_health <= 0) {
        return;
    }

    // Get damage-based color
    SDL_Color color = getDamageColor();
    
    // Set render color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Draw filled rectangle for building
    SDL_Rect rect = getBounds();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw darker border
    SDL_SetRenderDrawColor(renderer, 
        static_cast<Uint8>(color.r * 0.5f),
        static_cast<Uint8>(color.g * 0.5f),
        static_cast<Uint8>(color.b * 0.5f),
        255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Add windows for visual detail
    if (m_style == Style::TOWER || m_style == Style::COMMERCIAL) {
        SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255); // Dark blue windows
        
        int windowSize = static_cast<int>(std::min(m_width, m_height) * 0.15f);
        int spacing = windowSize + 5;
        
        for (int wx = static_cast<int>(m_x) + spacing; 
             wx < m_x + m_width - windowSize; 
             wx += spacing) {
            for (int wy = static_cast<int>(m_y) + spacing; 
                 wy < m_y + m_height - windowSize; 
                 wy += spacing) {
                SDL_Rect window{wx, wy, windowSize, windowSize};
                SDL_RenderFillRect(renderer, &window);
            }
        }
    }
}

SDL_Color CivilianBuilding::getStyleColor() const {
    switch (m_style) {
        case Style::RESIDENTIAL:
            return SDL_Color{180, 140, 100, 255}; // Tan/brown
        case Style::COMMERCIAL:
            return SDL_Color{160, 160, 180, 255}; // Light gray/blue
        case Style::INDUSTRIAL:
            return SDL_Color{120, 120, 120, 255}; // Dark gray
        case Style::TOWER:
            return SDL_Color{140, 150, 160, 255}; // Steel gray
        default:
            return SDL_Color{128, 128, 128, 255}; // Default gray
    }
}

} // namespace BombingRun
