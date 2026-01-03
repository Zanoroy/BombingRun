#include "systems/BuildingManager.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace BombingRun {

BuildingManager::BuildingManager() {
    // Seed random number generator
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        seeded = true;
    }
}

void BuildingManager::generateCityBuildings(int mapWidth, int mapHeight, int count) {
    clear();
    
    // Building size ranges
    const int minWidth = 30;
    const int maxWidth = 100;
    const int minHeight = 40;
    const int maxHeight = 120;
    
    // Margin from edges
    const int edgeMargin = 50;
    
    int attempts = 0;
    const int maxAttempts = count * 10; // Prevent infinite loop
    
    while (m_buildings.size() < static_cast<size_t>(count) && attempts < maxAttempts) {
        attempts++;
        
        // Random position
        float x = edgeMargin + (std::rand() % (mapWidth - 2 * edgeMargin));
        float y = edgeMargin + (std::rand() % (mapHeight - 2 * edgeMargin));
        
        // Random size
        float width = minWidth + (std::rand() % (maxWidth - minWidth));
        float height = minHeight + (std::rand() % (maxHeight - minHeight));
        
        // Check if position is valid (no overlap)
        if (isOverlapping(x, y, width, height, 20.0f)) {
            continue;
        }
        
        // Random style
        CivilianBuilding::Style style;
        int styleRoll = std::rand() % 100;
        
        if (styleRoll < 40) {
            style = CivilianBuilding::Style::RESIDENTIAL;
        } else if (styleRoll < 70) {
            style = CivilianBuilding::Style::COMMERCIAL;
        } else if (styleRoll < 90) {
            style = CivilianBuilding::Style::INDUSTRIAL;
        } else {
            style = CivilianBuilding::Style::TOWER;
            height = maxHeight + (std::rand() % 80); // Towers are taller
        }
        
        // Create building
        m_buildings.push_back(
            std::make_unique<CivilianBuilding>(x, y, width, height, style)
        );
    }
}

Runway* BuildingManager::generateMilitaryBase(int mapWidth, int mapHeight) {
    clear();
    
    // BattleGround map is 4x larger, so scale everything down by 4x
    const float SCALE = 0.25f;  // 4x smaller objects
    
    // Create massive military base runway (larger base dimensions)
    int runwayWidth = static_cast<int>(320 * SCALE);   // 80px final - massive runway
    int runwayHeight = static_cast<int>(700 * SCALE);  // 175px final - long military runway
    int runwayX = (mapWidth / 2) - (runwayWidth / 2);  // Center horizontally
    int runwayY = static_cast<int>(300 * SCALE);       // Scaled position from top
    
    m_runway = std::make_unique<Runway>(runwayX, runwayY, runwayWidth, runwayHeight);
    
    // Calculate fence dimensions (around entire massive airfield)
    int fenceMargin = static_cast<int>(80 * SCALE);
    int fenceLeft = runwayX - static_cast<int>(800 * SCALE);      // Much wider base
    int fenceRight = runwayX + runwayWidth + static_cast<int>(800 * SCALE);
    int fenceTop = runwayY - static_cast<int>(200 * SCALE);       // More space at top
    int fenceBottom = runwayY + runwayHeight + static_cast<int>(300 * SCALE);  // More space at bottom
    
    // Generate military buildings around the runway (massive base with 80 buildings)
    int hangarWidth = static_cast<int>(200 * SCALE);   // Larger hangars
    int hangarHeight = static_cast<int>(140 * SCALE);  // Taller hangars
    int hangarSpacing = static_cast<int>(90 * SCALE);  // More spacing
    
    // Left side hangars (8 hangars)
    for (int i = 0; i < 8; i++) {
        float x = runwayX - static_cast<int>(350 * SCALE);
        float y = runwayY + static_cast<int>(20 * SCALE) + i * (hangarHeight + hangarSpacing);
        m_buildings.push_back(
            std::make_unique<MilitaryBuilding>(
                x, y, hangarWidth, hangarHeight,
                MilitaryBuilding::MilitaryType::HANGAR,
                m_runway.get()
            )
        );
    }
    
    // Right side hangars (8 hangars)
    for (int i = 0; i < 8; i++) {
        float x = runwayX + runwayWidth + static_cast<int>(150 * SCALE);
        float y = runwayY + static_cast<int>(20 * SCALE) + i * (hangarHeight + hangarSpacing);
        m_buildings.push_back(
            std::make_unique<MilitaryBuilding>(
                x, y, hangarWidth, hangarHeight,
                MilitaryBuilding::MilitaryType::HANGAR,
                m_runway.get()
            )
        );
    }
    
    // Fuel tanks around the massive base
    int tankSize = static_cast<int>(60 * SCALE);       // Larger tanks
    int tankSpacing = static_cast<int>(40 * SCALE);    // More spacing
    
    // Left side fuel tanks - multiple columns (20 tanks)
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 10; row++) {
            float x = fenceLeft + static_cast<int>(60 * SCALE) + col * static_cast<int>(90 * SCALE);
            float y = runwayY + static_cast<int>(30 * SCALE) + row * (tankSize + tankSpacing);
            m_buildings.push_back(
                std::make_unique<MilitaryBuilding>(
                    x, y, tankSize, tankSize,
                    MilitaryBuilding::MilitaryType::FUEL_TANK,
                    m_runway.get()
                )
            );
        }
    }
    
    // Right side fuel tanks - multiple columns (20 tanks)    // Right side fuel tanks - multiple columns (20 tanks)
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 10; row++) {
            float x = fenceRight - static_cast<int>(150 * SCALE) - col * static_cast<int>(90 * SCALE);
            float y = runwayY + static_cast<int>(30 * SCALE) + row * (tankSize + tankSpacing);
            m_buildings.push_back(
                std::make_unique<MilitaryBuilding>(
                    x, y, tankSize, tankSize,
                    MilitaryBuilding::MilitaryType::FUEL_TANK,
                    m_runway.get()
                )
            );
        }
    }
    
    // Radar towers at corners (4 large towers)
    int radarWidth = static_cast<int>(80 * SCALE);
    int radarHeight = static_cast<int>(120 * SCALE);
    
    // Top left
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceLeft + static_cast<int>(30 * SCALE), fenceTop + static_cast<int>(10 * SCALE),
            radarWidth, radarHeight,
            MilitaryBuilding::MilitaryType::RADAR_TOWER,
            m_runway.get()
        )
    );
    
    // Top right
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceRight - radarWidth - static_cast<int>(30 * SCALE), fenceTop + static_cast<int>(10 * SCALE),
            radarWidth, radarHeight,
            MilitaryBuilding::MilitaryType::RADAR_TOWER,
            m_runway.get()
        )
    );
    
    // Bottom left
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceLeft + static_cast<int>(30 * SCALE), fenceBottom - radarHeight - static_cast<int>(10 * SCALE),
            radarWidth, radarHeight,
            MilitaryBuilding::MilitaryType::RADAR_TOWER,
            m_runway.get()
        )
    );
    
    // Bottom right
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceRight - radarWidth - static_cast<int>(30 * SCALE), fenceBottom - radarHeight - static_cast<int>(10 * SCALE),
            radarWidth, radarHeight,
            MilitaryBuilding::MilitaryType::RADAR_TOWER,
            m_runway.get()
        )
    );
    
    // Barracks (12 total - 6 on each side)
    int barracksWidth = static_cast<int>(140 * SCALE);
    int barracksHeight = static_cast<int>(80 * SCALE);
    
    // Left side barracks
    for (int i = 0; i < 6; i++) {
        m_buildings.push_back(
            std::make_unique<MilitaryBuilding>(
                fenceLeft + static_cast<int>(230 * SCALE), 
                fenceTop + static_cast<int>(150 * SCALE) + i * (barracksHeight + static_cast<int>(60 * SCALE)),
                barracksWidth, barracksHeight,
                MilitaryBuilding::MilitaryType::BARRACKS,
                m_runway.get()
            )
        );
    }
    
    // Right side barracks
    for (int i = 0; i < 6; i++) {
        m_buildings.push_back(
            std::make_unique<MilitaryBuilding>(
                fenceRight - barracksWidth - static_cast<int>(230 * SCALE), 
                fenceTop + static_cast<int>(150 * SCALE) + i * (barracksHeight + static_cast<int>(60 * SCALE)),
                barracksWidth, barracksHeight,
                MilitaryBuilding::MilitaryType::BARRACKS,
                m_runway.get()
            )
        );
    }
    
    // Ammo depots (4 total - reinforced, strategic positions)
    int depotWidth = static_cast<int>(100 * SCALE);
    int depotHeight = static_cast<int>(70 * SCALE);
    
    // Bottom left
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceLeft + static_cast<int>(250 * SCALE), 
            fenceBottom - depotHeight - static_cast<int>(30 * SCALE),
            depotWidth, depotHeight,
            MilitaryBuilding::MilitaryType::AMMO_DEPOT,
            m_runway.get()
        )
    );
    
    // Bottom right
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceRight - depotWidth - static_cast<int>(250 * SCALE), 
            fenceBottom - depotHeight - static_cast<int>(30 * SCALE),
            depotWidth, depotHeight,
            MilitaryBuilding::MilitaryType::AMMO_DEPOT,
            m_runway.get()
        )
    );
    
    // Top left
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceLeft + static_cast<int>(400 * SCALE), 
            fenceTop + static_cast<int>(30 * SCALE),
            depotWidth, depotHeight,
            MilitaryBuilding::MilitaryType::AMMO_DEPOT,
            m_runway.get()
        )
    );
    
    // Top right
    m_buildings.push_back(
        std::make_unique<MilitaryBuilding>(
            fenceRight - depotWidth - static_cast<int>(400 * SCALE), 
            fenceTop + static_cast<int>(30 * SCALE),
            depotWidth, depotHeight,
            MilitaryBuilding::MilitaryType::AMMO_DEPOT,
            m_runway.get()
        )
    );
    
    // Store fence coordinates for rendering
    m_fenceRect = {fenceLeft, fenceTop, fenceRight - fenceLeft, fenceBottom - fenceTop};
    m_hasFence = true;
    
    std::cout << "Generated military base with " << m_buildings.size() << " buildings around runway" << std::endl;
    
    return m_runway.get();
}

void BuildingManager::clear() {
    m_buildings.clear();
    m_runway.reset();
}

void BuildingManager::update(float deltaTime) {
    // Update runway if it exists
    if (m_runway) {
        m_runway->update(deltaTime);
    }
    
    // Update all buildings
    for (auto& building : m_buildings) {
        if (building) {
            building->update(deltaTime);
        }
    }
    
    // Remove destroyed buildings
    m_buildings.erase(
        std::remove_if(m_buildings.begin(), m_buildings.end(),
            [](const std::unique_ptr<Building>& b) {
                return !b || !b->isActive();
            }),
        m_buildings.end()
    );
}

void BuildingManager::render(SDL_Renderer* renderer) {
    // Render fence first (if present)
    if (m_hasFence) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Gray fence
        // Draw fence outline (3 pixel thick)
        for (int i = 0; i < 3; i++) {
            SDL_Rect fenceOutline = {
                m_fenceRect.x - i,
                m_fenceRect.y - i,
                m_fenceRect.w + 2*i,
                m_fenceRect.h + 2*i
            };
            SDL_RenderDrawRect(renderer, &fenceOutline);
        }
    }
    
    // Render runway (above fence, underneath buildings)
    if (m_runway) {
        m_runway->render(renderer);
    }
    
    // Render all buildings
    for (const auto& building : m_buildings) {
        if (building) {
            building->render(renderer);
        }
    }
}

int BuildingManager::checkCollision(float x, float y, float radius, int damage) {
    int damagedCount = 0;
    
    // Check runway collision first
    if (m_runway && m_runway->isActive()) {
        float runwayCenterX = m_runway->getX() + m_runway->getWidth() / 2.0f;
        float runwayCenterY = m_runway->getY() + m_runway->getHeight() / 2.0f;
        
        float dx = runwayCenterX - x;
        float dy = runwayCenterY - y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= radius) {
            m_runway->takeDamage(damage);
            damagedCount++;
        }
    }
    
    for (auto& building : m_buildings) {
        if (!building || !building->isActive()) {
            continue;
        }
        
        // Get building center
        float buildingCenterX = building->getX() + building->getWidth() / 2.0f;
        float buildingCenterY = building->getY() + building->getHeight() / 2.0f;
        
        // Calculate distance from explosion center to building center
        float dx = buildingCenterX - x;
        float dy = buildingCenterY - y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Check if within explosion radius
        if (distance <= radius) {
            if (building->takeDamage(damage)) {
                // Building was destroyed
                damagedCount++;
            } else {
                // Building was damaged but not destroyed
                damagedCount++;
            }
        }
    }
    
    return damagedCount;
}

int BuildingManager::getActiveCount() const {
    int count = 0;
    for (const auto& building : m_buildings) {
        if (building && building->isActive() && !building->isDestroyed()) {
            count++;
        }
    }
    return count;
}

int BuildingManager::getDestroyedCount() const {
    int count = 0;
    for (const auto& building : m_buildings) {
        if (building && building->isDestroyed()) {
            count++;
        }
    }
    return count;
}

bool BuildingManager::isOverlapping(float x, float y, float width, float height, float margin) const {
    SDL_Rect newRect{
        static_cast<int>(x - margin),
        static_cast<int>(y - margin),
        static_cast<int>(width + 2 * margin),
        static_cast<int>(height + 2 * margin)
    };
    
    for (const auto& building : m_buildings) {
        if (!building) continue;
        
        SDL_Rect existingRect{
            static_cast<int>(building->getX()),
            static_cast<int>(building->getY()),
            static_cast<int>(building->getWidth()),
            static_cast<int>(building->getHeight())
        };
        
        // Check for rectangle intersection
        if (SDL_HasIntersection(&newRect, &existingRect)) {
            return true;
        }
    }
    
    return false;
}

} // namespace BombingRun
