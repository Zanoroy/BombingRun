#include "systems/AircraftManager.h"
#include "systems/CollisionDetector.h"
#include "systems/WeaponManager.h"
#include "entities/Bomb.h"
#include <iostream>
#include <cmath>

namespace BombingRun {

AircraftManager::AircraftManager()
    : m_screenWidth(800)
    , m_screenHeight(600)
    , m_runwayX(0.0f)
    , m_runwayY(0.0f)
    , m_runwaySet(false)
    , m_randomEngine(std::random_device{}())
    , m_autoSpawnTimer(0.0f)
{
}

void AircraftManager::initialize(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    std::cout << "AircraftManager initialized for " << screenWidth << "x" << screenHeight << std::endl;
}

void AircraftManager::setRunwayPosition(float runwayX, float runwayY) {
    m_runwayX = runwayX;
    m_runwayY = runwayY;
    m_runwaySet = true;
    std::cout << "Runway position set to (" << runwayX << ", " << runwayY << ")" << std::endl;
}

bool AircraftManager::loadSprites() {
    bool bomberSprites = Bomber::loadSprites();
    bool fighterSprites = FighterJet::loadSprites();
    return bomberSprites && fighterSprites;
}

Bomber* AircraftManager::spawnBomber(float targetX, float targetY, int bombType) {
    // Clamp bomb type to valid range (0-6 for standard bombs, 8 for NUKE)
    if (bombType < 0) bombType = 0;
    if (bombType > 8) bombType = 8;
    if (bombType == 7) bombType = 7;  // Airstrike allowed

    // Spawn at bottom of screen, directly below target X position
    float spawnX = targetX;
    float spawnY = m_screenHeight + 50.0f;  // Below screen

    // If target not specified, use random position near top
    if (targetX < 0.0f) {
        spawnX = getRandomSpawnX();
        targetX = spawnX;  // Fly straight up
    }
    if (targetY < 0.0f) {
        targetY = 100.0f;  // Target near top of screen
    }

    // Get speed based on bomb type
    float speed = getSpeedForBombType(bombType);

    // Use single sprite (always 0)
    int spriteIndex = 0;

    // Create and add bomber
    auto bomber = std::make_unique<Bomber>(spawnX, spawnY, targetX, targetY, speed, bombType, spriteIndex);
    Bomber* bomberPtr = bomber.get();  // Get pointer before moving
    m_bombers.push_back(std::move(bomber));

    std::cout << "Spawned bomber at (" << spawnX << ", " << spawnY 
              << ") targeting (" << targetX << ", " << targetY 
              << ") with speed " << speed 
              << " bombType " << bombType
              << " sprite " << spriteIndex << std::endl;
    
    return bomberPtr;
}

void AircraftManager::deployAirstrike(float targetX, float targetY) {
    // If target not specified, use random position
    if (targetX < 0.0f) {
        targetX = getRandomTargetX();
    }
    if (targetY < 0.0f) {
        targetY = getGroundY();
    }

    std::cout << "Deploying airstrike at target (" << targetX << ", " << targetY << ")" << std::endl;

    // Airstrike uses 250lb bomb type (type 1) - each bomber carries 8 bombs
    const int bombType = 1;  // BOMB_250LB
    const float speed = getSpeedForBombType(bombType);
    
    // Deploy 5 bombers in triangle formation (no bottom)
    // Formation pattern (flipped - wide at front, point at back):
    //    4       5 (front row - wide part, aligned horizontally)
    //      2   3 (middle row, aligned horizontally)
    //        1 (back/point)
    const float spacing = 80.0f;  // Horizontal spacing between planes
    const float verticalSpacing = 100.0f;  // Vertical spacing between rows
    
    // Define positions for 5 planes in triangle pattern
    // Planes on the same row have identical Y coordinates for straight lines
    float positions[5][2] = {
        {targetX, targetY - verticalSpacing * 2},              // Plane 1: point at back
        {targetX - spacing, targetY - verticalSpacing},        // Plane 2: middle left
        {targetX + spacing, targetY - verticalSpacing},        // Plane 3: middle right (same Y as plane 2)
        {targetX - spacing * 2, targetY},                      // Plane 4: front left (wide part)
        {targetX + spacing * 2, targetY}                       // Plane 5: front right (same Y as plane 4)
    };
    
    // Spawn Y offsets for each row - planes on same row spawn at same height
    float spawnYOffsets[5] = {
        0.0f,   // Plane 1 (back point)
        30.0f,  // Plane 2 (middle left)
        30.0f,  // Plane 3 (middle right) - same as plane 2
        60.0f,  // Plane 4 (front left)
        60.0f   // Plane 5 (front right) - same as plane 4
    };
    
    for (int i = 0; i < 5; i++) {
        float targetPosX = positions[i][0];
        float targetPosY = positions[i][1];
        float spawnX = targetPosX;
        float spawnY = m_screenHeight + 50.0f + spawnYOffsets[i];  // Use row-aligned spawn positions
        
        // Create bomber with 250lb bombs
        auto bomber = std::make_unique<Bomber>(spawnX, spawnY, targetPosX, targetPosY, speed, bombType, 0);
        m_bombers.push_back(std::move(bomber));
        
        std::cout << "  Airstrike plane " << (i + 1) << " spawned at (" 
                  << spawnX << ", " << spawnY << ") targeting (" 
                  << targetPosX << ", " << targetPosY << ")" << std::endl;
    }
}

void AircraftManager::update(float deltaTime, WeaponManager* weaponManager) {
    // Update all bombers
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            bomber->update(deltaTime);
            
            // If bomber reached target and hasn't dropped bombs yet, drop all bombs!
            if (weaponManager && bomber->hasReachedTarget() && !bomber->hasBombsDropped()) {
                BombType bombType = static_cast<BombType>(bomber->getBombType());
                
                // Get number of bombs this type carries
                int bombCount = Bomb::getMaxBombs(bombType);
                
                // Get target radius for this bomb type
                // BattleGround map scale: 4x larger map = 4x smaller target radius
                int targetRadius = Bomb::getConfig(bombType).targetRadius;
                float scaledRadius = targetRadius * 0.25f;  // 4x smaller for BattleGround
                
                std::cout << "Bomber dropping " << bombCount << " bombs of type " << bomber->getBombType() << std::endl;
                
                // Drop all bombs in random pattern within target circle
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
                std::uniform_real_distribution<float> radiusDist(0.0f, scaledRadius);
                
                float bomberY = bomber->getY();  // Get bomber's current Y position
                
                for (int i = 0; i < bombCount; ++i) {
                    float angle = angleDist(m_randomEngine);
                    float radius = radiusDist(m_randomEngine);
                    
                    float offsetX = radius * cos(angle);
                    float offsetY = radius * sin(angle);
                    
                    float bombX = bomber->getTargetX() + offsetX;
                    float bombY = bomber->getTargetY() + offsetY;
                    
                    weaponManager->dropBomb(bombX, bombY, bombType, bomberY);
                }
                
                bomber->markBombsDropped();  // Mark as dropped
            }
        }
    }

    // Update all fighter jets
    for (auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            fighter->update(deltaTime);
            
            // Handle bullet firing - ALL bullets target the runway
            if (weaponManager && m_runwaySet) {
                // Attempt to fire (no distance restriction - can fire from anywhere on patrol)
                if (fighter->fire()) {
                    // Calculate bullet velocity toward runway center
                    float dx = m_runwayX - fighter->getX();
                    float dy = m_runwayY - fighter->getY();
                    float distance = std::sqrt(dx * dx + dy * dy);
                    
                    if (distance > 0.0f) {
                        // Normalize direction and apply bullet speed (800 px/s)
                        float bulletSpeed = 800.0f;
                        float velX = (dx / distance) * bulletSpeed;
                        float velY = (dy / distance) * bulletSpeed;
                        
                        // Pass fighter as owner to prevent self-collision
                        weaponManager->fireBullet(fighter->getX(), fighter->getY(), velX, velY, fighter.get());
                    }
                }
            }
        }
    }
    
    // Don't assign bomber targets - fighters just patrol and shoot runway
    // assignFighterTargets();
    
    // Check for automatic fighter spawning - DISABLED: fighters only spawn when F key is pressed
    // checkAutoSpawn();

    // Update AAA guns - they target both bombers and fighters
    std::vector<Bomber*> activeBombers;
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            activeBombers.push_back(bomber.get());
        }
    }
    std::vector<FighterJet*> activeFighters;
    for (auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            activeFighters.push_back(fighter.get());
        }
    }
    for (auto& gun : m_aaaGuns) {
        if (gun) {
            gun->updateTargeting(deltaTime, activeBombers, activeFighters, weaponManager);
        }
    }

    // Remove inactive bombers (destroyed or exited)
    m_bombers.erase(
        std::remove_if(m_bombers.begin(), m_bombers.end(),
            [](const std::unique_ptr<Bomber>& bomber) {
                return !bomber || !bomber->isActive();
            }),
        m_bombers.end()
    );
    
    // Remove inactive fighters
    m_fighters.erase(
        std::remove_if(m_fighters.begin(), m_fighters.end(),
            [](const std::unique_ptr<FighterJet>& fighter) {
                return !fighter || !fighter->isActive();
            }),
        m_fighters.end()
    );
}

void AircraftManager::render(SDL_Renderer* renderer) {
    // Render bombers
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            bomber->render(renderer);
        }
    }
    
    // Render fighters
    for (auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            fighter->render(renderer);
        }
    }
    
    // Render AAA guns
    for (auto& gun : m_aaaGuns) {
        if (gun) {
            gun->render(renderer);
        }
    }
}

int AircraftManager::getActiveBomberCount() const {
    int count = 0;
    for (const auto& bomber : m_bombers) {
        if (bomber->isActive()) {
            ++count;
        }
    }
    return count;
}

void AircraftManager::clearAll() {
    m_bombers.clear();
    m_fighters.clear();
    std::cout << "Cleared all aircraft" << std::endl;
}

void AircraftManager::destroyAllAircraft() {
    // Destroy all bombers
    for (auto& bomber : m_bombers) {
        if (bomber->isActive()) {
            bomber->destroy();
        }
    }
    
    // Destroy all fighter jets
    for (auto& fighter : m_fighters) {
        if (fighter->isActive()) {
            fighter->destroy();
        }
    }
    
    std::cout << "Nuclear blast destroyed " << m_bombers.size() << " bombers and " 
              << m_fighters.size() << " fighters" << std::endl;
}

void AircraftManager::spawnFighterJet() {
    if (!m_runwaySet) {
        std::cerr << "Cannot spawn fighter - runway position not set!" << std::endl;
        return;
    }
    
    // Always spawn from bottom of map, centered on runway X position
    float spawnX = m_runwayX;
    float spawnY = m_screenHeight + 50.0f;  // Below screen
    
    auto fighter = std::make_unique<FighterJet>(spawnX, spawnY, m_runwayX, m_runwayY);
    m_fighters.push_back(std::move(fighter));
    
    std::cout << "Fighter jet spawned at (" << spawnX << ", " << spawnY << ") targeting runway at (" << m_runwayX << ", " << m_runwayY << ")" << std::endl;
}

int AircraftManager::getActiveFighterCount() const {
    int count = 0;
    for (const auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            ++count;
        }
    }
    return count;
}

std::vector<Bomber*> AircraftManager::getActiveBombers() {
    std::vector<Bomber*> bombers;
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            bombers.push_back(bomber.get());
        }
    }
    return bombers;
}

std::vector<FighterJet*> AircraftManager::getActiveFighters() {
    std::vector<FighterJet*> fighters;
    for (auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            fighters.push_back(fighter.get());
        }
    }
    return fighters;
}

Bomber* AircraftManager::checkBomberCollision(float x, float y) {
    for (auto& bomber : m_bombers) {
        if (bomber && bomber->isActive()) {
            SDL_Rect bounds = bomber->getBounds();
            if (CollisionDetector::checkPointRect(x, y, bounds)) {
                return bomber.get();
            }
        }
    }
    return nullptr;
}

FighterJet* AircraftManager::checkFighterCollision(float x, float y) {
    for (auto& fighter : m_fighters) {
        if (fighter && fighter->isActive()) {
            SDL_Rect bounds = fighter->getBounds();
            if (CollisionDetector::checkPointRect(x, y, bounds)) {
                return fighter.get();
            }
        }
    }
    return nullptr;
}

void AircraftManager::assignFighterTargets() {
    // Get all active bombers
    auto bombers = getActiveBombers();
    if (bombers.empty()) {
        return;
    }
    
    // Assign targets to fighters that don't have one
    for (auto& fighter : m_fighters) {
        if (!fighter || !fighter->isActive()) {
            continue;
        }
        
        // If fighter doesn't have a target or target is inactive, find a new one
        if (!fighter->getTarget() || !fighter->getTarget()->isActive()) {
            // Find closest bomber
            Bomber* closestBomber = nullptr;
            float closestDist = std::numeric_limits<float>::max();
            
            for (auto* bomber : bombers) {
                float dx = bomber->getX() - fighter->getX();
                float dy = bomber->getY() - fighter->getY();
                float dist = dx * dx + dy * dy;  // Squared distance (no need for sqrt)
                
                if (dist < closestDist) {
                    closestDist = dist;
                    closestBomber = bomber;
                }
            }
            
            if (closestBomber) {
                fighter->setTarget(closestBomber);
            }
        }
    }
}

float AircraftManager::getSpeedForBombType(int bombType) const {
    // Speed affects how fast bomber moves (pixels per frame at 60 FPS)
    // Multiply by 60 to convert to pixels per second
    return BOMB_SPEEDS[bombType] * 60.0f;
}

float AircraftManager::getRandomSpawnX() const {
    std::uniform_real_distribution<float> dist(100.0f, m_screenWidth - 100.0f);
    return dist(const_cast<std::mt19937&>(m_randomEngine));
}

float AircraftManager::getRandomTargetX() const {
    std::uniform_real_distribution<float> dist(50.0f, m_screenWidth - 50.0f);
    return dist(const_cast<std::mt19937&>(m_randomEngine));
}

float AircraftManager::getGroundY() const {
    // Target is at 2/3 down the screen (ground level)
    return m_screenHeight * 0.66f;
}

void AircraftManager::checkAutoSpawn() {
    // Automatically spawn fighters when bombers are detected
    int bomberCount = getActiveBomberCount();
    int fighterCount = getActiveFighterCount();
    
    // Spawn a fighter if:
    // - There are bombers incoming
    // - We have fewer fighters than bombers (up to max 3 fighters)
    // - Timer has elapsed (to prevent constant spawning)
    if (bomberCount > 0 && fighterCount < 3 && fighterCount < bomberCount) {
        if (m_autoSpawnTimer <= 0.0f) {
            spawnFighterJet();
            m_autoSpawnTimer = 5.0f;  // 5 second cooldown between auto-spawns
            std::cout << "Auto-spawned fighter jet (Bombers: " << bomberCount 
                      << ", Fighters: " << (fighterCount + 1) << ")" << std::endl;
        }
    }
    
    // Decrease timer
    if (m_autoSpawnTimer > 0.0f) {
        m_autoSpawnTimer -= 0.016f;  // Assuming ~60 FPS
    }
}

void AircraftManager::initializeAAAGuns() {
    if (!m_runwaySet) {
        std::cerr << "Cannot initialize AAA guns - runway position not set!" << std::endl;
        return;
    }

    m_aaaGuns.clear();

    // Position 4 AAA guns around the runway in a defensive formation
    // Guns positioned at corners around runway area
    const float offsetX = 200.0f;  // Distance from runway center horizontally
    const float offsetY = 100.0f;  // Distance from runway center vertically

    // Gun 1: Top-left of runway
    m_aaaGuns.push_back(std::make_unique<AAAGun>(m_runwayX - offsetX, m_runwayY - offsetY));

    // Gun 2: Top-right of runway
    m_aaaGuns.push_back(std::make_unique<AAAGun>(m_runwayX + offsetX, m_runwayY - offsetY));

    // Gun 3: Bottom-left of runway
    m_aaaGuns.push_back(std::make_unique<AAAGun>(m_runwayX - offsetX, m_runwayY + offsetY));

    // Gun 4: Bottom-right of runway
    m_aaaGuns.push_back(std::make_unique<AAAGun>(m_runwayX + offsetX, m_runwayY + offsetY));

    std::cout << "Initialized 4 AAA guns around runway at (" << m_runwayX << ", " << m_runwayY << ")" << std::endl;
}

std::vector<AAAGun*> AircraftManager::getAAAGuns() {
    std::vector<AAAGun*> guns;
    for (auto& gun : m_aaaGuns) {
        if (gun) {
            guns.push_back(gun.get());
        }
    }
    return guns;
}

} // namespace BombingRun
