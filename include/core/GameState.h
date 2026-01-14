#pragma once

namespace BombingRun {

/**
 * @brief Enumeration of possible game states
 */
enum class GameState {
    MENU,       // Main menu with map selection
    PLAYING,    // Active gameplay (bombing missions)
    PVP,        // PVP dogfight mode
    JET_FIGHTS, // Player-controlled jet mode
    PAUSED,     // Game paused
    GAME_OVER   // Game ended (victory or defeat)
};

} // namespace BombingRun
