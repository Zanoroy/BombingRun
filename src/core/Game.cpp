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
    , m_font(nullptr)
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

    // Initialize SDL_ttf
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
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

    // Try to load a default font (will use fallback text rendering if it fails)
    m_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
    if (!m_font) {
        std::cerr << "Warning: Failed to load font, using fallback: " << TTF_GetError() << std::endl;
        // Try alternative font path
        m_font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf", 16);
        if (!m_font) {
            std::cerr << "Warning: Using no font - text rendering disabled" << std::endl;
        }
    }

    // Create and load map
    m_currentMap = std::make_unique<Map>(width, height);
    if (!m_currentMap->loadMap("battleground")) {
        std::cerr << "Failed to load battleground map" << std::endl;
        return false;
    }
    
    // Set runway position for fighter jets
    if (m_currentMap->getRunway()) {
        float runwayX = m_currentMap->getRunway()->getX() + m_currentMap->getRunway()->getWidth() / 2.0f;
        float runwayY = m_currentMap->getRunway()->getY() + m_currentMap->getRunway()->getHeight() / 2.0f;
        m_aircraftManager.setRunwayPosition(runwayX, runwayY);
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
                    case SDLK_f:
                        m_aircraftManager.spawnFighterJet();
                        std::cout << "F pressed - Fighter jet deployed!" << std::endl;
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
    
    // Update weapons (bombs and bullets)
    m_weaponManager.update(deltaTime);
    
    // Set runway Y position on all bullets
    if (m_currentMap && m_currentMap->getRunway()) {
        float runwayY = m_currentMap->getRunway()->getY();
        for (const auto& bullet : m_weaponManager.getBullets()) {
            if (bullet && bullet->isActive()) {
                bullet->setRunwayY(runwayY);
            }
        }
    }
    
    // Check bullet collisions with bombers
    for (const auto& bullet : m_weaponManager.getBullets()) {
        if (bullet && bullet->isActive()) {
            Bomber* hitBomber = m_aircraftManager.checkBomberCollision(bullet->getX(), bullet->getY());
            if (hitBomber) {
                hitBomber->takeDamage(1);  // Bullets do 1 damage
                bullet->markHit();  // Destroy the bullet
                
                // Check if bomber is destroyed
                if (hitBomber->getHealth() <= 0) {
                    std::cout << "Bomber destroyed by fighter jet!" << std::endl;
                    hitBomber->destroy();
                }
            }
            
            // Check bullet collision with runway (missed shots can damage runway)
            if (m_currentMap) {
                Runway* runway = m_currentMap->getRunway();
                if (runway) {
                    float runwayX = runway->getX();
                    float runwayY = runway->getY();
                    float runwayWidth = runway->getWidth();
                    float runwayHeight = runway->getHeight();
                    
                    // Expanded hit area to guarantee hits (3x width, 3x height)
                    float hitWidth = runwayWidth * 3.0f;
                    float hitHeight = runwayHeight * 3.0f;
                    
                    // Check if bullet hit runway (generous hitbox)
                    if (bullet->getX() >= runwayX - hitWidth / 2 &&
                        bullet->getX() <= runwayX + hitWidth / 2 &&
                        bullet->getY() >= runwayY - hitHeight / 2 &&
                        bullet->getY() <= runwayY + hitHeight / 2) {
                        
                        runway->takeDamage(1);  // Bullets do 1 damage to runway
                        bullet->markHit();  // Destroy the bullet
                        std::cout << "Bullet hit runway! HP: " << runway->getHealth() << "/" << runway->getMaxHealth() << std::endl;
                    }
                }
            }
        }
    }
    
    // Check for bomb explosions AFTER updating (bombs have hit ground)
    for (const auto& bomb : m_weaponManager.getBombs()) {
        // Only check active bombs to avoid infinite explosion loops
        if (bomb && bomb->isActive() && bomb->shouldExplode()) {
            float bombX = bomb->getX();
            float bombY = bomb->getY();
            int craterSize = bomb->getCraterSize();
            // Scale crater and explosion by 0.25x for BattleGround map (4x larger map)
            float scaledCraterSize = craterSize * 0.25f;
            int damage = bomb->getDamage();
            
            // Create explosion and crater
            m_explosionManager.createExplosion(
                bombX, 
                bombY, 
                static_cast<int>(scaledCraterSize * 2)  // Explosion is larger than crater
            );
            m_explosionManager.createCrater(
                bombX,
                bombY,
                static_cast<int>(scaledCraterSize)
            );
            
            // Damage buildings within explosion radius
            // Use scaled crater size for damage radius too!
            int destroyed = m_currentMap->damageBuildings(
                bombX, 
                bombY, 
                scaledCraterSize * 2.5f,  // Damage radius is larger than visual crater (scaled)
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
        m_currentMap->render(m_renderer, m_font);
    }
    
    // Render explosions and craters (after map, before other objects)
    m_explosionManager.render(m_renderer);
    
    // Render bombs
    m_weaponManager.render(m_renderer);
    
    // Render aircraft
    m_aircraftManager.render(m_renderer);
    
    // Render targeting circle (on top of everything)
    int targetRadius = Bomb::getConfig(m_selectedBombType).targetRadius;
    // Scale target circle by 0.25x for BattleGround map (4x larger map)
    float scaledRadius = targetRadius * 0.25f;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 100);  // Yellow with transparency
    
    // Draw circle
    int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (i * 2.0f * M_PI) / segments;
        float angle2 = ((i + 1) * 2.0f * M_PI) / segments;
        
        int x1 = m_mouseX + static_cast<int>(scaledRadius * cos(angle1));
        int y1 = m_mouseY + static_cast<int>(scaledRadius * sin(angle1));
        int x2 = m_mouseX + static_cast<int>(scaledRadius * cos(angle2));
        int y2 = m_mouseY + static_cast<int>(scaledRadius * sin(angle2));
        
        SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
    }
    
    // Draw crosshair at mouse position
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 200);  // Red crosshair
    SDL_RenderDrawLine(m_renderer, m_mouseX - 10, m_mouseY, m_mouseX + 10, m_mouseY);
    SDL_RenderDrawLine(m_renderer, m_mouseX, m_mouseY - 10, m_mouseX, m_mouseY + 10);
    
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    
    // Draw bomb selection indicator near mouse cursor
    if (m_font) {
        const char* bombNames[] = {
            "100lb",
            "250lb", 
            "500lb",
            "1000lb",
            "2000lb",
            "4000lb",
            "8000lb"
        };
        
        std::string indicatorText = bombNames[static_cast<int>(m_selectedBombType)];
        indicatorText += " (";
        indicatorText += std::to_string(m_weaponManager.getRemainingBombs(m_selectedBombType));
        indicatorText += ")";
        
        // Render text to surface
        SDL_Color textColor = {255, 255, 0, 255};  // Yellow
        SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, indicatorText.c_str(), textColor);
        
        if (textSurface) {
            // Create texture from surface
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
            
            if (textTexture) {
                // Position text offset from cursor (below and to the right)
                int offsetX = 20;  // Offset to the right of cursor
                int offsetY = 20;  // Offset below cursor
                int bottomMargin = 60;  // Keep away from bottom edge (window bar)
                int rightMargin = 20;   // Keep away from right edge
                
                // Calculate text box dimensions
                int boxWidth = textSurface->w + 8;
                int boxHeight = textSurface->h + 6;
                
                // Start with default position (below and right of cursor)
                int textX = m_mouseX + offsetX;
                int textY = m_mouseY + offsetY;
                
                // Check if too close to right edge - flip to left
                if (textX + boxWidth + rightMargin > m_windowWidth) {
                    textX = m_mouseX - boxWidth - offsetX;
                }
                
                // Check if too close to bottom edge - flip above
                if (textY + boxHeight + bottomMargin > m_windowHeight) {
                    textY = m_mouseY - boxHeight - offsetY;
                }
                
                // Final safety check - clamp to screen bounds
                textX = std::max(5, std::min(textX, m_windowWidth - boxWidth - 5));
                textY = std::max(5, std::min(textY, m_windowHeight - boxHeight - bottomMargin));
                
                // Draw small background box
                SDL_Rect textBox = {
                    textX,
                    textY,
                    boxWidth,
                    boxHeight
                };
                
                SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 180);
                SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
                SDL_RenderFillRect(m_renderer, &textBox);
                
                // Draw text
                SDL_Rect textRect = {
                    textBox.x + 4,
                    textBox.y + 3,
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(m_renderer, textTexture, nullptr, &textRect);
                
                SDL_DestroyTexture(textTexture);
            }
            
            SDL_FreeSurface(textSurface);
        }
    }
    
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    
    // Present the rendered frame
    SDL_RenderPresent(m_renderer);
}

void Game::shutdown() {
    // Cleanup font
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    
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

    TTF_Quit();
    SDL_Quit();
    std::cout << "Game shut down successfully." << std::endl;
}

} // namespace BombingRun
