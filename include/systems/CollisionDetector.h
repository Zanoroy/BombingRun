#pragma once

#include <SDL2/SDL.h>
#include <cmath>

namespace BombingRun {

/**
 * @brief Collision detection utilities
 * 
 * Provides static methods for various collision detection algorithms
 * including rectangle-rectangle, circle-circle, and circle-rectangle.
 */
class CollisionDetector {
public:
    /**
     * @brief Check collision between two rectangles
     * @param a First rectangle
     * @param b Second rectangle
     * @return true if rectangles overlap
     */
    static bool checkRectRect(const SDL_Rect& a, const SDL_Rect& b);

    /**
     * @brief Check collision between two circles
     * @param x1 First circle center X
     * @param y1 First circle center Y
     * @param r1 First circle radius
     * @param x2 Second circle center X
     * @param y2 Second circle center Y
     * @param r2 Second circle radius
     * @return true if circles overlap
     */
    static bool checkCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2);

    /**
     * @brief Check collision between circle and rectangle
     * @param cx Circle center X
     * @param cy Circle center Y
     * @param radius Circle radius
     * @param rect Rectangle
     * @return true if circle and rectangle overlap
     */
    static bool checkCircleRect(float cx, float cy, float radius, const SDL_Rect& rect);

    /**
     * @brief Check if a point is inside a rectangle
     * @param px Point X
     * @param py Point Y
     * @param rect Rectangle
     * @return true if point is inside rectangle
     */
    static bool checkPointRect(float px, float py, const SDL_Rect& rect);

    /**
     * @brief Check if a point is inside a circle
     * @param px Point X
     * @param py Point Y
     * @param cx Circle center X
     * @param cy Circle center Y
     * @param radius Circle radius
     * @return true if point is inside circle
     */
    static bool checkPointCircle(float px, float py, float cx, float cy, float radius);

    /**
     * @brief Calculate distance between two points
     * @param x1 First point X
     * @param y1 First point Y
     * @param x2 Second point X
     * @param y2 Second point Y
     * @return Distance between points
     */
    static float distance(float x1, float y1, float x2, float y2);
};

} // namespace BombingRun
