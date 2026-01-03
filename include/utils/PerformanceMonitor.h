#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <numeric>

namespace BombingRun {

/**
 * @brief Performance monitoring utility
 * 
 * Tracks frame rate and provides FPS statistics
 */
class PerformanceMonitor {
public:
    PerformanceMonitor();

    /**
     * @brief Start frame timing
     */
    void startFrame();

    /**
     * @brief End frame timing and update statistics
     */
    void endFrame();

    /**
     * @brief Get current FPS
     * @return Frames per second
     */
    float getFPS() const { return m_currentFPS; }

    /**
     * @brief Get average FPS over last second
     * @return Average FPS
     */
    float getAverageFPS() const;

    /**
     * @brief Get frame time in milliseconds
     * @return Frame time in ms
     */
    float getFrameTime() const { return m_frameTime; }

    /**
     * @brief Check if performance is below target
     * @param targetFPS Target frame rate
     * @return true if below target
     */
    bool isBelowTarget(float targetFPS) const;

private:
    Uint32 m_frameStart;
    float m_frameTime;
    float m_currentFPS;
    std::vector<float> m_fpsHistory;
    static constexpr size_t MAX_HISTORY = 60; // 1 second at 60 FPS
};

} // namespace BombingRun
