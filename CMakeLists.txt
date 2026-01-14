cmake_minimum_required(VERSION 3.15)
project(BombingRun VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find SDL2 packages
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Include directories
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/GameState.cpp
    src/entities/GameObject.cpp
    src/entities/Bomber.cpp
    src/entities/Bomb.cpp
    src/entities/Bullet.cpp
    src/entities/FighterJet.cpp
    src/entities/AAAGun.cpp
    src/entities/PlayerJet.cpp
    src/entities/PlayerControlledJet.cpp
    src/entities/Player2Jet.cpp
    src/entities/Building.cpp
    src/entities/CivilianBuilding.cpp
    src/entities/MilitaryBuilding.cpp
    src/entities/Runway.cpp
    src/systems/TextureManager.cpp
    src/systems/CollisionDetector.cpp
    src/systems/AircraftManager.cpp
    src/systems/WeaponManager.cpp
    src/systems/ExplosionManager.cpp
    src/systems/BuildingManager.cpp
    src/systems/AudioManager.cpp
    src/maps/Map.cpp
    src/utils/PerformanceMonitor.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets DESTINATION bin)
