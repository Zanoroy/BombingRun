#include "maps/Map.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace BombingRun {

Map::Map(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_destroyedBuildings(0)
    , m_grassColor{34, 139, 34, 255}  // Forest green
{
}

bool Map::loadMap(const std::string& mapName) {
    std::cout << "Loading map: " << mapName << std::endl;
    
    m_objects.clear();
    
    if (mapName == "map1") {
        createMap1();
        return true;
    }
    
    std::cerr << "Unknown map: " << mapName << std::endl;
    return false;
}

void Map::render(SDL_Renderer* renderer) {
    // Draw grass background
    SDL_SetRenderDrawColor(renderer, m_grassColor.r, m_grassColor.g, m_grassColor.b, 255);
    SDL_Rect grassRect = {0, 0, m_width, m_height};
    SDL_RenderFillRect(renderer, &grassRect);
    
    // Render all map objects
    for (const auto& obj : m_objects) {
        // Skip destroyed buildings
        if (obj->destructible && obj->health <= 0) {
            continue;
        }
        
        // Apply damage color tint to buildings
        SDL_Color renderColor = obj->color;
        if (obj->destructible && obj->health > 0) {
            // Get initial health based on building type
            int maxHealth = 10; // Default
            if (obj->type == StructureType::HOUSE_SMALL) maxHealth = 10;
            else if (obj->type == StructureType::HOUSE_MEDIUM) maxHealth = 20;
            else if (obj->type == StructureType::HOUSE_LARGE) maxHealth = 30;
            else if (obj->type == StructureType::HOSPITAL) maxHealth = 100;
            else if (obj->type == StructureType::INDUSTRIAL) maxHealth = 50;
            
            float healthPercent = static_cast<float>(obj->health) / maxHealth;
            
            // Tint red as health decreases
            if (healthPercent < 0.75f) {
                int redBoost = static_cast<int>((1.0f - healthPercent) * 100);
                renderColor.r = std::min(255, renderColor.r + redBoost);
                renderColor.g = std::max(0, renderColor.g - redBoost / 2);
                renderColor.b = std::max(0, renderColor.b - redBoost / 2);
            }
        }
        
        SDL_SetRenderDrawColor(renderer, renderColor.r, renderColor.g, renderColor.b, renderColor.a);
        SDL_RenderFillRect(renderer, &obj->bounds);
        
        // Draw border for buildings
        if (obj->type == StructureType::HOUSE_SMALL ||
            obj->type == StructureType::HOUSE_MEDIUM ||
            obj->type == StructureType::HOUSE_LARGE ||
            obj->type == StructureType::HOSPITAL ||
            obj->type == StructureType::INDUSTRIAL) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &obj->bounds);
        }
    }
}

MapObject* Map::checkCollision(float x, float y) {
    for (auto& obj : m_objects) {
        if (obj->destructible) {
            if (x >= obj->bounds.x && x <= obj->bounds.x + obj->bounds.w &&
                y >= obj->bounds.y && y <= obj->bounds.y + obj->bounds.h) {
                return obj.get();
            }
        }
    }
    return nullptr;
}

int Map::damageBuildings(float x, float y, float radius, int damage) {
    int destroyed = 0;
    
    for (auto& obj : m_objects) {
        // Only damage destructible buildings that aren't already destroyed
        if (!obj->destructible || obj->health <= 0) {
            continue;
        }
        
        // Calculate distance from explosion center to building center
        float buildingCenterX = obj->bounds.x + obj->bounds.w / 2.0f;
        float buildingCenterY = obj->bounds.y + obj->bounds.h / 2.0f;
        
        float dx = buildingCenterX - x;
        float dy = buildingCenterY - y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Check if building is within explosion radius
        if (distance < radius) {
            // Apply damage (reduced by distance)
            float damageMultiplier = 1.0f - (distance / radius);
            int actualDamage = static_cast<int>(damage * damageMultiplier);
            
            obj->health -= actualDamage;
            
            if (obj->health <= 0) {
                obj->health = 0;
                destroyed++;
                m_destroyedBuildings++;
                std::cout << "Building destroyed at (" << obj->bounds.x << ", " << obj->bounds.y << ")" << std::endl;
            } else {
                std::cout << "Building damaged: " << obj->health << " HP remaining" << std::endl;
            }
        }
    }
    
    return destroyed;
}

void Map::createMap1() {
    std::cout << "Creating City Map (" << m_width << "x" << m_height << ")" << std::endl;
    
    // Main horizontal roads across screen
    addRoad(0, m_height * 0.3, m_width, 50, true);  // Upper road
    addRoad(0, m_height * 0.6, m_width, 50, true);  // Lower road
    
    // Vertical roads - more evenly spaced across full width
    int roadSpacing = m_width / 6;
    for (int i = 1; i < 6; ++i) {
        addRoad(i * roadSpacing, 0, 50, m_height, false);
    }
    
    // River running diagonally through map
    addRiver(0, m_height * 0.15, m_width * 0.4, 40, true);
    addRiver(m_width * 0.4, m_height * 0.15, 40, m_height * 0.4, false);
    addRiver(m_width * 0.4, m_height * 0.55, m_width * 0.6, 40, true);
    
    // Bridges over river
    for (int i = 1; i < 6; ++i) {
        int roadX = i * roadSpacing;
        if (roadX < m_width * 0.4 && roadX > m_width * 0.1) {
            addBridge(roadX, m_height * 0.15, false);
        }
        if (roadX > m_width * 0.4 && roadX < m_width * 0.9) {
            addBridge(roadX, m_height * 0.55, false);
        }
    }
    
    // Section 1: Left side houses (before first road)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            addHouse(30 + i * 65, 30 + j * 80, 1);
        }
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            addHouse(30 + i * 65, m_height * 0.35 + j * 80, 1);
        }
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            addHouse(30 + i * 65, m_height * 0.65 + j * 90, 2);
        }
    }
    
    // Section 2-5: Between vertical roads
    for (int section = 1; section < 6; ++section) {
        int sectionX = section * roadSpacing + 60;
        int sectionWidth = roadSpacing - 120;
        
        // Upper area houses
        int housesPerRow = sectionWidth / 70;
        for (int i = 0; i < housesPerRow; ++i) {
            for (int j = 0; j < 2; ++j) {
                if (section == 2 && i == 0 && j == 0) continue; // Reserve for hospital
                addHouse(sectionX + i * 70, 30 + j * 80, (section + i) % 3 + 1);
            }
        }
        
        // Middle area (between roads)
        for (int i = 0; i < housesPerRow; ++i) {
            for (int j = 0; j < 2; ++j) {
                addHouse(sectionX + i * 70, m_height * 0.35 + j * 80, (i + j) % 2 + 1);
            }
        }
        
        // Lower area
        for (int i = 0; i < housesPerRow; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (section >= 3 && section <= 4 && j < 2) {
                    // Industrial area
                    if (i == 0 && j == 0) {
                        addIndustrial(sectionX, m_height * 0.65, sectionWidth / 2, 120);
                    }
                } else {
                    addHouse(sectionX + i * 70, m_height * 0.65 + j * 95, (i * j) % 2 + 1);
                }
            }
        }
    }
    
    // Add hospitals in sections 2 and 5
    addHospital(2 * roadSpacing + 60, 30);
    addHospital(5 * roadSpacing + 60, m_height * 0.65);
    
    // Add industrial complexes in sections 3 and 4
    addIndustrial(3 * roadSpacing + 60, 30, 250, 150);
    addIndustrial(4 * roadSpacing + 60, 30, 180, 120);
    addIndustrial(4 * roadSpacing + 250, 30, 150, 100);
    
    // Powerlines running along top and bottom
    addPowerline(50, 20, m_width - 50, 20);
    addPowerline(50, m_height - 20, m_width - 50, m_height - 20);
    
    // Powerline towers along top
    for (int i = 0; i < 10; ++i) {
        int towerX = 50 + i * ((m_width - 100) / 9);
        m_objects.push_back(std::make_unique<MapObject>(
            StructureType::POWERLINE_TOWER, towerX, 15, 8, 10,
            SDL_Color{80, 80, 80, 255}, false, 0
        ));
        m_objects.push_back(std::make_unique<MapObject>(
            StructureType::POWERLINE_TOWER, towerX, m_height - 25, 8, 10,
            SDL_Color{80, 80, 80, 255}, false, 0
        ));
    }
    
    std::cout << "Map created with " << m_objects.size() << " objects" << std::endl;
}

void Map::addRoad(int x, int y, int width, int height, bool horizontal) {
    m_objects.push_back(std::make_unique<MapObject>(
        horizontal ? StructureType::ROAD_HORIZONTAL : StructureType::ROAD_VERTICAL,
        x, y, width, height,
        SDL_Color{60, 60, 60, 255},  // Dark gray
        false, 0
    ));
    
    // Add road markings
    if (horizontal && width > 100) {
        for (int i = x; i < x + width; i += 40) {
            m_objects.push_back(std::make_unique<MapObject>(
                StructureType::ROAD_HORIZONTAL,
                i, y + height/2 - 1, 20, 2,
                SDL_Color{255, 255, 0, 255},  // Yellow line
                false, 0
            ));
        }
    } else if (!horizontal && height > 100) {
        for (int i = y; i < y + height; i += 40) {
            m_objects.push_back(std::make_unique<MapObject>(
                StructureType::ROAD_VERTICAL,
                x + width/2 - 1, i, 2, 20,
                SDL_Color{255, 255, 0, 255},  // Yellow line
                false, 0
            ));
        }
    }
}

void Map::addHouse(int x, int y, int size) {
    int width = 40 + size * 10;
    int height = 35 + size * 10;
    
    // Vary house colors
    SDL_Color colors[] = {
        {200, 150, 100, 255},  // Tan
        {180, 140, 120, 255},  // Brown
        {220, 200, 180, 255},  // Beige
        {160, 120, 100, 255}   // Dark brown
    };
    
    int colorIndex = (x + y) % 4;
    
    m_objects.push_back(std::make_unique<MapObject>(
        size == 1 ? StructureType::HOUSE_SMALL : 
        size == 2 ? StructureType::HOUSE_MEDIUM : StructureType::HOUSE_LARGE,
        x, y, width, height,
        colors[colorIndex],
        true, 10 * size  // Health based on size
    ));
}

void Map::addRiver(int x, int y, int width, int height, bool horizontal) {
    m_objects.push_back(std::make_unique<MapObject>(
        horizontal ? StructureType::RIVER_HORIZONTAL : StructureType::RIVER_VERTICAL,
        x, y, width, height,
        SDL_Color{70, 130, 180, 255},  // Steel blue
        false, 0
    ));
}

void Map::addBridge(int x, int y, bool horizontal) {
    if (horizontal) {
        m_objects.push_back(std::make_unique<MapObject>(
            StructureType::BRIDGE_HORIZONTAL,
            x - 20, y, 80, 30,
            SDL_Color{139, 90, 43, 255},  // Brown
            false, 0
        ));
    } else {
        m_objects.push_back(std::make_unique<MapObject>(
            StructureType::BRIDGE_VERTICAL,
            x, y - 20, 40, 80,
            SDL_Color{139, 90, 43, 255},  // Brown
            false, 0
        ));
    }
}

void Map::addHospital(int x, int y) {
    // Main building
    m_objects.push_back(std::make_unique<MapObject>(
        StructureType::HOSPITAL,
        x, y, 120, 90,
        SDL_Color{255, 255, 255, 255},  // White
        true, 100  // High health
    ));
    
    // Red cross
    m_objects.push_back(std::make_unique<MapObject>(
        StructureType::HOSPITAL,
        x + 50, y + 25, 20, 40,
        SDL_Color{255, 0, 0, 255},  // Red
        false, 0
    ));
    m_objects.push_back(std::make_unique<MapObject>(
        StructureType::HOSPITAL,
        x + 40, y + 35, 40, 20,
        SDL_Color{255, 0, 0, 255},  // Red
        false, 0
    ));
}

void Map::addIndustrial(int x, int y, int width, int height) {
    m_objects.push_back(std::make_unique<MapObject>(
        StructureType::INDUSTRIAL,
        x, y, width, height,
        SDL_Color{100, 100, 120, 255},  // Gray-blue
        true, 50
    ));
}

void Map::addPowerline(int x1, int y1, int x2, int y2) {
    m_objects.push_back(std::make_unique<MapObject>(
        StructureType::POWERLINE_HORIZONTAL,
        x1, y1, x2 - x1, 2,
        SDL_Color{40, 40, 40, 255},  // Dark gray
        false, 0
    ));
}

} // namespace BombingRun
