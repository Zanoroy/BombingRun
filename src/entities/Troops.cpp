#include "entities/Troops.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

namespace BombingRun {

static std::random_device rd;
static std::mt19937 gen(rd());

Troops::Troops(float x, float y, bool isEnemy)
    : GameObject(x, y)
    , m_destX(x)
    , m_destY(y)
    , m_hasDestination(false)
    , m_speed(50.0f)  // pixels per second
    , m_distanceTraveled(0.0f)
    , m_target(nullptr)
    , m_inCombat(false)
    , m_selected(false)
    , m_isResting(false)
    , m_restTimer(0.0f)
    , m_isEnemy(isEnemy)
{
    formationSetup();
}

void Troops::formationSetup() {
    m_soldiers.clear();
    
    // Create 2 rows of 5 soldiers each with random variation
    std::uniform_real_distribution<float> posVariation(-3.0f, 3.0f);
    
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            float offsetX = (col - 2.0f) * SOLDIER_SPACING + posVariation(gen);  // Add randomness
            float offsetY = row * ROW_SPACING + posVariation(gen);  // Add randomness
            m_soldiers.emplace_back(offsetX, offsetY);
            
            // Add random variation to shoot cooldown so they don't all fire at once
            std::uniform_real_distribution<float> dist(0.0f, 0.3f);
            m_soldiers.back().shootCooldown = dist(gen);
        }
    }
}

void Troops::update(float deltaTime) {
    if (!isActive()) return;
    
    // Update soldier cooldowns FIRST (always)
    for (auto& soldier : m_soldiers) {
        if (!soldier.isAlive) continue;
        
        if (soldier.shootCooldown > 0.0f) {
            soldier.shootCooldown -= deltaTime;
        }
    }
    
    // Update rest timer
    if (m_isResting) {
        updateResting(deltaTime);
        return;
    }
    
    // Update combat if engaged
    if (m_inCombat) {
        updateCombat(deltaTime);
        updateSoldierShooting(deltaTime);
        return;
    }
    
    // Update movement if not in combat
    if (m_hasDestination && !m_inCombat) {
        updateMovement(deltaTime);
    }
    
    // Check if all soldiers dead
    if (getAliveCount() == 0) {
        destroy();
    }
}

void Troops::updateMovement(float deltaTime) {
    float dx = m_destX - m_x;
    float dy = m_destY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < 5.0f) {
        // Reached destination
        m_hasDestination = false;
        m_distanceTraveled = 0.0f;
        return;
    }
    
    // Normalize and move
    float moveDistance = m_speed * deltaTime;
    if (moveDistance > distance) {
        moveDistance = distance;
    }
    
    m_x += (dx / distance) * moveDistance;
    m_y += (dy / distance) * moveDistance;
    m_distanceTraveled += moveDistance;
    
    // Check if rest is needed
    checkRestNeeded();
}

void Troops::updateCombat(float deltaTime) {
    if (!m_target || !m_target->isActive()) {
        // Target lost, disengage
        disengageTarget();
        return;
    }
    
    float dist = getDistanceTo(m_target);
    
    // If target out of range, try to move closer (but stay in combat)
    if (dist > FIRING_RANGE * 1.5f) {
        // Too far, disengage
        disengageTarget();
    }
}

void Troops::updateSoldierShooting(float deltaTime) {
    if (!m_target || !m_target->isActive()) return;
    
    // Get all alive enemy soldier positions
    std::vector<std::pair<float, float>> targetPositions;
    m_target->getSoldierPositions(targetPositions);
    
    if (targetPositions.empty()) return;
    
    // Each alive soldier shoots continuously
    for (auto& soldier : m_soldiers) {
        if (!soldier.isAlive) continue;
        
        // Skip if on cooldown
        if (soldier.shootCooldown > 0.0f) continue;
        
        // Calculate soldier world position
        float soldierX = m_x + soldier.offsetX;
        float soldierY = m_y + soldier.offsetY;
        
        // Pick closest target
        float closestDist = 999999.0f;
        int closestTarget = 0;
        for (size_t i = 0; i < targetPositions.size(); i++) {
            float dx = targetPositions[i].first - soldierX;
            float dy = targetPositions[i].second - soldierY;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < closestDist) {
                closestDist = dist;
                closestTarget = i;
            }
        }
        
        auto targetPos = targetPositions[closestTarget];
        float dx = targetPos.first - soldierX;
        float dy = targetPos.second - soldierY;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        // Fire if in range
        if (dist <= FIRING_RANGE) {
            // Calculate angle to target
            float angle = std::atan2(dy, dx);
            
            // Add slight inaccuracy
            std::uniform_real_distribution<float> accuracyVar(-0.1f, 0.1f);
            angle += accuracyVar(gen);
            
            // Create and fire bullet
            auto bullet = std::make_shared<Bullet>(soldierX, soldierY, angle, 400.0f, nullptr, 3.0f);
            bullet->setColor(0, 0, 0);
            m_bulletsFired.push_back(bullet);
            
            // Set random cooldown between shots (0.2-0.6 seconds)
            std::uniform_real_distribution<float> cooldownVar(0.2f, 0.6f);
            soldier.shootCooldown = cooldownVar(gen);
        }
    }
}

void Troops::updateResting(float deltaTime) {
    m_restTimer += deltaTime;
    if (m_restTimer >= REST_DURATION) {
        m_isResting = false;
        m_restTimer = 0.0f;
        m_distanceTraveled = 0.0f;
    }
}

void Troops::checkRestNeeded() {
    if (m_distanceTraveled >= REST_DISTANCE) {
        m_isResting = true;
        m_restTimer = 0.0f;
        m_hasDestination = false;  // Stop moving
    }
}

void Troops::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!isActive()) return;
    
    int aliveCount = getAliveCount();
    if (aliveCount == 0) {
        // No alive soldiers, mark group for removal
        destroy();
        return;
    }
    
    // Render each alive soldier
    for (const auto& soldier : m_soldiers) {
        if (!soldier.isAlive) continue;  // Skip dead soldiers
        
        float soldierX = m_x + soldier.offsetX;
        float soldierY = m_y + soldier.offsetY;
        
        // Determine color based on HP and team
        SDL_Color color;
        if (m_isEnemy) {
            // Enemy troops (red tones)
            if (soldier.hp >= 10) {
                color = {180, 0, 0, 255};  // Dark red (full health)
            } else if (soldier.hp >= 5) {
                color = {220, 50, 50, 255};  // Red (medium health)
            } else {
                color = {255, 100, 100, 255};  // Light red (low health)
            }
        } else {
            // Friendly troops (blue tones)
            if (soldier.hp >= 10) {
                color = {0, 100, 180, 255};  // Dark blue (full health)
            } else if (soldier.hp >= 5) {
                color = {50, 150, 220, 255};  // Blue (medium health)
            } else {
                color = {100, 180, 255, 255};  // Light blue (low health)
            }
        }
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        
        // Draw soldier as small circle
        int radius = 4;
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, soldierX + dx, soldierY + dy);
                }
            }
        }
        
        // Show reload indicator (small gray square)
        if (soldier.isReloading) {
            SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
            SDL_Rect reloadRect = {
                static_cast<int>(soldierX - 2),
                static_cast<int>(soldierY - 8),
                4, 2
            };
            SDL_RenderFillRect(renderer, &reloadRect);
        }
    }
    
    // Draw selection indicator
    if (m_selected) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow
        SDL_Rect selectionRect = {
            static_cast<int>(m_x - 40),
            static_cast<int>(m_y - 20),
            80, 50
        };
        SDL_RenderDrawRect(renderer, &selectionRect);
    }
    
    // Draw destination marker if moving
    if (m_hasDestination && !m_inCombat) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 128);  // Green semi-transparent
        for (int i = 0; i < 8; i++) {
            float angle = (i / 8.0f) * 2.0f * M_PI;
            int x1 = m_destX + std::cos(angle) * 10;
            int y1 = m_destY + std::sin(angle) * 10;
            int x2 = m_destX + std::cos(angle) * 5;
            int y2 = m_destY + std::sin(angle) * 5;
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    // Draw rest indicator
    if (m_isResting) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect restRect = {
            static_cast<int>(m_x - 15),
            static_cast<int>(m_y - 30),
            30, 3
        };
        SDL_RenderFillRect(renderer, &restRect);
        
        // Progress bar
        float progress = m_restTimer / REST_DURATION;
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
        SDL_Rect progressRect = {
            static_cast<int>(m_x - 15),
            static_cast<int>(m_y - 30),
            static_cast<int>(30 * progress), 3
        };
        SDL_RenderFillRect(renderer, &progressRect);
    }
    
    // Draw alive count for debugging
    if (m_inCombat) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        // Draw small indicator that this group is in combat
        SDL_Rect combatRect = {
            static_cast<int>(m_x - 5),
            static_cast<int>(m_y - 35),
            10, 2
        };
        SDL_RenderFillRect(renderer, &combatRect);
    }
}

void Troops::setDestination(float x, float y) {
    if (m_inCombat) return;  // Cannot move during combat
    
    m_destX = x;
    m_destY = y;
    m_hasDestination = true;
}

void Troops::select() {
    m_selected = true;
}

void Troops::deselect() {
    m_selected = false;
}

void Troops::engageTarget(Troops* target) {
    if (!target || !target->isActive()) return;
    
    m_target = target;
    m_inCombat = true;
    m_hasDestination = false;  // Cancel movement orders
}

void Troops::disengageTarget() {
    m_target = nullptr;
    m_inCombat = false;
}

int Troops::getAliveCount() const {
    int count = 0;
    for (const auto& soldier : m_soldiers) {
        if (soldier.isAlive) count++;
    }
    return count;
}

bool Troops::containsPoint(float px, float py) const {
    // Check if point is within group bounds
    float left = m_x - 40;
    float right = m_x + 40;
    float top = m_y - 20;
    float bottom = m_y + 40;
    
    return px >= left && px <= right && py >= top && py <= bottom;
}

float Troops::getDistanceTo(const Troops* other) const {
    if (!other) return 999999.0f;
    
    float dx = other->m_x - m_x;
    float dy = other->m_y - m_y;
    return std::sqrt(dx * dx + dy * dy);
}

void Troops::getSoldierPositions(std::vector<std::pair<float, float>>& positions) const {
    positions.clear();
    for (const auto& soldier : m_soldiers) {
        if (soldier.isAlive) {
            positions.emplace_back(m_x + soldier.offsetX, m_y + soldier.offsetY);
        }
    }
}

void Troops::damageSoldierAt(float x, float y, int damage) {
    // Find closest alive soldier to the hit position
    float closestDist = 999999.0f;
    Soldier* closestSoldier = nullptr;
    
    for (auto& soldier : m_soldiers) {
        if (!soldier.isAlive) continue;
        
        float soldierX = m_x + soldier.offsetX;
        float soldierY = m_y + soldier.offsetY;
        float dx = soldierX - x;
        float dy = soldierY - y;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist < closestDist && dist < 5.0f) {  // Smaller hit radius
            closestDist = dist;
            closestSoldier = &soldier;
        }
    }
    
    if (closestSoldier) {
        closestSoldier->hp -= damage;
        if (closestSoldier->hp <= 0) {
            closestSoldier->isAlive = false;
            std::cout << "Soldier killed! Remaining: " << getAliveCount() << "/10" << std::endl;
        }
    }
}

} // namespace BombingRun
