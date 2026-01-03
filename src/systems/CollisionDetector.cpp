#include "systems/CollisionDetector.h"

namespace BombingRun {

bool CollisionDetector::checkRectRect(const SDL_Rect& a, const SDL_Rect& b) {
    return !(a.x + a.w < b.x || 
             b.x + b.w < a.x || 
             a.y + a.h < b.y || 
             b.y + b.h < a.y);
}

bool CollisionDetector::checkCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dist = distance(x1, y1, x2, y2);
    return dist < (r1 + r2);
}

bool CollisionDetector::checkCircleRect(float cx, float cy, float radius, const SDL_Rect& rect) {
    // Find the closest point on the rectangle to the circle center
    float closestX = std::max(static_cast<float>(rect.x), 
                              std::min(cx, static_cast<float>(rect.x + rect.w)));
    float closestY = std::max(static_cast<float>(rect.y), 
                              std::min(cy, static_cast<float>(rect.y + rect.h)));
    
    // Calculate distance from circle center to closest point
    float dist = distance(cx, cy, closestX, closestY);
    
    return dist < radius;
}

bool CollisionDetector::checkPointRect(float px, float py, const SDL_Rect& rect) {
    return px >= rect.x && 
           px <= rect.x + rect.w && 
           py >= rect.y && 
           py <= rect.y + rect.h;
}

bool CollisionDetector::checkPointCircle(float px, float py, float cx, float cy, float radius) {
    float dist = distance(px, py, cx, cy);
    return dist <= radius;
}

float CollisionDetector::distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}

} // namespace BombingRun
