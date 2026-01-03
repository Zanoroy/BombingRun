#pragma once

#include <SDL2/SDL.h>
#include <vector>

namespace BombingRun {

/**
 * Explosion
 * Represents an explosion animation with expanding rings
 */
class Explosion {
public:
    Explosion(float x, float y, int maxRadius, float duration);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    bool isFinished() const { return m_finished; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    int getMaxRadius() const { return m_maxRadius; }

private:
    float m_x, m_y;
    int m_maxRadius;
    float m_duration;
    float m_elapsed;
    float m_currentRadius;
    bool m_finished;
    
    // Color that fades from bright to dark
    uint8_t m_alpha;
};

/**
 * Crater
 * Permanent ground damage from bomb impact
 */
class Crater {
public:
    Crater(float x, float y, int radius);
    
    void render(SDL_Renderer* renderer);
    void update(float deltaTime);  // For aging effects later
    
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    int getRadius() const { return m_radius; }
    bool overlaps(float x, float y, int radius) const;

private:
    float m_x, m_y;
    int m_radius;
    float m_age;  // For visual aging effects
};

/**
 * ExplosionManager
 * Manages all explosions and craters
 */
class ExplosionManager {
public:
    ExplosionManager();
    ~ExplosionManager() = default;
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    // Create explosions and craters
    void createExplosion(float x, float y, int radius, float duration = 0.5f);
    void createCrater(float x, float y, int radius);
    
    // Queries
    bool isInCrater(float x, float y) const;
    const std::vector<Crater>& getCraters() const { return m_craters; }
    
    void clearAll();

private:
    std::vector<Explosion> m_explosions;
    std::vector<Crater> m_craters;
    
    void removeFinishedExplosions();
};

} // namespace BombingRun
