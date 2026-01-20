#pragma once

#include "GameObject.h"
#include "Bullet.h"
#include <vector>
#include <memory>

namespace BombingRun {

// Individual soldier in a troop group
struct Soldier {
    float offsetX;        // Position offset from group center
    float offsetY;
    int hp;               // Hit points (max 15)
    int ammoCount;        // Bullets remaining (reloads at 0, max 3)
    float shootCooldown;  // Time until can shoot again
    float reloadTime;     // Time remaining for reload
    bool isReloading;
    bool isAlive;
    int targetIndex;      // Which enemy soldier this one is targeting (-1 = none)
    
    Soldier(float ox, float oy) 
        : offsetX(ox), offsetY(oy), hp(15), ammoCount(3), 
          shootCooldown(0.0f), reloadTime(0.0f), 
          isReloading(false), isAlive(true), targetIndex(-1) {}
};

/**
 * @brief Troops unit controlled by player
 * 
 * Group of 10 soldiers that can be ordered to move and engage enemies
 * Each soldier has 3 HP, 5-shot capacity before reload
 */
class Troops : public GameObject {
public:
    Troops(float x, float y, bool isEnemy = false);
    ~Troops() override = default;

    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer, TTF_Font* font = nullptr) override;
    
    // Movement and selection
    void setDestination(float x, float y);
    void select();
    void deselect();
    bool isSelected() const { return m_selected; }
    bool hasDestination() const { return m_hasDestination; }
    
    // Combat
    void engageTarget(Troops* target);
    void disengageTarget();
    bool isInCombat() const { return m_inCombat; }
    bool canMove() const { return !m_inCombat && !m_isResting; }
    
    // Get bullets fired this update
    std::vector<std::shared_ptr<Bullet>>& getBullets() { return m_bulletsFired; }
    void clearBullets() { m_bulletsFired.clear(); }
    
    // Status checks
    bool isResting() const { return m_isResting; }
    int getAliveCount() const;
    bool isEnemy() const { return m_isEnemy; }
    
    // Collision detection
    bool containsPoint(float px, float py) const;
    float getDistanceTo(const Troops* other) const;
    
    // Position getters for soldiers
    void getSoldierPositions(std::vector<std::pair<float, float>>& positions) const;
    
    // Damage system
    void damageSoldierAt(float x, float y, int damage = 1);

private:
    void updateMovement(float deltaTime);
    void updateCombat(float deltaTime);
    void updateResting(float deltaTime);
    void updateSoldierShooting(float deltaTime);
    
    void formationSetup();  // Initialize soldier formation
    void checkRestNeeded();
    
    std::vector<Soldier> m_soldiers;
    std::vector<std::shared_ptr<Bullet>> m_bulletsFired;
    
    // Movement
    float m_destX;
    float m_destY;
    bool m_hasDestination;
    float m_speed;
    float m_distanceTraveled;
    static constexpr float REST_DISTANCE = 300.0f;  // Distance before rest needed
    static constexpr float REST_DURATION = 3.0f;     // Rest time in seconds
    
    // Combat
    Troops* m_target;
    bool m_inCombat;
    static constexpr float FIRING_RANGE = 150.0f;
    static constexpr float RELOAD_TIME = 0.5f;  // Fast reload
    
    // State
    bool m_selected;
    bool m_isResting;
    float m_restTimer;
    bool m_isEnemy;
    
    // Formation (2 rows of 5)
    static constexpr float SOLDIER_SPACING = 12.0f;
    static constexpr float ROW_SPACING = 15.0f;
    static constexpr int SOLDIERS_PER_GROUP = 10;
};

} // namespace BombingRun
