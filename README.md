# Bombing Run

A 2D top-down aerial combat strategy game built with C++ and SDL2.

**View Perspective:** Bird's eye view (top-down 2D)

## Project Status
**Phase 3: Bomber Aircraft System** (In Progress)

See [Development-Phases.md](Development-Phases.md) for full development roadmap.

## Build Instructions

### Linux
```bash
# Install dependencies
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev cmake g++

# Build
mkdir build && cd build
cmake ..
make

# Run
./bin/BombingRun
```

### Windows
```bash
# Install SDL2 development libraries
# Download from: https://www.libsdl.org/download-2.0.php
# Set SDL2_DIR environment variable

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run
bin\Release\BombingRun.exe
```

## Controls
- **ESC** - Quit
- **SPACE** - Deploy bomber
- **8** - Deploy airstrike
- **1-7** - Select bomb type (100lb-8000lb)
- **Left Click** - Drop bomb at cursor

## Project Structure
```
BombingRun/
├── src/            # Source files
│   ├── core/       # Game loop, state management
│   ├── entities/   # Aircraft, weapons, buildings
│   ├── systems/    # Collision, rendering, physics
│   └── main.cpp
├── include/        # Header files
├── assets/         # Game assets
│   ├── sprites/
│   ├── audio/
│   └── textures/
└── docs/           # Documentation
```

## Documentation
- [Game Specification](Bombing%20Run%20Game%20Specification.md)
- [Development Phases](Development-Phases.md)
- [Copilot Instructions](.github/copilot-instructions.md)

## License
MIT License - See LICENSE file for details

## Development
Started: 3 January 2026  
View: Top-Down 2D (bird's eye perspective)  
Target: 60 FPS 2D aerial combat game  
Platform: Cross-platform (Linux/Windows)
