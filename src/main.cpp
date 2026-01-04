#include "core/Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;
    
    std::cout << "==================================" << std::endl;
    std::cout << "  Bombing Run - Version 1.0.0" << std::endl;
    std::cout << "  Phase 1: Core Infrastructure" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;
    
    BombingRun::Game game;
    
    // Initialize game with 800x600 window
    if (!game.initialize("Bombing Run - Phase 1", 800, 600)) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC       - Quit" << std::endl;
    std::cout << "  SPACE     - Deploy bomber" << std::endl;
    std::cout << "  8         - Deploy airstrike" << std::endl;
    std::cout << "  F         - Deploy fighter jet" << std::endl;
    std::cout << "  1-7       - Select bomb type" << std::endl;
    std::cout << "  Click     - Drop bomb at cursor" << std::endl;
    std::cout << std::endl;
    
    // Run the game loop
    game.run();
    
    return 0;
}
