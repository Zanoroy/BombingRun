#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <string>
#include "systems/TextureManager.h"
#include "systems/AircraftManager.h"
#include "systems/WeaponManager.h"
#include "systems/ExplosionManager.h"
#include "systems/BuildingManager.h"
#include "systems/AudioManager.h"
#include "entities/PlayerJet.h"
#include "entities/Bullet.h"
#include "entities/AAAGun.h"
#include "entities/PlayerControlledJet.h"
#include "entities/Player2Jet.h"
#include "entities/Troops.h"
#include "maps/Map.h"
#include "utils/PerformanceMonitor.h"
#include "entities/Bomb.h"
#include "core/GameState.h"

namespace BombingRun {

/**
 * @brief Game difficulty levels
 */
enum class Difficulty {
    EASY,
    NORMAL,
    HARD
};

/**
 * @brief Optional mission objectives
 */
struct Mission {
    std::string name;
    std::string description;
    bool completed;
    bool failed;
    
    // Mission tracking variables
    int targetCount;
    int currentCount;
    bool trackAAA;  // Track AAA gun destruction
    bool prohibitAAA;  // Fail if AAA destroyed
    float timeLimit;  // Time limit in seconds (0 = no limit)
    float timeElapsed;
};

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

    /**
     * @brief Render main menu
     */
    void renderMenu();

    /**
     * @brief Render PVP mode
     */
    void renderPVP();

    /**
     * @brief Update PVP mode
     */
    void updatePVP(float deltaTime);

    /**
     * @brief Render Jet Fights mode
     */
    void renderJetFights();

    /**
     * @brief Update Jet Fights mode
     */
    void updateJetFights(float deltaTime);

    /**
     * @brief Render HUD elements
     */
    void renderHUD();
    
    /**
     * @brief Render victory screen
     * @param winnerID Player ID of the winner (1 or 2)
     */
    void renderVictory(int winnerID);
    
    /**
     * @brief Render defeat screen
     * @param drawFlag 0 for draw, other values unused
     */
    void renderDefeat(int drawFlag);
    
    /**
     * @brief Render pause menu
     */
    void renderPauseMenu();
    
    /**
     * @brief Initialize mission objectives
     */
    void initializeMissions();
    
    /**
     * @brief Update mission progress
     */
    void updateMissions();
    
    /**
     * @brief Render mission objectives
     */
    void renderMissions();
    
    /**
     * @brief Check if mouse is over a rectangle
     */
    bool isMouseOver(int mouseX, int mouseY, const SDL_Rect& rect);
    
    /**
     * @brief Trigger screen shake effect
     * @param intensity Shake intensity (larger = more shake)
     * @param duration Duration in seconds
     */
    void triggerScreenShake(float intensity, float duration);

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
    BuildingManager m_buildingManager;
    AudioManager m_audioManager;
    std::unique_ptr<Map> m_currentMap;
    BombType m_selectedBombType;  // Currently selected bomb type
    int m_mouseX;                  // Current mouse X position
    int m_mouseY;                  // Current mouse Y position
    TTF_Font* m_font;              // Font for text rendering
    bool m_airstrikeMode;          // Whether airstrike targeting is active
    float m_airstrikeTargetX;      // Target X for airstrike
    float m_airstrikeTargetY;      // Target Y for airstrike
    GameState m_gameState;         // Current game state
    std::string m_selectedMap;     // Selected map ("city" or "battleground")
    std::vector<std::unique_ptr<AAAGun>> m_aaaGuns;  // Anti-aircraft guns
    
    // Troops system
    std::vector<std::unique_ptr<Troops>> m_playerTroops;   // Player-controlled troop groups
    std::vector<std::unique_ptr<Troops>> m_enemyTroops;    // Enemy troop groups
    Troops* m_selectedTroopGroup;                          // Currently selected troop group
    bool m_troopOrderMode;                                 // Waiting for move order click
    std::vector<std::shared_ptr<Bullet>> m_troopBullets;  // Bullets from troops
    
    // PVP mode
    std::unique_ptr<PlayerJet> m_player1Jet;
    std::unique_ptr<PlayerJet> m_player2Jet;
    std::vector<std::unique_ptr<Bullet>> m_pvpBullets;
    int m_pvpMapSize;              // Size of PVP ocean map
    
    // Jet Fights mode
    std::unique_ptr<PlayerControlledJet> m_playerControlledJet;
    std::unique_ptr<Player2Jet> m_player2ControlledJet;
    std::vector<std::unique_ptr<Bullet>> m_jetFightsBullets;
    bool m_wPressed;
    bool m_sPressed;
    bool m_aPressed;
    bool m_dPressed;
    bool m_spacePressed;
    bool m_upPressed;
    bool m_downPressed;
    bool m_leftPressed;
    bool m_rightPressed;
    bool m_rctrlPressed;
    bool m_jetFightsGameOver;
    int m_jetFightsWinner;  // 0=draw, 1=player1, 2=player2
    
    // Difficulty and missions
    Difficulty m_difficulty;
    std::vector<Mission> m_missions;
    int m_aaaGunsDestroyedThisGame;
    int m_buildingsDestroyedThisGame;
    int m_bombersLostThisGame;
    float m_gameTime;
    
    // Trail effects
    struct TrailParticle {
        float x, y;
        float lifetime;
        float maxLifetime;
    };
    std::vector<TrailParticle> m_trailParticles;
    
    // Smoke effects
    struct SmokeParticle {
        float x, y;
        float vx, vy;
        float lifetime;
        float maxLifetime;
        int alpha;
    };
    std::vector<SmokeParticle> m_smokeParticles;
    
    // Screen shake effects
    float m_screenShakeIntensity;  // Current shake intensity
    float m_screenShakeDuration;   // Remaining shake duration
    float m_screenShakeX;          // Current shake offset X
    float m_screenShakeY;          // Current shake offset Y

    // Target frame time for 60 FPS (16.67ms)
    static constexpr Uint32 TARGET_FRAME_TIME = 16;
};

} // namespace BombingRun
