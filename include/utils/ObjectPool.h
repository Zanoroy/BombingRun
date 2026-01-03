#pragma once

#include <vector>
#include <memory>
#include <functional>

namespace BombingRun {

/**
 * @brief Generic object pool for efficient object reuse
 * 
 * Manages a pool of reusable objects to avoid frequent allocation/deallocation.
 * Objects are created on demand and reused when available.
 * 
 * @tparam T Type of objects to pool
 */
template<typename T>
class ObjectPool {
public:
    /**
     * @brief Constructor with optional initial capacity
     * @param initialSize Number of objects to pre-allocate
     */
    explicit ObjectPool(size_t initialSize = 0) {
        if (initialSize > 0) {
            m_pool.reserve(initialSize);
            for (size_t i = 0; i < initialSize; ++i) {
                m_pool.push_back(std::make_unique<T>());
            }
        }
    }

    /**
     * @brief Get an object from the pool
     * @return Pointer to available object
     */
    T* acquire() {
        // Try to find an inactive object
        for (auto& obj : m_pool) {
            if (!obj->isActive()) {
                return obj.get();
            }
        }

        // No inactive objects, create new one
        m_pool.push_back(std::make_unique<T>());
        return m_pool.back().get();
    }

    /**
     * @brief Update all active objects
     * @param deltaTime Time since last update
     */
    void updateAll(float deltaTime) {
        for (auto& obj : m_pool) {
            if (obj->isActive()) {
                obj->update(deltaTime);
            }
        }
    }

    /**
     * @brief Render all active objects
     * @param renderer SDL renderer
     */
    void renderAll(SDL_Renderer* renderer) {
        for (auto& obj : m_pool) {
            if (obj->isActive()) {
                obj->render(renderer);
            }
        }
    }

    /**
     * @brief Get count of active objects
     * @return Number of active objects
     */
    size_t getActiveCount() const {
        size_t count = 0;
        for (const auto& obj : m_pool) {
            if (obj->isActive()) {
                ++count;
            }
        }
        return count;
    }

    /**
     * @brief Get total pool size
     * @return Total number of objects in pool
     */
    size_t getPoolSize() const {
        return m_pool.size();
    }

    /**
     * @brief Deactivate all objects
     */
    void clear() {
        for (auto& obj : m_pool) {
            obj->destroy();
        }
    }

    /**
     * @brief Execute a function on all active objects
     * @param func Function to execute
     */
    void forEachActive(std::function<void(T*)> func) {
        for (auto& obj : m_pool) {
            if (obj->isActive()) {
                func(obj.get());
            }
        }
    }

private:
    std::vector<std::unique_ptr<T>> m_pool;
};

} // namespace BombingRun
