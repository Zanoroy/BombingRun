#include "systems/ExplosionManager.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace BombingRun {

// ============================================================================
// Explosion Implementation
// ============================================================================

Explosion::Explosion(float x, float y, int maxRadius, float duration)
    : m_x(x)
    , m_y(y)
    , m_maxRadius(maxRadius)
    , m_duration(duration)
    , m_elapsed(0.0f)
    , m_currentRadius(0.0f)
    , m_finished(false)
    , m_alpha(255)
{
}

void Explosion::update(float deltaTime) {
    if (m_finished) return;
    
    m_elapsed += deltaTime;
    
    // Calculate progress (0.0 to 1.0)
    float progress = m_elapsed / m_duration;
    
    if (progress >= 1.0f) {
        m_finished = true;
        return;
    }
    
    // Expand radius
    m_currentRadius = m_maxRadius * progress;
    
    // Fade out alpha
    m_alpha = static_cast<uint8_t>(255 * (1.0f - progress));
}

void Explosion::render(SDL_Renderer* renderer) {
    if (m_finished || m_currentRadius < 1.0f) return;
    
    // Draw multiple rings for a more dramatic effect
    for (int ring = 0; ring < 3; ring++) {
        float ringRadius = m_currentRadius - (ring * m_maxRadius * 0.1f);
        if (ringRadius < 1.0f) continue;
        
        // Color shifts from yellow/orange to red
        uint8_t red = 255;
        uint8_t green = static_cast<uint8_t>(200 - ring * 60);
        uint8_t blue = 0;
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, red, green, blue, m_alpha / (ring + 1));
        
        // Draw circle approximation
        int segments = 32;
        for (int i = 0; i < segments; i++) {
            float angle1 = (i * 2.0f * M_PI) / segments;
            float angle2 = ((i + 1) * 2.0f * M_PI) / segments;
            
            int x1 = static_cast<int>(m_x + ringRadius * cos(angle1));
            int y1 = static_cast<int>(m_y + ringRadius * sin(angle1));
            int x2 = static_cast<int>(m_x + ringRadius * cos(angle2));
            int y2 = static_cast<int>(m_y + ringRadius * sin(angle2));
            
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// ============================================================================
// Crater Implementation
// ============================================================================

Crater::Crater(float x, float y, int radius)
    : m_x(x)
    , m_y(y)
    , m_radius(radius)
    , m_age(0.0f)
{
}

void Crater::update(float deltaTime) {
    m_age += deltaTime;
    // TODO: Implement aging visual effects in Phase 10
}

void Crater::render(SDL_Renderer* renderer) {
    // Draw crater as a dark circle
    SDL_SetRenderDrawColor(renderer, 60, 50, 40, 255);  // Dark brown
    
    // Fill circle approximation
    for (int dy = -m_radius; dy <= m_radius; dy++) {
        for (int dx = -m_radius; dx <= m_radius; dx++) {
            if (dx * dx + dy * dy <= m_radius * m_radius) {
                SDL_RenderDrawPoint(renderer,
                    static_cast<int>(m_x + dx),
                    static_cast<int>(m_y + dy));
            }
        }
    }
    
    // Draw crater rim (lighter edge)
    SDL_SetRenderDrawColor(renderer, 100, 90, 70, 255);
    int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (i * 2.0f * M_PI) / segments;
        float angle2 = ((i + 1) * 2.0f * M_PI) / segments;
        
        int x1 = static_cast<int>(m_x + m_radius * cos(angle1));
        int y1 = static_cast<int>(m_y + m_radius * sin(angle1));
        int x2 = static_cast<int>(m_x + m_radius * cos(angle2));
        int y2 = static_cast<int>(m_y + m_radius * sin(angle2));
        
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

bool Crater::overlaps(float x, float y, int radius) const {
    float dx = x - m_x;
    float dy = y - m_y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < (m_radius + radius);
}

// ============================================================================
// ExplosionManager Implementation
// ============================================================================

ExplosionManager::ExplosionManager() {
}

void ExplosionManager::update(float deltaTime) {
    // Update all explosions
    for (auto& explosion : m_explosions) {
        explosion.update(deltaTime);
    }
    
    // Update all craters
    for (auto& crater : m_craters) {
        crater.update(deltaTime);
    }
    
    // Remove finished explosions
    removeFinishedExplosions();
}

void ExplosionManager::render(SDL_Renderer* renderer) {
    // Render craters first (they're on the ground)
    for (auto& crater : m_craters) {
        crater.render(renderer);
    }
    
    // Render explosions on top
    for (auto& explosion : m_explosions) {
        if (!explosion.isFinished()) {
            explosion.render(renderer);
        }
    }
}

void ExplosionManager::createExplosion(float x, float y, int radius, float duration) {
    m_explosions.emplace_back(x, y, radius, duration);
    std::cout << "Created explosion at (" << x << ", " << y 
              << ") radius: " << radius << std::endl;
}

void ExplosionManager::createCrater(float x, float y, int radius) {
    m_craters.emplace_back(x, y, radius);
    std::cout << "Created crater at (" << x << ", " << y 
              << ") radius: " << radius << std::endl;
}

bool ExplosionManager::isInCrater(float x, float y) const {
    for (const auto& crater : m_craters) {
        float dx = x - crater.getX();
        float dy = y - crater.getY();
        float distance = sqrt(dx * dx + dy * dy);
        if (distance <= crater.getRadius()) {
            return true;
        }
    }
    return false;
}

void ExplosionManager::clearAll() {
    m_explosions.clear();
    m_craters.clear();
    std::cout << "Cleared all explosions and craters" << std::endl;
}

void ExplosionManager::removeFinishedExplosions() {
    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
            [](const Explosion& exp) { return exp.isFinished(); }),
        m_explosions.end()
    );
}

} // namespace BombingRun
