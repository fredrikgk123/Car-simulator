# Bilsimulator

**Kandidatnummer:** [LEGG INN DITT KANDIDATNUMMER HER]

## Introduction

Bilsimulator is a 3D vehicle simulator built with modern C++20 and advanced graphics rendering. The application demonstrates arcade-style vehicle physics, dynamic audio feedback, and a modular software architecture designed for extensibility and maintainability. 

Players control a vehicle in a 3D environment with speed-sensitive steering, drift mechanics, and nitrous boost pickups. The simulation features real-time collision detection, dynamic camera systems, and responsive audio that adjusts to vehicle speed and state. The project emphasizes clean code architecture with separation of concerns, comprehensive unit testing, and adherence to modern C++ best practices.

### Key Features
- Arcade-style vehicle physics with velocity-based movement
- Speed-sensitive steering for authentic driving feel
- Drift mechanics with angle accumulation and decay
- Nitrous boost powerup system
- Dynamic camera with follow and hood-cam modes
- 2D AABB collision detection for powerup collection
- Speed-reactive audio engine with pitch and volume modulation
- Custom 7-segment digital speedometer (0-150 km/h)
- Top-down minimap for spatial awareness
- Real-time shadow rendering

### Controls
- **W/↑**: Accelerate forward
- **S/↓**: Brake/Reverse
- **A/←**: Turn left
- **D/→**: Turn right
- **Space**: Drift
- **F**: Activate nitrous (when available)
- **C**: Toggle camera mode
- **R**: Reset vehicle

## Technical Stack

- **Language**: C++20
- **Graphics**: threepp (C++ port of three.js) [1]
- **Audio**: miniaudio [2]
- **Testing**: Catch2 v3 [3]
- **Build System**: CMake

## References

### Primary Libraries & Frameworks

[1] Hjermann, M. (2024). *threepp: C++20 port of three.js (r129)*. GitHub. https://github.com/markaren/threepp

[2] Mackron, D. (2024). *miniaudio: Single file audio playback and capture library*. GitHub. https://github.com/mackron/miniaudio

[3] Catch2 Contributors. (2024). *Catch2 v3: Modern C++ test framework for unit-tests, TDD and BDD*. GitHub. https://github.com/catchorg/Catch2

### Documentation & Technical Resources

[4] three.js Contributors. (2024). *three.js Documentation*. https://threejs.org/docs/

[5] three.js Contributors. (2024). *PerspectiveCamera – three.js docs*. https://threejs.org/docs/#api/en/cameras/PerspectiveCamera

[6] three.js Contributors. (2024). *OrthographicCamera – three.js docs*. https://threejs.org/docs/#api/en/cameras/OrthographicCamera

[7] Wikipedia Contributors. (2024). *Seven-segment display*. Wikipedia. https://en.wikipedia.org/wiki/Seven-segment_display

[8] Stroustrup, B. (2024). *C++ Core Guidelines*. GitHub. https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

### Game Development & Software Architecture

[9] Nystrom, R. (2014). *Game Programming Patterns*. Genever Benning. https://gameprogrammingpatterns.com/

[10] Red Blob Games. (2024). *Introduction to A* Pathfinding and Game Math*. https://www.redblobgames.com/

[11] Martin, R. C. (2017). *Clean Architecture: A Craftsman's Guide to Software Structure and Design*. Prentice Hall.

[12] MDN Web Docs. (2024). *2D collision detection*. Mozilla Developer Network. https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection

## Build Instructions

```bash
# Configure (only needed once)
cmake -B build

# Build
cmake --build build

# Run the simulator
./build/src/bilsim

# Run tests
cd build && ctest --output-on-failure

# Or run test executable directly for detailed output
./build/tests/run_tests
```

## Project Structure

```
src/
├── core/          # Game logic and physics
├── graphics/      # Rendering and visualization
├── input/         # User input handling
├── audio/         # Sound management
└── ui/            # User interface elements

tests/
├── test_vehicle.cpp      # Vehicle physics tests
├── test_collision.cpp    # Collision detection tests
└── test_gameObject.cpp   # Base class tests
```

## Reflection

### What I'm Satisfied With

**Architecture and Design:**
The project demonstrates strong adherence to software design principles. The separation between game logic (core/), rendering (graphics/), and input handling creates a maintainable and extensible codebase. The use of the GameObject base class provides clean abstraction, while specific implementations (Vehicle, Powerup) encapsulate their unique behaviors. This follows the Open/Closed Principle - the system is open for extension but closed for modification.

**Modern C++ Practices:**
Consistent use of smart pointers (std::unique_ptr, std::shared_ptr) eliminates memory management concerns and prevents leaks. Anonymous namespaces effectively prevent global namespace pollution, and constexpr constants provide compile-time optimization. The code adheres to C++20 standards throughout, demonstrating understanding of modern language features.

**Physics Implementation:**
The vehicle physics feel responsive and realistic. Speed-sensitive steering (slower turning at high speeds), drift mechanics with angle accumulation/decay, and momentum-based movement create an engaging driving experience. The physics constants are well-tuned for arcade-style gameplay while maintaining believability.

**UI Implementation:**
The custom 7-segment speedometer is a technical highlight, implementing proper digit patterns and segment rendering from first principles rather than using text rendering. The minimap with orthographic camera provides excellent spatial awareness, and the speed-reactive audio creates immersive feedback.

### Areas for Improvement

**Test Coverage:**
While test_vehicle.cpp provides comprehensive unit tests for the Vehicle class, test_collision.cpp and test_gameObject.cpp remain unimplemented (marked as TODO). Complete test coverage would include:
- Collision detection edge cases (boundary touching, contained objects)
- GameObject lifecycle (construction, reset, active/inactive states)
- PowerupManager spawn distribution and collision handling
- Integration tests for the full game loop

**Better approach:** Implement Test-Driven Development (TDD) from the start, writing tests before implementation to ensure complete coverage and catch edge cases early.

**Environmental Variety:**
The current environment is minimal - a flat plane with powerups. The project could benefit from:
- Static obstacles (walls, barriers) for collision avoidance
- Multiple powerup types (speed boost, shield, handling modifier)
- Terrain variation or track boundaries
- Visual feedback for collisions (particle effects, screen shake)

**Better approach:** Create an ObstacleManager similar to PowerupManager, implement multiple PowerupType cases in the collision handler, and add visual/audio feedback systems.

**Input Configuration:**
Input mappings are hardcoded in InputHandler. A configuration system would allow users to rebind keys or use gamepads.

**Better approach:** Implement an InputConfig class that maps logical actions (ACCELERATE, TURN_LEFT) to physical inputs, stored in a config file (JSON/TOML) that can be edited without recompiling.

**Performance Profiling:**
No performance measurement or optimization was performed. With complex 3D models and many objects, framerate could become an issue.

**Better approach:** Integrate a profiling tool (Tracy, Optick) to identify bottlenecks. Implement object pooling for frequently created/destroyed objects, use spatial partitioning (quadtree) for collision detection with many objects.

**Documentation:**
While code is generally self-documenting with good naming, complex algorithms (drift physics, turn rate calculation) would benefit from more detailed comments explaining the mathematical reasoning.

**Better approach:** Add Doxygen-style comments to public interfaces, include diagrams for complex systems, and document the physics equations used.

### Technical Insights Demonstrated

The project showcases understanding of:
- **Game loop architecture:** Fixed timestep with delta time for frame-rate independent physics
- **Entity-Component patterns:** Separation of data (Vehicle) from presentation (VehicleRenderer)
- **Resource management:** RAII principles with smart pointers, automatic asset copying via CMake
- **Real-time rendering:** Multiple cameras (perspective, orthographic), viewport/scissor management for split-screen effects
- **Dependency management:** CMake FetchContent for reproducible builds across platforms
