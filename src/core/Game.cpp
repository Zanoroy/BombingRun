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
    , m_airstrikeMode(false)
    , m_airstrikeTargetX(0.0f)
    , m_airstrikeTargetY(0.0f)
    , m_gameState(GameState::MENU)  // Start at menu
    , m_selectedMap("battleground")  // Default to battleground
    , m_player1Jet(nullptr)
    , m_player2Jet(nullptr)
    , m_pvpMapSize(3000)  // Large ocean map
    , m_screenShakeIntensity(0.0f)
    , m_screenShakeDuration(0.0f)
    , m_screenShakeX(0.0f)
    , m_screenShakeY(0.0f)
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

    // Initialize AudioManager
    if (!m_audioManager.initialize()) {
        std::cerr << "Warning: AudioManager initialization failed - game will run without audio" << std::endl;
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

    // Don't load map yet - wait for menu selection
    // Map will be loaded when user clicks "Play" in menu

    m_running = true;
    m_lastFrameTime = SDL_GetTicks();

    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Window: " << width << "x" << height << std::endl;
    std::cout << "Starting in menu mode - select map and press Play" << std::endl;
    
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
                // Handle keyboard input based on game state
                if (m_gameState == GameState::MENU) {
                    // Menu controls
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        m_running = false;
                    }
                } else if (m_gameState == GameState::PVP) {
                    // PVP controls - no ESC exit during match
                    // Players handle their own controls in updatePVP()
                } else if (m_gameState == GameState::PLAYING) {
                    // Bombing mode controls
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            m_running = false;
                            break;
                        case SDLK_SPACE:
                            m_aircraftManager.spawnBomber(-1.0f, -1.0f, static_cast<int>(m_selectedBombType));
                            std::cout << "Spacebar pressed - Deployed bomber with bomb type " 
                                      << static_cast<int>(m_selectedBombType) << std::endl;
                            break;
                        case SDLK_8:
                            // Activate airstrike targeting mode
                            m_airstrikeMode = true;
                            std::cout << "8 pressed - Airstrike targeting mode activated. Click to deploy." << std::endl;
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
                }
                break;
                
            case SDL_MOUSEMOTION:
                // Track mouse position
                m_mouseX = event.motion.x;
                m_mouseY = event.motion.y;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (m_gameState == GameState::MENU) {
                        // Menu click handling
                        int clickX = event.button.x;
                        int clickY = event.button.y;
                        
                        // Check map selection boxes
                        int boxWidth = 280;
                        int boxHeight = 220;
                        int spacing = 40;
                        int y = 260;  // Match renderMenu position
                        
                        // City Strike box
                        int cityX = m_windowWidth/2 - boxWidth - spacing/2;
                        SDL_Rect cityRect = {cityX, y, boxWidth, boxHeight};
                        if (isMouseOver(clickX, clickY, cityRect)) {
                            m_selectedMap = "city";
                            m_audioManager.playSound(AudioManager::SoundEffect::MENU_CLICK);
                            std::cout << "Selected City Strike map" << std::endl;
                        }
                        
                        // BattleGround box
                        int bgX = m_windowWidth/2 + spacing/2;
                        SDL_Rect bgRect = {bgX, y, boxWidth, boxHeight};
                        if (isMouseOver(clickX, clickY, bgRect)) {
                            m_selectedMap = "battleground";
                            m_audioManager.playSound(AudioManager::SoundEffect::MENU_CLICK);
                            std::cout << "Selected BattleGround map" << std::endl;
                        }
                        
                        // Start Mission button
                        int buttonY = y + boxHeight + 40;
                        int buttonWidth = 280;
                        int buttonHeight = 55;
                        int buttonSpacing = 30;
                        int startX = m_windowWidth/2 - buttonWidth - buttonSpacing/2;
                        SDL_Rect startRect = {startX, buttonY, buttonWidth, buttonHeight};
                        if (isMouseOver(clickX, clickY, startRect)) {
                            m_audioManager.playSound(AudioManager::SoundEffect::MENU_CLICK);
                            m_gameState = GameState::PLAYING;
                            
                            // Create map and load selected map name
                            m_currentMap = std::make_unique<Map>(m_windowWidth, m_windowHeight);
                            m_currentMap->loadMap(m_selectedMap);
                            
                            std::cout << "Starting mission on " << m_selectedMap << " map!" << std::endl;
                        }
                        
                        // PVP Dogfight button
                        int pvpX = m_windowWidth/2 + buttonSpacing/2;
                        SDL_Rect pvpRect = {pvpX, buttonY, buttonWidth, buttonHeight};
                        if (isMouseOver(clickX, clickY, pvpRect)) {
                            m_audioManager.playSound(AudioManager::SoundEffect::MENU_CLICK);
                            m_gameState = GameState::PVP;
                            
                            // Initialize PVP mode - spawn two player jets
                            m_player1Jet = std::make_unique<PlayerJet>(500.0f, 500.0f, PlayerJet::Player::PLAYER1);
                            m_player2Jet = std::make_unique<PlayerJet>(2500.0f, 2500.0f, PlayerJet::Player::PLAYER2);
                            m_pvpBullets.clear();
                            
                            std::cout << "Starting PVP Dogfight!" << std::endl;
                            std::cout << "Player 1: WASD + Space | Player 2: Arrows + Enter" << std::endl;
                        }
                        
                    } else if (m_gameState == GameState::PLAYING) {
                        // Bombing mode click handling
                        if (m_airstrikeMode) {
                            // Deploy airstrike at clicked position
                            float targetX = static_cast<float>(event.button.x);
                            float targetY = static_cast<float>(event.button.y);
                            m_aircraftManager.deployAirstrike(targetX, targetY);
                            m_airstrikeMode = false;  // Deactivate targeting mode
                            std::cout << "Airstrike deployed at (" << targetX << ", " << targetY << ")" << std::endl;
                        } else {
                            // Normal bomber deployment
                            float targetX = static_cast<float>(event.button.x);
                            float targetY = static_cast<float>(event.button.y);
                            
                            // Spawn bomber that will fly to target and drop bombs
                            m_aircraftManager.spawnBomber(targetX, targetY, static_cast<int>(m_selectedBombType));
                            
                            std::cout << "Mouse clicked at: (" 
                                      << event.button.x << ", " 
                                      << event.button.y << ") - Spawned bomber" << std::endl;
                        }
                    }
                }
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Update based on game state
    if (m_gameState == GameState::MENU) {
        // No updates needed in menu
        return;
    }
    
    if (m_gameState == GameState::PVP) {
        updatePVP(deltaTime);
        return;
    }
    
    // PLAYING mode update
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
            
            // Check bullet collisions with fighters (AAA guns can shoot them down)
            FighterJet* hitFighter = m_aircraftManager.checkFighterCollision(bullet->getX(), bullet->getY());
            if (hitFighter) {
                float fighterX = hitFighter->getX();
                float fighterY = hitFighter->getY();
                bool destroyed = hitFighter->takeDamage(1);  // Bullets do 1 damage
                bullet->markHit();  // Destroy the bullet
                
                // If fighter was destroyed, create explosion
                if (destroyed) {
                    std::cout << "Fighter jet destroyed! Creating explosion" << std::endl;
                    // Create explosion at fighter position (medium size - 60px radius)
                    m_explosionManager.createExplosion(fighterX, fighterY, 60, 0.8f);
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
    // Render based on game state
    if (m_gameState == GameState::MENU) {
        // Clear screen with dark background
        SDL_SetRenderDrawColor(m_renderer, 30, 35, 45, 255);
        SDL_RenderClear(m_renderer);
        renderMenu();
        SDL_RenderPresent(m_renderer);
        return;
    }
    
    if (m_gameState == GameState::PVP) {
        renderPVP();
        SDL_RenderPresent(m_renderer);
        return;
    }
    
    // PLAYING mode render
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
    
    // Draw airstrike target indicators if in airstrike mode
    if (m_airstrikeMode) {
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        
        // Calculate bomber positions (same logic as deployAirstrike)
        // Triangle formation without bottom (flipped - wide at front)
        // Planes on same row aligned horizontally
        const float spacing = 80.0f;
        const float verticalSpacing = 100.0f;
        
        float positions[5][2] = {
            {static_cast<float>(m_mouseX), m_mouseY - verticalSpacing * 2},  // Back point
            {m_mouseX - spacing, m_mouseY - verticalSpacing},                // Middle left
            {m_mouseX + spacing, m_mouseY - verticalSpacing},                // Middle right (same Y)
            {m_mouseX - spacing * 2, static_cast<float>(m_mouseY)},          // Front left
            {m_mouseX + spacing * 2, static_cast<float>(m_mouseY)}           // Front right (same Y)
        };
        
        // Draw 5 target indicators for each bomber
        for (int i = 0; i < 5; i++) {
            float targetX = positions[i][0];
            float targetY = positions[i][1];
            
            // Draw target circle (30px radius for visibility)
            SDL_SetRenderDrawColor(m_renderer, 255, 165, 0, 150);  // Orange with transparency
            int circleRadius = 30;
            int segments = 24;
            for (int j = 0; j < segments; j++) {
                float angle1 = (j * 2.0f * M_PI) / segments;
                float angle2 = ((j + 1) * 2.0f * M_PI) / segments;
                
                int x1 = static_cast<int>(targetX + circleRadius * cos(angle1));
                int y1 = static_cast<int>(targetY + circleRadius * sin(angle1));
                int x2 = static_cast<int>(targetX + circleRadius * cos(angle2));
                int y2 = static_cast<int>(targetY + circleRadius * sin(angle2));
                
                SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
            }
            
            // Draw crosshair
            SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 180);  // Red
            int crossSize = 15;
            SDL_RenderDrawLine(m_renderer, 
                static_cast<int>(targetX) - crossSize, static_cast<int>(targetY),
                static_cast<int>(targetX) + crossSize, static_cast<int>(targetY));
            SDL_RenderDrawLine(m_renderer,
                static_cast<int>(targetX), static_cast<int>(targetY) - crossSize,
                static_cast<int>(targetX), static_cast<int>(targetY) + crossSize);
        }
        
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);
    }
    
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
        std::string indicatorText;
        
        if (m_airstrikeMode) {
            indicatorText = "AIRSTRIKE (250lb x 5 planes)";
        } else {
            const char* bombNames[] = {
                "100lb",
                "250lb", 
                "500lb",
                "1000lb",
                "2000lb",
                "4000lb",
                "8000lb"
            };
            
            indicatorText = bombNames[static_cast<int>(m_selectedBombType)];
            indicatorText += " (";
            indicatorText += std::to_string(m_weaponManager.getRemainingBombs(m_selectedBombType));
            indicatorText += ")";
        }
        
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
    
    // Render HUD in playing mode
    renderHUD();
    
    // Present the rendered frame
    SDL_RenderPresent(m_renderer);
}

void Game::renderMenu() {
    if (!m_font) return;
    
    // Colors - more sophisticated palette
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color lightGray = {180, 190, 200, 255};
    SDL_Color darkGray = {60, 70, 80, 255};
    SDL_Color titleColor = {220, 220, 230, 255};
    SDL_Color accentBlue = {100, 150, 200, 255};
    
    // Draw sophisticated background gradient
    for (int i = 0; i < m_windowHeight; i++) {
        int r = 25 + (i * 15 / m_windowHeight);
        int g = 30 + (i * 25 / m_windowHeight);
        int b = 40 + (i * 40 / m_windowHeight);
        SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
        SDL_RenderDrawLine(m_renderer, 0, i, m_windowWidth, i);
    }
    
    // Draw title with subtle shadow - using blended text for smoothness
    int titleY = 60;
    
    // Title shadow
    SDL_Surface* shadowSurface = TTF_RenderText_Blended(m_font, "BOMBING RUN", darkGray);
    if (shadowSurface) {
        SDL_Texture* shadowTexture = SDL_CreateTextureFromSurface(m_renderer, shadowSurface);
        int shadowWidth = shadowSurface->w * 2.5;
        int shadowHeight = shadowSurface->h * 2.5;
        SDL_Rect shadowRect = {m_windowWidth/2 - shadowWidth/2 + 3, titleY + 3, shadowWidth, shadowHeight};
        SDL_RenderCopy(m_renderer, shadowTexture, nullptr, &shadowRect);
        SDL_DestroyTexture(shadowTexture);
        SDL_FreeSurface(shadowSurface);
    }
    
    // Main title - smooth anti-aliased text
    SDL_Surface* titleSurface = TTF_RenderText_Blended(m_font, "BOMBING RUN", titleColor);
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(m_renderer, titleSurface);
        int titleWidth = titleSurface->w * 2.5;
        int titleHeight = titleSurface->h * 2.5;
        SDL_Rect titleRect = {m_windowWidth/2 - titleWidth/2, titleY, titleWidth, titleHeight};
        SDL_RenderCopy(m_renderer, titleTexture, nullptr, &titleRect);
        SDL_DestroyTexture(titleTexture);
        SDL_FreeSurface(titleSurface);
    }
    
    // Draw main panel with subtle styling
    int boxWidth = 700;
    int boxHeight = 480;
    int boxX = m_windowWidth/2 - boxWidth/2;
    int boxY = 200;
    
    // Soft shadow
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_Rect shadowBox = {boxX + 8, boxY + 8, boxWidth, boxHeight};
    SDL_RenderFillRect(m_renderer, &shadowBox);
    
    // Panel background - darker, more professional
    SDL_SetRenderDrawColor(m_renderer, 35, 45, 55, 240);
    SDL_Rect box = {boxX, boxY, boxWidth, boxHeight};
    SDL_RenderFillRect(m_renderer, &box);
    
    // Subtle border
    SDL_SetRenderDrawColor(m_renderer, 70, 90, 110, 255);
    SDL_RenderDrawRect(m_renderer, &box);
    SDL_SetRenderDrawColor(m_renderer, 50, 70, 90, 180);
    SDL_Rect innerBorder = {boxX + 1, boxY + 1, boxWidth - 2, boxHeight - 2};
    SDL_RenderDrawRect(m_renderer, &innerBorder);
    
    // "Select Mission" title - smooth text
    int y = boxY + 30;
    SDL_Surface* sectionSurface = TTF_RenderText_Blended(m_font, "Select Mission", accentBlue);
    if (sectionSurface) {
        SDL_Texture* sectionTexture = SDL_CreateTextureFromSurface(m_renderer, sectionSurface);
        int sectionWidth = sectionSurface->w * 1.4;
        int sectionHeight = sectionSurface->h * 1.4;
        SDL_Rect sectionRect = {m_windowWidth/2 - sectionWidth/2, y, sectionWidth, sectionHeight};
        SDL_RenderCopy(m_renderer, sectionTexture, nullptr, &sectionRect);
        SDL_DestroyTexture(sectionTexture);
        SDL_FreeSurface(sectionSurface);
    }
    y += 60;
    
    // Map selection boxes with visual representations
    int mapBoxWidth = 280;
    int mapBoxHeight = 220;
    int spacing = 40;
    
    // === CITY STRIKE BOX ===
    int cityX = m_windowWidth/2 - mapBoxWidth - spacing/2;
    bool citySelected = (m_selectedMap == "city");
    SDL_Rect cityRect = {cityX, y, mapBoxWidth, mapBoxHeight};
    bool cityHovered = isMouseOver(m_mouseX, m_mouseY, cityRect);
    int cityOffset = (cityHovered ? 3 : 0);
    
    // City box shadow
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 100);
    SDL_Rect cityShadow = {cityX + 5, y + 5 - cityOffset, mapBoxWidth, mapBoxHeight};
    SDL_RenderFillRect(m_renderer, &cityShadow);
    
    // City box background - subtle gradient
    SDL_Rect cityBox = {cityX, y - cityOffset, mapBoxWidth, mapBoxHeight};
    for (int i = 0; i < mapBoxHeight; i++) {
        int shade = 45 + (i * 15 / mapBoxHeight);
        SDL_SetRenderDrawColor(m_renderer, shade, shade + 5, shade + 10, 255);
        SDL_RenderDrawLine(m_renderer, cityX, y - cityOffset + i, cityX + mapBoxWidth, y - cityOffset + i);
    }
    
    // Draw city skyline illustration
    int iconY = y - cityOffset + 20;
    int iconCenterX = cityX + mapBoxWidth/2;
    
    // Buildings with windows
    SDL_SetRenderDrawColor(m_renderer, 70, 75, 85, 255);
    SDL_Rect building1 = {iconCenterX - 80, iconY + 60, 40, 80};
    SDL_RenderFillRect(m_renderer, &building1);
    SDL_Rect building2 = {iconCenterX - 30, iconY + 30, 50, 110};
    SDL_RenderFillRect(m_renderer, &building2);
    SDL_Rect building3 = {iconCenterX + 30, iconY + 50, 45, 90};
    SDL_RenderFillRect(m_renderer, &building3);
    
    // Windows on buildings
    SDL_SetRenderDrawColor(m_renderer, 200, 180, 100, 255);
    for (int wx = 0; wx < 3; wx++) {
        for (int wy = 0; wy < 6; wy++) {
            SDL_Rect window = {iconCenterX - 25 + wx*15, iconY + 40 + wy*15, 8, 10};
            SDL_RenderFillRect(m_renderer, &window);
        }
    }
    
    // Small windows on side buildings
    for (int i = 0; i < 5; i++) {
        SDL_Rect w1 = {iconCenterX - 72, iconY + 70 + i*14, 6, 8};
        SDL_Rect w2 = {iconCenterX + 40, iconY + 60 + i*14, 6, 8};
        SDL_RenderFillRect(m_renderer, &w1);
        SDL_RenderFillRect(m_renderer, &w2);
    }
    
    // City box border
    if (citySelected) {
        SDL_SetRenderDrawColor(m_renderer, 120, 180, 220, 255);
        for (int i = 0; i < 3; i++) {
            SDL_Rect cityBorder = {cityX - i - 1, y - cityOffset - i - 1, mapBoxWidth + (i+1)*2, mapBoxHeight + (i+1)*2};
            SDL_RenderDrawRect(m_renderer, &cityBorder);
        }
    } else {
        SDL_SetRenderDrawColor(m_renderer, 80, 90, 100, 255);
        SDL_RenderDrawRect(m_renderer, &cityBox);
    }
    
    // City text label - smooth
    SDL_Surface* citySurface = TTF_RenderText_Blended(m_font, "City Strike", white);
    if (citySurface) {
        SDL_Texture* cityTexture = SDL_CreateTextureFromSurface(m_renderer, citySurface);
        int cityTextWidth = citySurface->w * 1.2;
        int cityTextHeight = citySurface->h * 1.2;
        SDL_Rect cityTextRect = {cityX + mapBoxWidth/2 - cityTextWidth/2, y + mapBoxHeight - 35 - cityOffset, cityTextWidth, cityTextHeight};
        SDL_RenderCopy(m_renderer, cityTexture, nullptr, &cityTextRect);
        SDL_DestroyTexture(cityTexture);
        SDL_FreeSurface(citySurface);
    }
    
    // City description
    SDL_Surface* cityDescSurface = TTF_RenderText_Blended(m_font, "Urban Targets", lightGray);
    if (cityDescSurface) {
        SDL_Texture* cityDescTexture = SDL_CreateTextureFromSurface(m_renderer, cityDescSurface);
        SDL_Rect cityDescRect = {cityX + mapBoxWidth/2 - cityDescSurface->w/2, y + mapBoxHeight - 18 - cityOffset, cityDescSurface->w, cityDescSurface->h};
        SDL_RenderCopy(m_renderer, cityDescTexture, nullptr, &cityDescRect);
        SDL_DestroyTexture(cityDescTexture);
        SDL_FreeSurface(cityDescSurface);
    }
    
    // === BATTLEGROUND BOX ===
    int bgX = m_windowWidth/2 + spacing/2;
    bool bgSelected = (m_selectedMap == "battleground");
    SDL_Rect bgRect = {bgX, y, mapBoxWidth, mapBoxHeight};
    bool bgHovered = isMouseOver(m_mouseX, m_mouseY, bgRect);
    int bgOffset = (bgHovered ? 3 : 0);
    
    // BG box shadow
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 100);
    SDL_Rect bgShadow = {bgX + 5, y + 5 - bgOffset, mapBoxWidth, mapBoxHeight};
    SDL_RenderFillRect(m_renderer, &bgShadow);
    
    // BG box background - subtle gradient
    SDL_Rect bgBox = {bgX, y - bgOffset, mapBoxWidth, mapBoxHeight};
    for (int i = 0; i < mapBoxHeight; i++) {
        int shade = 45 + (i * 15 / mapBoxHeight);
        SDL_SetRenderDrawColor(m_renderer, shade, shade + 5, shade + 10, 255);
        SDL_RenderDrawLine(m_renderer, bgX, y - bgOffset + i, bgX + mapBoxWidth, y - bgOffset + i);
    }
    
    // Draw military base illustration
    int baseY = y - bgOffset + 30;
    int baseCenterX = bgX + mapBoxWidth/2;
    
    // Runway
    SDL_SetRenderDrawColor(m_renderer, 60, 65, 70, 255);
    SDL_Rect runway = {baseCenterX - 90, baseY + 90, 180, 50};
    SDL_RenderFillRect(m_renderer, &runway);
    
    // Runway markings
    SDL_SetRenderDrawColor(m_renderer, 200, 200, 200, 255);
    for (int i = 0; i < 6; i++) {
        SDL_Rect marking = {baseCenterX - 80 + i*30, baseY + 110, 20, 8};
        SDL_RenderFillRect(m_renderer, &marking);
    }
    
    // Hangar buildings
    SDL_SetRenderDrawColor(m_renderer, 75, 80, 70, 255);
    SDL_Rect hangar1 = {baseCenterX - 70, baseY + 40, 50, 45};
    SDL_Rect hangar2 = {baseCenterX + 20, baseY + 40, 50, 45};
    SDL_RenderFillRect(m_renderer, &hangar1);
    SDL_RenderFillRect(m_renderer, &hangar2);
    
    // Hangar roofs
    SDL_SetRenderDrawColor(m_renderer, 90, 85, 75, 255);
    SDL_Point roof1[] = {
        {baseCenterX - 75, baseY + 40},
        {baseCenterX - 45, baseY + 25},
        {baseCenterX - 15, baseY + 40}
    };
    SDL_Point roof2[] = {
        {baseCenterX + 15, baseY + 40},
        {baseCenterX + 45, baseY + 25},
        {baseCenterX + 75, baseY + 40}
    };
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawLine(m_renderer, roof1[i].x, roof1[i].y, roof1[(i+1)%3].x, roof1[(i+1)%3].y);
        SDL_RenderDrawLine(m_renderer, roof2[i].x, roof2[i].y, roof2[(i+1)%3].x, roof2[(i+1)%3].y);
    }
    
    // Control tower
    SDL_SetRenderDrawColor(m_renderer, 70, 75, 80, 255);
    SDL_Rect tower = {baseCenterX - 10, baseY + 50, 20, 40};
    SDL_RenderFillRect(m_renderer, &tower);
    SDL_Rect towerTop = {baseCenterX - 18, baseY + 45, 36, 15};
    SDL_RenderFillRect(m_renderer, &towerTop);
    
    // Fighter jet icon on runway
    SDL_SetRenderDrawColor(m_renderer, 90, 100, 110, 255);
    SDL_Rect jetBody = {baseCenterX + 30, baseY + 105, 25, 8};
    SDL_Rect jetWing = {baseCenterX + 32, baseY + 108, 20, 2};
    SDL_RenderFillRect(m_renderer, &jetBody);
    SDL_RenderFillRect(m_renderer, &jetWing);
    
    // BG box border
    if (bgSelected) {
        SDL_SetRenderDrawColor(m_renderer, 120, 180, 220, 255);
        for (int i = 0; i < 3; i++) {
            SDL_Rect bgBorder = {bgX - i - 1, y - bgOffset - i - 1, mapBoxWidth + (i+1)*2, mapBoxHeight + (i+1)*2};
            SDL_RenderDrawRect(m_renderer, &bgBorder);
        }
    } else {
        SDL_SetRenderDrawColor(m_renderer, 80, 90, 100, 255);
        SDL_RenderDrawRect(m_renderer, &bgBox);
    }
    
    // BG text label - smooth
    SDL_Surface* bgSurface = TTF_RenderText_Blended(m_font, "BattleGround", white);
    if (bgSurface) {
        SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(m_renderer, bgSurface);
        int bgTextWidth = bgSurface->w * 1.2;
        int bgTextHeight = bgSurface->h * 1.2;
        SDL_Rect bgTextRect = {bgX + mapBoxWidth/2 - bgTextWidth/2, y + mapBoxHeight - 35 - bgOffset, bgTextWidth, bgTextHeight};
        SDL_RenderCopy(m_renderer, bgTexture, nullptr, &bgTextRect);
        SDL_DestroyTexture(bgTexture);
        SDL_FreeSurface(bgSurface);
    }
    
    // BG description
    SDL_Surface* bgDescSurface = TTF_RenderText_Blended(m_font, "Military Base", lightGray);
    if (bgDescSurface) {
        SDL_Texture* bgDescTexture = SDL_CreateTextureFromSurface(m_renderer, bgDescSurface);
        SDL_Rect bgDescRect = {bgX + mapBoxWidth/2 - bgDescSurface->w/2, y + mapBoxHeight - 18 - bgOffset, bgDescSurface->w, bgDescSurface->h};
        SDL_RenderCopy(m_renderer, bgDescTexture, nullptr, &bgDescRect);
        SDL_DestroyTexture(bgDescTexture);
        SDL_FreeSurface(bgDescSurface);
    }
    
    y += mapBoxHeight + 40;
    
    // Two buttons side by side
    int buttonWidth = 280;
    int buttonHeight = 55;
    int buttonSpacing = 30;
    
    // START MISSION button - sleek design
    int startX = m_windowWidth/2 - buttonWidth - buttonSpacing/2;
    SDL_Rect startRect = {startX, y, buttonWidth, buttonHeight};
    bool startHovered = isMouseOver(m_mouseX, m_mouseY, startRect);
    int startOffset = (startHovered ? 3 : 0);
    
    // Button shadow
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_Rect startShadow = {startX + 5, y + 5 - startOffset, buttonWidth, buttonHeight};
    SDL_RenderFillRect(m_renderer, &startShadow);
    
    // Button gradient background
    SDL_Rect startBox = {startX, y - startOffset, buttonWidth, buttonHeight};
    for (int i = 0; i < buttonHeight; i++) {
        int r = 100 + (i * 40 / buttonHeight);
        int g = 140 + (i * 30 / buttonHeight);
        int b = 180 + (i * 20 / buttonHeight);
        if (startHovered) {
            r += 20;
            g += 20;
            b += 20;
        }
        SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
        SDL_RenderDrawLine(m_renderer, startX, y - startOffset + i, startX + buttonWidth, y - startOffset + i);
    }
    
    // Button border
    SDL_SetRenderDrawColor(m_renderer, 120, 160, 200, 255);
    SDL_RenderDrawRect(m_renderer, &startBox);
    SDL_SetRenderDrawColor(m_renderer, 140, 180, 220, 180);
    SDL_Rect innerStartBorder = {startX + 1, y - startOffset + 1, buttonWidth - 2, buttonHeight - 2};
    SDL_RenderDrawRect(m_renderer, &innerStartBorder);
    
    // Button text - smooth
    SDL_Surface* startSurface = TTF_RenderText_Blended(m_font, "START MISSION", white);
    if (startSurface) {
        SDL_Texture* startTexture = SDL_CreateTextureFromSurface(m_renderer, startSurface);
        int startTextWidth = startSurface->w * 1.3;
        int startTextHeight = startSurface->h * 1.3;
        SDL_Rect startTextRect = {startX + buttonWidth/2 - startTextWidth/2, y + buttonHeight/2 - startTextHeight/2 - startOffset, startTextWidth, startTextHeight};
        SDL_RenderCopy(m_renderer, startTexture, nullptr, &startTextRect);
        SDL_DestroyTexture(startTexture);
        SDL_FreeSurface(startSurface);
    }
    
    // PVP DOGFIGHT button
    int pvpX = m_windowWidth/2 + buttonSpacing/2;
    SDL_Rect pvpRect = {pvpX, y, buttonWidth, buttonHeight};
    bool pvpHovered = isMouseOver(m_mouseX, m_mouseY, pvpRect);
    int pvpOffset = (pvpHovered ? 3 : 0);
    
    // PVP Button shadow
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 120);
    SDL_Rect pvpShadow = {pvpX + 5, y + 5 - pvpOffset, buttonWidth, buttonHeight};
    SDL_RenderFillRect(m_renderer, &pvpShadow);
    
    // PVP Button gradient (red theme)
    SDL_Rect pvpBox = {pvpX, y - pvpOffset, buttonWidth, buttonHeight};
    for (int i = 0; i < buttonHeight; i++) {
        int r = 150 + (i * 30 / buttonHeight);
        int g = 50 + (i * 40 / buttonHeight);
        int b = 50 + (i * 30 / buttonHeight);
        if (pvpHovered) {
            r += 20;
            g += 20;
            b += 20;
        }
        SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
        SDL_RenderDrawLine(m_renderer, pvpX, y - pvpOffset + i, pvpX + buttonWidth, y - pvpOffset + i);
    }
    
    // PVP Button border
    SDL_SetRenderDrawColor(m_renderer, 200, 100, 100, 255);
    SDL_RenderDrawRect(m_renderer, &pvpBox);
    SDL_SetRenderDrawColor(m_renderer, 220, 120, 120, 180);
    SDL_Rect innerPvpBorder = {pvpX + 1, y - pvpOffset + 1, buttonWidth - 2, buttonHeight - 2};
    SDL_RenderDrawRect(m_renderer, &innerPvpBorder);
    
    // PVP Button text
    SDL_Surface* pvpSurface = TTF_RenderText_Blended(m_font, "PVP DOGFIGHT", white);
    if (pvpSurface) {
        SDL_Texture* pvpTexture = SDL_CreateTextureFromSurface(m_renderer, pvpSurface);
        int pvpTextWidth = pvpSurface->w * 1.3;
        int pvpTextHeight = pvpSurface->h * 1.3;
        SDL_Rect pvpTextRect = {pvpX + buttonWidth/2 - pvpTextWidth/2, y + buttonHeight/2 - pvpTextHeight/2 - pvpOffset, pvpTextWidth, pvpTextHeight};
        SDL_RenderCopy(m_renderer, pvpTexture, nullptr, &pvpTextRect);
        SDL_DestroyTexture(pvpTexture);
        SDL_FreeSurface(pvpSurface);
    }
    
    y += buttonHeight + 25;
    
    // ESC hint
    SDL_Surface* escSurface = TTF_RenderText_Blended(m_font, "ESC - Quit", lightGray);
    if (escSurface) {
        SDL_Texture* escTexture = SDL_CreateTextureFromSurface(m_renderer, escSurface);
        SDL_Rect escRect = {m_windowWidth/2 - escSurface->w/2, y, escSurface->w, escSurface->h};
        SDL_RenderCopy(m_renderer, escTexture, nullptr, &escRect);
        SDL_DestroyTexture(escTexture);
        SDL_FreeSurface(escSurface);
    }
    
    // Version info
    SDL_Surface* versionSurface = TTF_RenderText_Blended(m_font, "v1.0.0 - Phase 9", darkGray);
    if (versionSurface) {
        SDL_Texture* versionTexture = SDL_CreateTextureFromSurface(m_renderer, versionSurface);
        SDL_Rect versionRect = {m_windowWidth - versionSurface->w - 20, m_windowHeight - 30, versionSurface->w, versionSurface->h};
        SDL_RenderCopy(m_renderer, versionTexture, nullptr, &versionRect);
        SDL_DestroyTexture(versionTexture);
        SDL_FreeSurface(versionSurface);
    }
}

bool Game::isMouseOver(int mouseX, int mouseY, const SDL_Rect& rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}

void Game::renderPVP() {
    // Draw ocean background
    SDL_SetRenderDrawColor(m_renderer, 20, 80, 150, 255);
    SDL_RenderClear(m_renderer);
    
    // Draw water pattern (simple waves)
    SDL_SetRenderDrawColor(m_renderer, 30, 90, 160, 255);
    for (int y = 0; y < m_windowHeight; y += 40) {
        for (int x = 0; x < m_windowWidth; x += 60) {
            SDL_Rect wave = {x, y, 40, 3};
            SDL_RenderFillRect(m_renderer, &wave);
        }
    }
    
    // Calculate camera center (midpoint between two players)
    float cameraX = m_windowWidth / 2.0f;
    float cameraY = m_windowHeight / 2.0f;
    
    if (m_player1Jet && m_player2Jet) {
        float midX = (m_player1Jet->getX() + m_player2Jet->getX()) / 2.0f;
        float midY = (m_player1Jet->getY() + m_player2Jet->getY()) / 2.0f;
        
        // Smooth camera follow
        cameraX = midX;
        cameraY = midY;
    }
    
    // Render player jets
    if (m_player1Jet && m_player1Jet->isActive()) {
        m_player1Jet->render(m_renderer, cameraX - m_windowWidth/2, cameraY - m_windowHeight/2);
    }
    
    if (m_player2Jet && m_player2Jet->isActive()) {
        m_player2Jet->render(m_renderer, cameraX - m_windowWidth/2, cameraY - m_windowHeight/2);
    }
    
    // Render bullets
    for (const auto& bullet : m_pvpBullets) {
        if (bullet && bullet->isActive()) {
            float screenX = bullet->getX() - (cameraX - m_windowWidth/2);
            float screenY = bullet->getY() - (cameraY - m_windowHeight/2);
            
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 255);
            SDL_Rect bulletRect = {
                static_cast<int>(screenX) - 2,
                static_cast<int>(screenY) - 2,
                4, 4
            };
            SDL_RenderFillRect(m_renderer, &bulletRect);
        }
    }
    
    // Render HUD
    if (m_font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color red = {255, 0, 0, 255};
        SDL_Color blue = {0, 150, 255, 255};
        
        // Player 1 health (top left)
        if (m_player1Jet) {
            std::string p1Text = "Player 1 HP: " + std::to_string(m_player1Jet->getHealth());
            SDL_Surface* p1Surface = TTF_RenderText_Solid(m_font, p1Text.c_str(), red);
            if (p1Surface) {
                SDL_Texture* p1Texture = SDL_CreateTextureFromSurface(m_renderer, p1Surface);
                SDL_Rect p1Rect = {20, 20, p1Surface->w * 2, p1Surface->h * 2};
                SDL_RenderCopy(m_renderer, p1Texture, nullptr, &p1Rect);
                SDL_DestroyTexture(p1Texture);
                SDL_FreeSurface(p1Surface);
            }
        }
        
        // Player 2 health (top right)
        if (m_player2Jet) {
            std::string p2Text = "Player 2 HP: " + std::to_string(m_player2Jet->getHealth());
            SDL_Surface* p2Surface = TTF_RenderText_Solid(m_font, p2Text.c_str(), blue);
            if (p2Surface) {
                SDL_Texture* p2Texture = SDL_CreateTextureFromSurface(m_renderer, p2Surface);
                int textWidth = p2Surface->w * 2;
                SDL_Rect p2Rect = {m_windowWidth - textWidth - 20, 20, textWidth, p2Surface->h * 2};
                SDL_RenderCopy(m_renderer, p2Texture, nullptr, &p2Rect);
                SDL_DestroyTexture(p2Texture);
                SDL_FreeSurface(p2Surface);
            }
        }
    }
}

void Game::updatePVP(float deltaTime) {
    if (!m_player1Jet || !m_player2Jet) return;
    
    // Get keyboard state for continuous input
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    
    // Player 1 controls (WASD + Space)
    if (m_player1Jet->isActive()) {
        if (keys[SDL_SCANCODE_W]) m_player1Jet->increaseThrottle();
        if (keys[SDL_SCANCODE_S]) m_player1Jet->decreaseThrottle();
        if (keys[SDL_SCANCODE_A]) m_player1Jet->rotateLeft(deltaTime);
        if (keys[SDL_SCANCODE_D]) m_player1Jet->rotateRight(deltaTime);
        if (keys[SDL_SCANCODE_SPACE]) {
            m_player1Jet->shoot();
            // Create bullet - calculate target position ahead of jet
            float bulletX = m_player1Jet->getX();
            float bulletY = m_player1Jet->getY();
            float bulletAngle = m_player1Jet->getAngle();
            float targetX = bulletX + std::cos(bulletAngle) * 1000.0f;
            float targetY = bulletY + std::sin(bulletAngle) * 1000.0f;
            
            auto bullet = std::make_unique<Bullet>(bulletX, bulletY, targetX, targetY);
            m_pvpBullets.push_back(std::move(bullet));
        }
        
        m_player1Jet->update(deltaTime);
        
        // Wrap around screen edges (manually update position via reset)
        float p1X = m_player1Jet->getX();
        float p1Y = m_player1Jet->getY();
        if (p1X < 0) m_player1Jet->reset(m_pvpMapSize, p1Y);
        if (p1X > m_pvpMapSize) m_player1Jet->reset(0, p1Y);
        if (p1Y < 0) m_player1Jet->reset(p1X, m_pvpMapSize);
        if (p1Y > m_pvpMapSize) m_player1Jet->reset(p1X, 0);
    }
    
    // Player 2 controls (Arrow keys + Enter)
    if (m_player2Jet->isActive()) {
        if (keys[SDL_SCANCODE_UP]) m_player2Jet->increaseThrottle();
        if (keys[SDL_SCANCODE_DOWN]) m_player2Jet->decreaseThrottle();
        if (keys[SDL_SCANCODE_LEFT]) m_player2Jet->rotateLeft(deltaTime);
        if (keys[SDL_SCANCODE_RIGHT]) m_player2Jet->rotateRight(deltaTime);
        if (keys[SDL_SCANCODE_RETURN]) {
            m_player2Jet->shoot();
            // Create bullet - calculate target position ahead of jet
            float bulletX = m_player2Jet->getX();
            float bulletY = m_player2Jet->getY();
            float bulletAngle = m_player2Jet->getAngle();
            float targetX = bulletX + std::cos(bulletAngle) * 1000.0f;
            float targetY = bulletY + std::sin(bulletAngle) * 1000.0f;
            
            auto bullet = std::make_unique<Bullet>(bulletX, bulletY, targetX, targetY);
            m_pvpBullets.push_back(std::move(bullet));
        }
        
        m_player2Jet->update(deltaTime);
        
        // Wrap around screen edges (manually update position via reset)
        float p2X = m_player2Jet->getX();
        float p2Y = m_player2Jet->getY();
        if (p2X < 0) m_player2Jet->reset(m_pvpMapSize, p2Y);
        if (p2X > m_pvpMapSize) m_player2Jet->reset(0, p2Y);
        if (p2Y < 0) m_player2Jet->reset(p2X, m_pvpMapSize);
        if (p2Y > m_pvpMapSize) m_player2Jet->reset(p2X, 0);
    }
    
    // Update bullets
    for (auto& bullet : m_pvpBullets) {
        if (bullet && bullet->isActive()) {
            bullet->update(deltaTime);
            
            // Check collision with both players (no owner tracking for now)
            
            // Check Player 1 hit
            if (m_player1Jet->isActive()) {
                float dx = bullet->getX() - m_player1Jet->getX();
                float dy = bullet->getY() - m_player1Jet->getY();
                float dist = std::sqrt(dx*dx + dy*dy);
                
                if (dist < 30.0f) {  // Hit radius
                    m_player1Jet->takeDamage(1);
                    bullet->markHit();
                    m_audioManager.playSound(AudioManager::SoundEffect::EXPLOSION_SMALL);
                    
                    if (!m_player1Jet->isActive()) {
                        std::cout << "Player 2 WINS!" << std::endl;
                        renderVictory(2);
                        SDL_Delay(3000);
                        m_running = false;
                    }
                }
            }
            
            // Check Player 2 hit
            if (m_player2Jet->isActive()) {
                float dx = bullet->getX() - m_player2Jet->getX();
                float dy = bullet->getY() - m_player2Jet->getY();
                float dist = std::sqrt(dx*dx + dy*dy);
                
                if (dist < 30.0f) {  // Hit radius
                    m_player2Jet->takeDamage(1);
                    bullet->markHit();
                    m_audioManager.playSound(AudioManager::SoundEffect::EXPLOSION_SMALL);
                    
                    if (!m_player2Jet->isActive()) {
                        std::cout << "Player 1 WINS!" << std::endl;
                        renderVictory(1);
                        SDL_Delay(3000);
                        m_running = false;
                    }
                }
            }
            
            // Remove bullets that go off map
            if (bullet->getX() < -100 || bullet->getX() > m_pvpMapSize + 100 ||
                bullet->getY() < -100 || bullet->getY() > m_pvpMapSize + 100) {
                bullet->markHit();
            }
        }
    }
    
    // Clean up inactive bullets
    m_pvpBullets.erase(
        std::remove_if(m_pvpBullets.begin(), m_pvpBullets.end(),
            [](const std::unique_ptr<Bullet>& b) { return !b || !b->isActive(); }),
        m_pvpBullets.end()
    );
    
    // Check for jet-to-jet collision
    if (m_player1Jet->isActive() && m_player2Jet->isActive()) {
        float dx = m_player1Jet->getX() - m_player2Jet->getX();
        float dy = m_player1Jet->getY() - m_player2Jet->getY();
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist < 40.0f) {  // Collision radius
            m_player1Jet->takeDamage(3);
            m_player2Jet->takeDamage(3);
            m_audioManager.playSound(AudioManager::SoundEffect::EXPLOSION_LARGE);
            
            // Push jets apart (manually set positions)
            float pushAngle = std::atan2(dy, dx);
            float p1NewX = m_player1Jet->getX() + std::cos(pushAngle) * 50;
            float p1NewY = m_player1Jet->getY() + std::sin(pushAngle) * 50;
            float p2NewX = m_player2Jet->getX() - std::cos(pushAngle) * 50;
            float p2NewY = m_player2Jet->getY() - std::sin(pushAngle) * 50;
            
            // Can't use setPosition directly, would need to use reset or modify internal state
            // Skip pushing for now - jets will naturally separate
            
            // Check if either died from collision
            if (!m_player1Jet->isActive() && !m_player2Jet->isActive()) {
                std::cout << "DRAW! Both players destroyed!" << std::endl;
                renderDefeat(0);
                SDL_Delay(3000);
                m_running = false;
            } else if (!m_player1Jet->isActive()) {
                std::cout << "Player 2 WINS!" << std::endl;
                renderVictory(2);
                SDL_Delay(3000);
                m_running = false;
            } else if (!m_player2Jet->isActive()) {
                std::cout << "Player 1 WINS!" << std::endl;
                renderVictory(1);
                SDL_Delay(3000);
                m_running = false;
            }
        }
    }
}

void Game::renderVictory(int winnerID) {
    if (!m_font) return;
    
    // Draw semi-transparent overlay
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 200);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlay = {0, 0, m_windowWidth, m_windowHeight};
    SDL_RenderFillRect(m_renderer, &overlay);
    
    SDL_Color winnerColor = (winnerID == 1) ? SDL_Color{255, 0, 0, 255} : SDL_Color{0, 150, 255, 255};
    SDL_Color white = {255, 255, 255, 255};
    
    // Victory text
    std::string victoryText = "PLAYER " + std::to_string(winnerID) + " WINS!";
    SDL_Surface* victorySurface = TTF_RenderText_Blended(m_font, victoryText.c_str(), winnerColor);
    if (victorySurface) {
        SDL_Texture* victoryTexture = SDL_CreateTextureFromSurface(m_renderer, victorySurface);
        int textWidth = victorySurface->w * 3;
        int textHeight = victorySurface->h * 3;
        SDL_Rect victoryRect = {
            m_windowWidth/2 - textWidth/2,
            m_windowHeight/2 - textHeight/2,
            textWidth, textHeight
        };
        SDL_RenderCopy(m_renderer, victoryTexture, nullptr, &victoryRect);
        SDL_DestroyTexture(victoryTexture);
        SDL_FreeSurface(victorySurface);
    }
    
    SDL_RenderPresent(m_renderer);
}

void Game::renderDefeat(int drawFlag) {
    if (!m_font) return;
    
    // Draw semi-transparent overlay
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 200);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect overlay = {0, 0, m_windowWidth, m_windowHeight};
    SDL_RenderFillRect(m_renderer, &overlay);
    
    SDL_Color gray = {150, 150, 150, 255};
    
    // Draw text
    std::string defeatText = "DRAW - BOTH DESTROYED";
    SDL_Surface* defeatSurface = TTF_RenderText_Blended(m_font, defeatText.c_str(), gray);
    if (defeatSurface) {
        SDL_Texture* defeatTexture = SDL_CreateTextureFromSurface(m_renderer, defeatSurface);
        int textWidth = defeatSurface->w * 3;
        int textHeight = defeatSurface->h * 3;
        SDL_Rect defeatRect = {
            m_windowWidth/2 - textWidth/2,
            m_windowHeight/2 - textHeight/2,
            textWidth, textHeight
        };
        SDL_RenderCopy(m_renderer, defeatTexture, nullptr, &defeatRect);
        SDL_DestroyTexture(defeatTexture);
        SDL_FreeSurface(defeatSurface);
    }
    
    SDL_RenderPresent(m_renderer);
}


void Game::renderHUD() {
    if (!m_font || !m_currentMap) return;
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color red = {255, 0, 0, 255};
    
    // Bomb selector - top left
    int y = 10;
    for (int i = 0; i < 7; i++) {
        BombType bombType = static_cast<BombType>(i);
        int remaining = m_weaponManager.getRemainingBombs(bombType);
        
        std::string bombName;
        switch (bombType) {
            case BombType::BOMB_100LB: bombName = "1:100lb"; break;
            case BombType::BOMB_250LB: bombName = "2:250lb"; break;
            case BombType::BOMB_500LB: bombName = "3:500lb"; break;
            case BombType::BOMB_1000LB: bombName = "4:1000lb"; break;
            case BombType::BOMB_2000LB: bombName = "5:2000lb"; break;
            case BombType::BOMB_4000LB: bombName = "6:4000lb"; break;
            case BombType::BOMB_8000LB: bombName = "7:8000lb"; break;
        }
        
        std::string text = bombName + " x" + std::to_string(remaining);
        SDL_Color color = (bombType == m_selectedBombType) ? green : white;
        
        SDL_Surface* surface = TTF_RenderText_Solid(m_font, text.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
            SDL_Rect rect = {10, y, 150, 20};
            SDL_RenderCopy(m_renderer, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
        y += 25;
    }
    
    // Airstrike count
    std::string airstrikeText = "8:Airstrike x15";
    SDL_Surface* airstrikeSurface = TTF_RenderText_Solid(m_font, airstrikeText.c_str(), yellow);
    if (airstrikeSurface) {
        SDL_Texture* airstrikeTexture = SDL_CreateTextureFromSurface(m_renderer, airstrikeSurface);
        SDL_Rect airstrikeRect = {10, y, 150, 20};
        SDL_RenderCopy(m_renderer, airstrikeTexture, nullptr, &airstrikeRect);
        SDL_DestroyTexture(airstrikeTexture);
        SDL_FreeSurface(airstrikeSurface);
    }
    y += 30;
    
    // Aircraft info
    int bombers = m_aircraftManager.getActiveBomberCount();
    int fighters = m_aircraftManager.getActiveFighterCount();
    std::string aircraftText = "Bombers: " + std::to_string(bombers) + " | Fighters: " + std::to_string(fighters);
    SDL_Surface* aircraftSurface = TTF_RenderText_Solid(m_font, aircraftText.c_str(), white);
    if (aircraftSurface) {
        SDL_Texture* aircraftTexture = SDL_CreateTextureFromSurface(m_renderer, aircraftSurface);
        SDL_Rect aircraftRect = {10, y, 200, 20};
        SDL_RenderCopy(m_renderer, aircraftTexture, nullptr, &aircraftRect);
        SDL_DestroyTexture(aircraftTexture);
        SDL_FreeSurface(aircraftSurface);
    }
    
    // Runway health bar (bottom center)
    if (m_currentMap->getRunway()) {
        Runway* runway = m_currentMap->getRunway();
        int health = runway->getHealth();
        int maxHealth = runway->getMaxHealth();
        float healthPercent = static_cast<float>(health) / static_cast<float>(maxHealth);
        
        // Bar dimensions
        int barWidth = 400;
        int barHeight = 30;
        int barX = m_windowWidth / 2 - barWidth / 2;
        int barY = m_windowHeight - 50;
        
        // Background
        SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 255);
        SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
        SDL_RenderFillRect(m_renderer, &bgRect);
        
        // Health fill
        SDL_Color barColor = healthPercent > 0.6f ? green :
                            healthPercent > 0.3f ? yellow : red;
        SDL_SetRenderDrawColor(m_renderer, barColor.r, barColor.g, barColor.b, 255);
        SDL_Rect healthRect = {barX + 2, barY + 2, 
                              static_cast<int>((barWidth - 4) * healthPercent), 
                              barHeight - 4};
        SDL_RenderFillRect(m_renderer, &healthRect);
        
        // Text
        std::string healthText = "Runway: " + std::to_string(health) + "/" + std::to_string(maxHealth);
        SDL_Surface* healthSurface = TTF_RenderText_Solid(m_font, healthText.c_str(), white);
        if (healthSurface) {
            SDL_Texture* healthTexture = SDL_CreateTextureFromSurface(m_renderer, healthSurface);
            SDL_Rect textRect = {barX + barWidth/2 - 80, barY + 5, 160, 20};
            SDL_RenderCopy(m_renderer, healthTexture, nullptr, &textRect);
            SDL_DestroyTexture(healthTexture);
            SDL_FreeSurface(healthSurface);
        }
    }
}

void Game::triggerScreenShake(float intensity, float duration) {
    m_screenShakeIntensity = intensity;
    m_screenShakeDuration = duration;
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
