#include "utils/PerformanceMonitor.h"
#include <iostream>

namespace BombingRun {

PerformanceMonitor::PerformanceMonitor()
    : m_frameStart(0)
    , m_frameTime(0.0f)
    , m_currentFPS(0.0f)
{
    m_fpsHistory.reserve(MAX_HISTORY);
}

void PerformanceMonitor::startFrame() {
    m_frameStart = SDL_GetTicks();
}

void PerformanceMonitor::endFrame() {
    Uint32 frameEnd = SDL_GetTicks();
    m_frameTime = static_cast<float>(frameEnd - m_frameStart);
    
    // Calculate FPS
    if (m_frameTime > 0.0f) {
        m_currentFPS = 1000.0f / m_frameTime;
        
        // Update history
        m_fpsHistory.push_back(m_currentFPS);
        if (m_fpsHistory.size() > MAX_HISTORY) {
            m_fpsHistory.erase(m_fpsHistory.begin());
        }
    }
}

float PerformanceMonitor::getAverageFPS() const {
    if (m_fpsHistory.empty()) {
        return 0.0f;
    }
    
    float sum = std::accumulate(m_fpsHistory.begin(), m_fpsHistory.end(), 0.0f);
    return sum / m_fpsHistory.size();
}

bool PerformanceMonitor::isBelowTarget(float targetFPS) const {
    return getAverageFPS() < targetFPS;
}

} // namespace BombingRun
