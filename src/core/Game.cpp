#include "core/Game.h"
#include <iostream>

namespace BombingRun {

Game::Game()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_running(false)
    , m_lastFrameTime(0)
    , m_windowWidth(800)
    , m_windowHeight(600)
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
                        std::cout << "Spacebar pressed - Deploy bomber" << std::endl;
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
                        std::cout << "Bomb type " << (event.key.keysym.sym - SDLK_0) << " selected" << std::endl;
                        break;
                }
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    std::cout << "Mouse clicked at: (" 
                              << event.button.x << ", " 
                              << event.button.y << ")" << std::endl;
                }
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Game logic updates will go here
    // For now, just track that the game is updating
    static float totalTime = 0.0f;
    totalTime += deltaTime;
    
    // Print FPS every 5 seconds
    if (static_cast<int>(totalTime) % 5 == 0 && deltaTime > 0) {
        static int lastSecond = -1;
        int currentSecond = static_cast<int>(totalTime);
        if (currentSecond != lastSecond) {
            std::cout << "Game running... FPS: " << m_perfMonitor.getAverageFPS() 
                      << " (" << currentSecond << "s)" << std::endl;
            lastSecond = currentSecond;
        }
    }
}

void Game::render() {
    // Clear screen with sky blue color
    SDL_SetRenderDrawColor(m_renderer, 135, 206, 235, 255);
    SDL_RenderClear(m_renderer);
    
    // Draw grass ground (bottom third of screen)
    int groundHeight = m_windowHeight / 3;
    SDL_Rect groundRect = {0, m_windowHeight - groundHeight, m_windowWidth, groundHeight};
    SDL_SetRenderDrawColor(m_renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(m_renderer, &groundRect);
    
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
