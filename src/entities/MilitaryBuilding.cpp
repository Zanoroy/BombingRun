#include "entities/MilitaryBuilding.h"
#include "entities/Runway.h"
#include <algorithm>

namespace BombingRun {

MilitaryBuilding::MilitaryBuilding(float x, float y, float width, float height, 
                                   MilitaryType militaryType, Runway* runway)
    : Building(x, y, width, height, getMilitaryHealth(), Type::MILITARY)
    , m_militaryType(militaryType)
    , m_runway(runway)
    , m_hasNotifiedRunway(false)
{
    m_baseColor = getMilitaryColor();
}

void MilitaryBuilding::render(SDL_Renderer* renderer) {
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
    
    // Draw type-specific details
    switch (m_militaryType) {
        case MilitaryType::HANGAR: {
            // Draw large doors
            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
            SDL_Rect door = {
                static_cast<int>(m_x + m_width * 0.2f),
                static_cast<int>(m_y + m_height * 0.3f),
                static_cast<int>(m_width * 0.6f),
                static_cast<int>(m_height * 0.6f)
            };
            SDL_RenderFillRect(renderer, &door);
            break;
        }
            
        case MilitaryType::FUEL_TANK: {
            // Draw as circular tank
            SDL_SetRenderDrawColor(renderer, 
                static_cast<Uint8>(color.r * 0.8f),
                static_cast<Uint8>(color.g * 0.8f),
                static_cast<Uint8>(color.b * 0.8f),
                255);
            // Draw circle approximation
            for (int i = 0; i < 16; i++) {
                float angle = (i * 2.0f * 3.14159f) / 16.0f;
                float nextAngle = ((i + 1) * 2.0f * 3.14159f) / 16.0f;
                int x1 = static_cast<int>(m_x + m_width/2 + (m_width/2 - 5) * cos(angle));
                int y1 = static_cast<int>(m_y + m_height/2 + (m_height/2 - 5) * sin(angle));
                int x2 = static_cast<int>(m_x + m_width/2 + (m_width/2 - 5) * cos(nextAngle));
                int y2 = static_cast<int>(m_y + m_height/2 + (m_height/2 - 5) * sin(nextAngle));
                SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            }
            break;
        }
            
        case MilitaryType::RADAR_TOWER: {
            // Draw tall tower with antenna
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_Rect antenna = {
                static_cast<int>(m_x + m_width * 0.4f),
                static_cast<int>(m_y),
                static_cast<int>(m_width * 0.2f),
                static_cast<int>(m_height * 0.3f)
            };
            SDL_RenderFillRect(renderer, &antenna);
            break;
        }
            
        case MilitaryType::BARRACKS: {
            // Draw windows
            SDL_SetRenderDrawColor(renderer, 80, 80, 100, 255);
            for (int i = 0; i < 3; i++) {
                SDL_Rect window = {
                    static_cast<int>(m_x + m_width * 0.15f + i * m_width * 0.3f),
                    static_cast<int>(m_y + m_height * 0.3f),
                    static_cast<int>(m_width * 0.15f),
                    static_cast<int>(m_height * 0.25f)
                };
                SDL_RenderFillRect(renderer, &window);
            }
            break;
        }
            
        case MilitaryType::AMMO_DEPOT: {
            // Draw reinforced walls
            SDL_SetRenderDrawColor(renderer, 
                static_cast<Uint8>(color.r * 0.6f),
                static_cast<Uint8>(color.g * 0.6f),
                static_cast<Uint8>(color.b * 0.6f),
                255);
            SDL_RenderDrawRect(renderer, &rect);
            SDL_Rect innerRect = {
                static_cast<int>(m_x + 5),
                static_cast<int>(m_y + 5),
                static_cast<int>(m_width - 10),
                static_cast<int>(m_height - 10)
            };
            SDL_RenderDrawRect(renderer, &innerRect);
            break;
        }
    }
    
    // Draw darker border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

bool MilitaryBuilding::takeDamage(int damage) {
    bool wasDestroyed = Building::takeDamage(damage);
    
    // If destroyed and runway is linked, reduce runway max health
    if (wasDestroyed && m_runway && !m_hasNotifiedRunway) {
        m_runway->reduceMaxHealth(10);
        m_hasNotifiedRunway = true;
    }
    
    return wasDestroyed;
}

SDL_Color MilitaryBuilding::getMilitaryColor() const {
    switch (m_militaryType) {
        case MilitaryType::HANGAR:
            return SDL_Color{120, 120, 140, 255};  // Steel blue-gray
        case MilitaryType::FUEL_TANK:
            return SDL_Color{180, 180, 180, 255};  // Light gray/silver
        case MilitaryType::RADAR_TOWER:
            return SDL_Color{140, 140, 140, 255};  // Gray
        case MilitaryType::BARRACKS:
            return SDL_Color{160, 140, 120, 255};  // Tan
        case MilitaryType::AMMO_DEPOT:
            return SDL_Color{100, 100, 100, 255};  // Dark gray
        default:
            return SDL_Color{128, 128, 128, 255};  // Default gray
    }
}

int MilitaryBuilding::getMilitaryHealth() const {
    switch (m_militaryType) {
        case MilitaryType::HANGAR:
            return 5;  // Large but not heavily armored
        case MilitaryType::FUEL_TANK:
            return 3;  // Vulnerable to explosions
        case MilitaryType::RADAR_TOWER:
            return 4;  // Medium strength
        case MilitaryType::BARRACKS:
            return 4;  // Medium strength
        case MilitaryType::AMMO_DEPOT:
            return 6;  // Heavily reinforced
        default:
            return 3;
    }
}

} // namespace BombingRun
