#include "core/Game.h"
#include <iostream>
#include <cmath>

namespace BombingRun {

Game::Game()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_running(false)
    , m_lastFrameTime(0)
    , m_windowWidth(800)
    , m_windowHeight(600)
    , m_selectedBombType(BombType::BOMB_500LB)  // Default to 500lb
    , m_mouseX(0)
    , m_mouseY(0)
{
}

Game::~Game() {
    shutdown();
}

bool Game::initialize(const std::string& title, int width, int height) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Get display bounds to create window at full screen size
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
        width = displayMode.w;
        height = displayMode.h;
        std::cout << "Display resolution: " << width << "x" << height << std::endl;
    }

    // Create window (maximized at full display size)
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED
    );

    if (!m_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Store window dimensions
    m_windowWidth = width;
    m_windowHeight = height;

    // Create renderer with hardware acceleration
    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return false;
    }

    // Set blend mode for transparency support
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // Initialize and set up texture manager
    TextureManager::getInstance().initialize();
    TextureManager::getInstance().setRenderer(m_renderer);

    // Initialize aircraft manager
    m_aircraftManager.initialize(width, height);
    
    // Initialize weapon manager
    m_weaponManager.initialize(width, height);
    
    // Load aircraft sprites
    if (!m_aircraftManager.loadSprites()) {
        std::cerr << "Warning: Failed to load aircraft sprites, using fallback rendering" << std::endl;
    }

    // Create and load map
    m_currentMap = std::make_unique<Map>(width, height);
    if (!m_currentMap->loadMap("map1")) {
        std::cerr << "Failed to load map1" << std::endl;
        return false;
    }

    m_running = true;
    m_lastFrameTime = SDL_GetTicks();

    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Window: " << width << "x" << height << std::endl;
    
    return true;
}

void Game::run() {
    while (m_running) {
        m_perfMonitor.startFrame();
        
        handleEvents();
        
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;
        m_lastFrameTime = currentTime;
        
        update(deltaTime);
        render();
        
        m_perfMonitor.endFrame();
        
        // Frame rate limiting
        Uint32 frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frameTime);
        }
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_running = false;
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    m_windowWidth = event.window.data1;
                    m_windowHeight = event.window.data2;
                    std::cout << "Window resized to: " << m_windowWidth << "x" << m_windowHeight << std::endl;
                }
                break;
                
            case SDL_KEYDOWN:
                // Handle keyboard input
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        m_running = false;
                        break;
                    case SDLK_SPACE:
                        m_aircraftManager.spawnBomber(-1.0f, -1.0f, static_cast<int>(m_selectedBombType));
                        std::cout << "Spacebar pressed - Deployed bomber with bomb type " 
                                  << static_cast<int>(m_selectedBombType) << std::endl;
                        break;
                    case SDLK_a:
                        std::cout << "A pressed - Deploy airstrike" << std::endl;
                        break;
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                    case SDLK_5:
                    case SDLK_6:
                    case SDLK_7:
                        m_selectedBombType = static_cast<BombType>(event.key.keysym.sym - SDLK_1);
                        m_weaponManager.setSelectedBombType(m_selectedBombType);
                        std::cout << "Bomb type " << (static_cast<int>(m_selectedBombType) + 1) 
                                  << " selected (" 
                                  << m_weaponManager.getRemainingBombs(m_selectedBombType)
                                  << " remaining)" << std::endl;
                        break;
                }
                break;
                
            case SDL_MOUSEMOTION:
                // Track mouse position for targeting circle
                m_mouseX = event.motion.x;
                m_mouseY = event.motion.y;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    float targetX = static_cast<float>(event.button.x);
                    float targetY = static_cast<float>(event.button.y);
                    
                    // Spawn bomber that will fly to target and drop bombs
                    m_aircraftManager.spawnBomber(targetX, targetY, static_cast<int>(m_selectedBombType));
                    
                    std::cout << "Mouse clicked at: (" 
                              << event.button.x << ", " 
                              << event.button.y << ") - Spawned bomber" << std::endl;
                }
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Update aircraft (pass weapon manager so bombers can drop bombs)
    m_aircraftManager.update(deltaTime, &m_weaponManager);
    
    // Update weapons (bombs)
    m_weaponManager.update(deltaTime);
    
    // Check for bomb explosions AFTER updating (bombs have hit ground)
    for (const auto& bomb : m_weaponManager.getBombs()) {
        // Only check active bombs to avoid infinite explosion loops
        if (bomb && bomb->isActive() && bomb->shouldExplode()) {
            float bombX = bomb->getX();
            float bombY = bomb->getY();
            int craterSize = bomb->getCraterSize();
            int damage = bomb->getDamage();
            
            // Create explosion and crater
            m_explosionManager.createExplosion(
                bombX, 
                bombY, 
                craterSize * 2  // Explosion is larger than crater
            );
            m_explosionManager.createCrater(
                bombX,
                bombY,
                craterSize
            );
            
            // Damage buildings within explosion radius
            int destroyed = m_currentMap->damageBuildings(
                bombX, 
                bombY, 
                craterSize * 2.5f,  // Damage radius is larger than visual crater
                damage
            );
            
            if (destroyed > 0) {
                std::cout << "Explosion destroyed " << destroyed << " building(s)" << std::endl;
            }
            
            // Mark bomb as exploded so it doesn't explode again next frame
            bomb->markExploded();
        }
    }
    
    // Clean up inactive bombs AFTER processing explosions
    m_weaponManager.cleanupInactiveBombs();
    
    // Update explosions
    m_explosionManager.update(deltaTime);
    
    // Game logic updates will go here
    // For now, just track that the game is updating
    static float totalTime = 0.0f;
    totalTime += deltaTime;
    
    // Print FPS and bomber count every 5 seconds
    if (static_cast<int>(totalTime) % 5 == 0 && deltaTime > 0) {
        static int lastSecond = -1;
        int currentSecond = static_cast<int>(totalTime);
        if (currentSecond != lastSecond) {
            std::cout << "Game running... FPS: " << m_perfMonitor.getAverageFPS() 
                      << " | Bombers: " << m_aircraftManager.getActiveBomberCount()
                      << " | Bombs: " << m_weaponManager.getActiveBombCount()
                      << " | Craters: " << m_explosionManager.getCraters().size()
                      << " | Buildings Destroyed: " << m_currentMap->getDestroyedBuildingCount()
                      << " (" << currentSecond << "s)" << std::endl;
            lastSecond = currentSecond;
        }
    }
}

void Game::render() {
    // Clear screen with sky blue color
    SDL_SetRenderDrawColor(m_renderer, 135, 206, 235, 255);
    SDL_RenderClear(m_renderer);
    
    // Render map (includes grass, roads, buildings, etc.)
    if (m_currentMap) {
        m_currentMap->render(m_renderer);
    }
    
    // Render explosions and craters (after map, before other objects)
    m_explosionManager.render(m_renderer);
    
    // Render bombs
    m_weaponManager.render(m_renderer);
    
    // Render aircraft
    m_aircraftManager.render(m_renderer);
    
    // Render targeting circle (on top of everything)
    int targetRadius = Bomb::getConfig(m_selectedBombType).targetRadius;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 100);  // Yellow with transparency
    
    // Draw circle
    int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (i * 2.0f * M_PI) / segments;
        float angle2 = ((i + 1) * 2.0f * M_PI) / segments;
        
        int x1 = m_mouseX + static_cast<int>(targetRadius * cos(angle1));
        int y1 = m_mouseY + static_cast<int>(targetRadius * sin(angle1));
        int x2 = m_mouseX + static_cast<int>(targetRadius * cos(angle2));
        int y2 = m_mouseY + static_cast<int>(targetRadius * sin(angle2));
        
        SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
    }
    
    // Draw crosshair at mouse position
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 200);  // Red crosshair
    SDL_RenderDrawLine(m_renderer, m_mouseX - 10, m_mouseY, m_mouseX + 10, m_mouseY);
    SDL_RenderDrawLine(m_renderer, m_mouseX, m_mouseY - 10, m_mouseX, m_mouseY + 10);
    
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    
    // Present the rendered frame
    SDL_RenderPresent(m_renderer);
}

void Game::shutdown() {
    // Cleanup texture manager
    TextureManager::getInstance().cleanup();
    
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    SDL_Quit();
    std::cout << "Game shut down successfully." << std::endl;
}

} // namespace BombingRun
