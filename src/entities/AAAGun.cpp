#include "entities/AAAGun.h"
#include "entities/Bomber.h"
#include "entities/FighterJet.h"
#include "systems/WeaponManager.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <random>

namespace BombingRun {

AAAGun::AAAGun(float x, float y)
    : GameObject(x, y)
    , m_barrelAngle(0.0f)
    , m_fireTimer(0.0f)
    , m_fireRate(FIRE_RATE)
    , m_range(RANGE)
    , m_bulletSpeed(BULLET_SPEED)
    , m_targetX(x)
    , m_targetY(y)
    , m_isFiring(false)
    , m_muzzleFlashTimer(0.0f)
    , m_currentTarget(nullptr)
    , m_health(MAX_HEALTH)
{
    m_active = true;
}

void AAAGun::update(float deltaTime) {
    // Base implementation - unused
}

void AAAGun::updateTargeting(float deltaTime, const std::vector<Bomber*>& bombers,
                            const std::vector<FighterJet*>& fighters,
                            WeaponManager* weaponManager) {
    // Decrease fire timer
    if (m_fireTimer > 0.0f) {
        m_fireTimer -= deltaTime;
    }

    // Decrease muzzle flash timer
    if (m_muzzleFlashTimer > 0.0f) {
        m_muzzleFlashTimer -= deltaTime;
        if (m_muzzleFlashTimer <= 0.0f) {
            m_isFiring = false;
        }
    }

    // Find closest aircraft (bomber or fighter) in range
    GameObject* closestTarget = nullptr;
    float closestDist = m_range;

    // Check all bombers
    for (Bomber* bomber : bombers) {
        if (!bomber || !bomber->isActive()) continue;

        float dx = bomber->getX() - m_x;
        float dy = bomber->getY() - m_y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < closestDist) {
            closestDist = dist;
            closestTarget = bomber;
        }
    }

    // Check all fighters
    for (FighterJet* fighter : fighters) {
        if (!fighter || !fighter->isActive()) continue;

        float dx = fighter->getX() - m_x;
        float dy = fighter->getY() - m_y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < closestDist) {
            closestDist = dist;
            closestTarget = fighter;
        }
    }

    m_currentTarget = closestTarget;

    // If we have a target, aim and fire
    if (m_currentTarget) {
        // Get target velocity
        float targetVelX = m_currentTarget->getVelocityX();
        float targetVelY = m_currentTarget->getVelocityY();

        // Calculate intercept point
        float interceptX, interceptY;
        if (calculateInterceptPoint(m_currentTarget->getX(), m_currentTarget->getY(),
                                     targetVelX, targetVelY,
                                     interceptX, interceptY)) {
            m_targetX = interceptX;
            m_targetY = interceptY;

            // Calculate desired barrel angle
            float dx = m_targetX - m_x;
            float dy = m_targetY - m_y;
            float desiredAngle = std::atan2(dy, dx);

            // Smoothly rotate barrel towards target
            float angleDiff = desiredAngle - m_barrelAngle;
            // Normalize angle difference to [-PI, PI]
            while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
            while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

            float rotationStep = ROTATION_SPEED * deltaTime;
            if (std::abs(angleDiff) < rotationStep) {
                m_barrelAngle = desiredAngle;
            } else {
                m_barrelAngle += (angleDiff > 0 ? rotationStep : -rotationStep);
            }

            // Fire if cooldown ready and aimed close enough
            if (m_fireTimer <= 0.0f && std::abs(angleDiff) < 0.1f && weaponManager) {
                fire(weaponManager);
                m_fireTimer = 1.0f / m_fireRate;
            }
        }
    }
}

bool AAAGun::calculateInterceptPoint(float targetX, float targetY,
                                       float targetVelX, float targetVelY,
                                       float& outX, float& outY) {
    // Calculate intercept point for bullet projectile
    // This solves for where to aim so bullet and target meet
    
    float dx = targetX - m_x;
    float dy = targetY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 1.0f) {
        outX = targetX;
        outY = targetY;
        return true;
    }

    // Use bullet speed to calculate time to intercept
    float timeToIntercept = distance / m_bulletSpeed;

    // Iteratively refine prediction (2 iterations should be enough)
    for (int i = 0; i < 2; i++) {
        outX = targetX + targetVelX * timeToIntercept;
        outY = targetY + targetVelY * timeToIntercept;
        
        dx = outX - m_x;
        dy = outY - m_y;
        distance = std::sqrt(dx * dx + dy * dy);
        timeToIntercept = distance / m_bulletSpeed;
    }

    // Check if predicted position is in range
    return distance <= m_range;
}

void AAAGun::fire(WeaponManager* weaponManager) {
    if (!m_currentTarget || !weaponManager) return;

    m_isFiring = true;
    m_muzzleFlashTimer = 0.1f;  // Short muzzle flash

    // Fire bullet projectile at intercept point
    float dx = m_targetX - m_x;
    float dy = m_targetY - m_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0.0f) {
        // Calculate bullet velocity toward intercept point
        float velX = (dx / distance) * m_bulletSpeed;
        float velY = (dy / distance) * m_bulletSpeed;

        // Add inaccuracy - random spread of ±50 pixels at target distance
        // This gives roughly 30-40% accuracy depending on distance
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> spreadDist(-50.0f, 50.0f);
        
        float spreadX = spreadDist(gen);
        float spreadY = spreadDist(gen);
        
        // Apply spread to target position
        float adjustedDx = dx + spreadX;
        float adjustedDy = dy + spreadY;
        float adjustedDist = std::sqrt(adjustedDx * adjustedDx + adjustedDy * adjustedDy);
        
        if (adjustedDist > 0.0f) {
            velX = (adjustedDx / adjustedDist) * m_bulletSpeed;
            velY = (adjustedDy / adjustedDist) * m_bulletSpeed;
        }

        // Fire bullet from barrel end
        const float BARREL_LENGTH = 25.0f;
        float barrelEndX = m_x + std::cos(m_barrelAngle) * BARREL_LENGTH;
        float barrelEndY = m_y + std::sin(m_barrelAngle) * BARREL_LENGTH;

        // Fire larger AAA bullet (12.0f size vs default 6.0f)
        weaponManager->fireBullet(barrelEndX, barrelEndY, velX, velY, nullptr, 12.0f);
        
        std::cout << "AAA fired at target! Vel: (" << velX << ", " << velY << ")" << std::endl;
    }
}

void AAAGun::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (!m_active) return;

    // Draw base (dark gray circle)
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    const int baseRadius = 10;
    for (int w = 0; w < baseRadius * 2; w++) {
        for (int h = 0; h < baseRadius * 2; h++) {
            int dx = baseRadius - w;
            int dy = baseRadius - h;
            if ((dx*dx + dy*dy) <= (baseRadius * baseRadius)) {
                SDL_RenderDrawPoint(renderer, static_cast<int>(m_x) + dx, 
                                   static_cast<int>(m_y) + dy);
            }
        }
    }

    // Draw barrel (orange line pointing at target)
    SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);
    const int barrelLength = 25;
    int barrelEndX = static_cast<int>(m_x + std::cos(m_barrelAngle) * barrelLength);
    int barrelEndY = static_cast<int>(m_y + std::sin(m_barrelAngle) * barrelLength);
    
    // Draw thick barrel (3 pixel width)
    for (int offset = -1; offset <= 1; offset++) {
        SDL_RenderDrawLine(renderer, 
                          static_cast<int>(m_x), static_cast<int>(m_y) + offset,
                          barrelEndX, barrelEndY + offset);
    }

    // Draw muzzle flash if firing
    if (m_isFiring && m_muzzleFlashTimer > 0.0f) {
        // Draw bright white muzzle flash circle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        const int flashRadius = 6;
        for (int w = 0; w < flashRadius * 2; w++) {
            for (int h = 0; h < flashRadius * 2; h++) {
                int dx = flashRadius - w;
                int dy = flashRadius - h;
                if ((dx*dx + dy*dy) <= (flashRadius * flashRadius)) {
                    SDL_RenderDrawPoint(renderer, barrelEndX + dx, barrelEndY + dy);
                }
            }
        }
    }
}

void AAAGun::takeDamage(int damage) {
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_active = false;
        std::cout << "AAA Gun destroyed at (" << m_x << ", " << m_y << ")!" << std::endl;
    } else {
        std::cout << "AAA Gun hit! Health: " << m_health << "/" << MAX_HEALTH << std::endl;
    }
}

} // namespace BombingRun
