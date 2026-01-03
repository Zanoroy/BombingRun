#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include "systems/TextureManager.h"
#include "systems/AircraftManager.h"
#include "systems/WeaponManager.h"
#include "systems/ExplosionManager.h"
#include "maps/Map.h"
#include "utils/PerformanceMonitor.h"
#include "entities/Bomb.h"

namespace BombingRun {

/**
 * @brief Main game controller managing the game loop and window
 * 
 * Handles initialization, main game loop, event processing,
 * updating game state, and rendering.
 */
class Game {
public:
    Game();
    ~Game();

    // Delete copy constructor and assignment operator
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    /**
     * @brief Initialize SDL2, create window and renderer
     * @param title Window title
     * @param width Window width
     * @param height Window height
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& title, int width, int height);

    /**
     * @brief Main game loop
     */
    void run();

    /**
     * @brief Shutdown and cleanup SDL2 resources
     */
    void shutdown();

    /**
     * @brief Check if game is running
     * @return true if running, false if should quit
     */
    bool isRunning() const { return m_running; }

private:
    /**
     * @brief Process input events
     */
    void handleEvents();

    /**
     * @brief Update game logic
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Render all game objects
     */
    void render();

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    bool m_running;
    Uint32 m_lastFrameTime;
    int m_windowWidth;
    int m_windowHeight;
    PerformanceMonitor m_perfMonitor;
    AircraftManager m_aircraftManager;
    WeaponManager m_weaponManager;
    ExplosionManager m_explosionManager;
    std::unique_ptr<Map> m_currentMap;
    BombType m_selectedBombType;  // Currently selected bomb type
    int m_mouseX;                  // Current mouse X position
    int m_mouseY;                  // Current mouse Y position

    // Target frame time for 60 FPS (16.67ms)
    static constexpr Uint32 TARGET_FRAME_TIME = 16;
};

} // namespace BombingRun
