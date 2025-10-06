# Project Architecture Review & Expansion Guide

## 🎯 Overall Assessment: **EXCELLENT Foundation - Ready for Required Features**

Your refactored project has a **solid architecture** that's ready for the mandatory assignment requirements: powerups, obstacles, and collision responses.

---

## 📋 Assignment Requirements Status

### ✅ Completed Requirements

#### Vehicle Control (Bilkontroll)
- ✅ **Forward/backward movement** - Implemented with acceleration and friction
- ✅ **Rotation** - Implemented with speed-sensitive turning
- ✅ **Simple collision detection** - AABB implemented in GameObject
- ⚠️ **Movement visualization** - No wheel rotation yet (should add)

#### Bonus Features (Already Implemented!)
- ✅ **Sound** - Dynamic engine sound with pitch/volume
- ✅ **UI elements** - 7-segment speedometer display + minimap
- ✅ **Dynamic steering** - Acceleration, momentum, friction modeled

### 🚨 CRITICAL Missing Requirements

#### 1. **NO UNIT TESTS** ⚠️ (MANDATORY)
The assignment explicitly requires unit tests with Catch2.
- **Priority: CRITICAL**
- **Time: 1-2 days**

#### 2. **Object Interaction** ⚠️ (Core Requirement)
- ❌ No powerups to collect
- ❌ No obstacles to avoid
- ❌ No collision response (objects exist in architecture but not in game)
- **Priority: CRITICAL**
- **Time: 2-3 days**

#### 3. **Environment Objects** ⚠️
- ❌ No scattered objects in the world
- ❌ No obstacles/hazards/doors
- **Priority: HIGH**
- **Time: Included in #2**

#### 4. **Movement Visualization** (Should Add)
- ❌ No wheel rotation or other animation
- **Priority: MEDIUM**
- **Time: 1 day**

#### 5. **Documentation** ⚠️
- ❌ README missing UML diagram
- ❌ README missing reflection
- ❌ README missing candidate number
- **Priority: CRITICAL**
- **Time: 1 day**

---

## 🚀 Priority Action Plan (6-8 Days)

### Phase 1: CRITICAL - Must Complete (4 days)

#### Day 1-2: Add Unit Tests
**Status: MANDATORY REQUIREMENT**

```cmake
# Add to root CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_subdirectory(tests)
```

**Required test files:**
```cpp
// tests/test_vehicle.cpp
TEST_CASE("Vehicle accelerates correctly") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);
    vehicle.accelerateForward();
    vehicle.update(0.1f);  // 0.1 second
    REQUIRE(vehicle.getVelocity() > 0.0f);
}

TEST_CASE("Vehicle respects max speed") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 100; i++) {
        vehicle.accelerateForward();
        vehicle.update(0.1f);
    }
    REQUIRE(vehicle.getVelocity() <= vehicle.getMaxSpeed());
}

// tests/test_collision.cpp
TEST_CASE("Collision detection works") {
    GameObject obj1(0.0f, 0.0f, 0.0f);
    GameObject obj2(0.5f, 0.0f, 0.0f);  // Overlapping
    REQUIRE(obj1.intersects(obj2) == true);
    
    GameObject obj3(10.0f, 0.0f, 0.0f);  // Far away
    REQUIRE(obj1.intersects(obj3) == false);
}
```

**Create tests/CMakeLists.txt:**
```cmake
add_executable(run_tests
    test_vehicle.cpp
    test_collision.cpp
    test_gameObject.cpp
)

target_link_libraries(run_tests PRIVATE
    core
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(run_tests)
```

#### Day 3-4: Implement Powerups and Obstacles

**1. Create Powerup Class:**
```cpp
// src/core/powerup.hpp
#pragma once
#include "gameObject.hpp"

class Powerup : public GameObject {
public:
    enum class Type {
        SPEED_BOOST,    // Increases max speed
        SIZE_CHANGE,    // Makes vehicle bigger/smaller
        INVINCIBILITY   // Temporary shield
    };

    Powerup(float x, float y, float z, Type type);
    
    void update(float deltaTime) override;
    
    Type getType() const { return type_; }
    bool isCollected() const { return collected_; }
    void collect();

private:
    Type type_;
    bool collected_;
    float rotationSpeed_;  // For spinning animation
};
```

```cpp
// src/core/powerup.cpp
#include "powerup.hpp"
#include <cmath>

Powerup::Powerup(float x, float y, float z, Type type)
    : GameObject(x, y, z),
      type_(type),
      collected_(false),
      rotationSpeed_(2.0f) {
    // Smaller size for powerups
    size_[0] = 0.5f;
    size_[1] = 0.5f;
    size_[2] = 0.5f;
}

void Powerup::update(float deltaTime) {
    if (collected_ == false) {
        // Spin the powerup for visual effect
        rotation_ = rotation_ + (rotationSpeed_ * deltaTime);
        
        // Bob up and down
        position_[1] = 0.5f + (std::sin(rotation_) * 0.2f);
    }
}

void Powerup::collect() {
    collected_ = true;
    active_ = false;  // Hide it
}
```

**2. Create Obstacle Class:**
```cpp
// src/core/obstacle.hpp
#pragma once
#include "gameObject.hpp"

class Obstacle : public GameObject {
public:
    Obstacle(float x, float y, float z, float width, float height, float length);
    
    void update(float deltaTime) override {
        // Static obstacles don't move
    }
};
```

```cpp
// src/core/obstacle.cpp
#include "obstacle.hpp"

Obstacle::Obstacle(float x, float y, float z, float width, float height, float length)
    : GameObject(x, y, z) {
    size_[0] = width;
    size_[1] = height;
    size_[2] = length;
}
```

**3. Create Powerup Renderer:**
```cpp
// src/graphics/powerupRenderer.hpp
#pragma once
#include "gameObjectRenderer.hpp"
#include "../core/powerup.hpp"

class PowerupRenderer : public GameObjectRenderer {
public:
    PowerupRenderer(threepp::Scene& scene, const Powerup& powerup);

protected:
    void createModel() override;

private:
    const Powerup& powerup_;
};
```

```cpp
// src/graphics/powerupRenderer.cpp
#include "powerupRenderer.hpp"

using namespace threepp;

PowerupRenderer::PowerupRenderer(Scene& scene, const Powerup& powerup)
    : GameObjectRenderer(scene, powerup),
      powerup_(powerup) {
}

void PowerupRenderer::createModel() {
    std::array<float, 3> size = powerup_.getSize();

    // Create sphere or star shape for powerup
    auto geometry = SphereGeometry::create(size[0]);
    auto material = MeshPhongMaterial::create();
    
    // Different colors for different types
    switch (powerup_.getType()) {
        case Powerup::Type::SPEED_BOOST:
            material->color = Color::yellow;
            break;
        case Powerup::Type::SIZE_CHANGE:
            material->color = Color::green;
            break;
        case Powerup::Type::INVINCIBILITY:
            material->color = Color::blue;
            break;
    }
    
    material->emissive = material->color;
    material->emissiveIntensity = 0.3f;

    bodyMesh_ = Mesh::create(geometry, material);
    bodyMesh_->castShadow = true;

    objectGroup_->add(bodyMesh_);
}
```

**4. Update CMakeLists.txt files:**
```cmake
# src/core/CMakeLists.txt
add_library(core 
    gameObject.cpp 
    gameObject.hpp
    vehicle.cpp 
    vehicle.hpp
    powerup.cpp
    powerup.hpp
    obstacle.cpp
    obstacle.hpp
)
```

```cmake
# src/graphics/CMakeLists.txt
add_library(graphics
    gameObjectRenderer.cpp
    gameObjectRenderer.hpp
    vehicleRenderer.cpp
    vehicleRenderer.hpp
    powerupRenderer.cpp
    powerupRenderer.hpp
    sceneManager.cpp
    sceneManager.hpp
)
```

**5. Add to main.cpp:**
```cpp
#include "core/powerup.hpp"
#include "core/obstacle.hpp"
#include "graphics/powerupRenderer.hpp"

// After vehicle creation in main():
std::vector<Powerup> powerups;
std::vector<std::unique_ptr<PowerupRenderer>> powerupRenderers;
std::vector<Obstacle> obstacles;
std::vector<std::unique_ptr<GameObjectRenderer>> obstacleRenderers;

// Create powerups
powerups.emplace_back(-5.0f, 0.5f, 5.0f, Powerup::Type::SPEED_BOOST);
powerups.emplace_back(5.0f, 0.5f, 5.0f, Powerup::Type::SIZE_CHANGE);
powerups.emplace_back(0.0f, 0.5f, -5.0f, Powerup::Type::INVINCIBILITY);
powerups.emplace_back(-8.0f, 0.5f, -3.0f, Powerup::Type::SPEED_BOOST);
powerups.emplace_back(8.0f, 0.5f, 3.0f, Powerup::Type::SIZE_CHANGE);

for (Powerup& powerup : powerups) {
    powerupRenderers.push_back(
        std::make_unique<PowerupRenderer>(sceneManager.getScene(), powerup)
    );
}

// Create obstacles (walls)
obstacles.emplace_back(-10.0f, 0.0f, 0.0f, 1.0f, 2.0f, 5.0f);
obstacles.emplace_back(10.0f, 0.0f, 0.0f, 1.0f, 2.0f, 5.0f);
obstacles.emplace_back(0.0f, 0.0f, 10.0f, 10.0f, 2.0f, 1.0f);
obstacles.emplace_back(0.0f, 0.0f, -10.0f, 10.0f, 2.0f, 1.0f);

for (Obstacle& obstacle : obstacles) {
    obstacleRenderers.push_back(
        std::make_unique<GameObjectRenderer>(sceneManager.getScene(), obstacle)
    );
}

// In game loop, after vehicle.update():
// Update powerups
for (Powerup& powerup : powerups) {
    powerup.update(deltaTime);
}

// Check collisions with powerups
for (Powerup& powerup : powerups) {
    if (vehicle.intersects(powerup) && powerup.isCollected() == false) {
        powerup.collect();
        
        // Apply powerup effect
        switch (powerup.getType()) {
            case Powerup::Type::SPEED_BOOST:
                std::cout << "Speed boost collected!" << std::endl;
                // Could increase vehicle max speed here
                break;
            case Powerup::Type::SIZE_CHANGE:
                std::cout << "Size change collected!" << std::endl;
                // Could change vehicle size here
                break;
            case Powerup::Type::INVINCIBILITY:
                std::cout << "Invincibility collected!" << std::endl;
                break;
        }
    }
}

// Check collisions with obstacles
for (const Obstacle& obstacle : obstacles) {
    if (vehicle.intersects(obstacle)) {
        // Stop vehicle on collision
        std::array<float, 3> pos = vehicle.getPosition();
        std::array<float, 3> obstaclePos = obstacle.getPosition();
        
        // Simple pushback: move vehicle slightly away from obstacle
        float dx = pos[0] - obstaclePos[0];
        float dz = pos[2] - obstaclePos[2];
        float distance = std::sqrt(dx * dx + dz * dz);
        
        if (distance > 0.0f) {
            float pushDistance = 0.1f;
            vehicle.setPosition(
                pos[0] + (dx / distance) * pushDistance,
                pos[1],
                pos[2] + (dz / distance) * pushDistance
            );
        }
    }
}

// Update renderers
for (auto& renderer : powerupRenderers) {
    renderer->update();
}
for (auto& renderer : obstacleRenderers) {
    renderer->update();
}
```

#### Day 5: Complete README Documentation

Create a comprehensive README.md:

```markdown
# Bilsimulator
**Kandidatnummer:** [YOUR NUMBER HERE]

## Beskrivelse
En 3D bilsimulator bygget med threepp hvor spilleren kan kjøre rundt, samle power-ups, og unngå hindringer. Prosjektet demonstrerer moderne C++ prinsipper, objektorientert design, og 3D rendering.

## Funksjoner

### Kjernefunksjonalitet
- **Realistisk bilkontroll** med akselerasjon, friksjon, og momentum
- **Kollisjonsdeteksjon** med AABB (Axis-Aligned Bounding Box)
- **Power-ups** som kan samles opp (speed boost, size change, invincibility)
- **Hindringer** som blokkerer veien
- **Hastighetssensitiv sving** - lettere å svinge ved lav hastighet

### Bonus Funksjoner
- **Dynamisk motor-lyd** som endrer pitch og volum basert på hastighet
- **UI elementer** med 7-segment speedometer display
- **Minimap** for oversikt
- **Enhetstester** med Catch2

## Hvordan Bruke

### Bygging
```bash
# Clone repository
git clone [YOUR_REPO_URL]
cd bilsim

# Build
cmake -B build
cmake --build build

# Run tests
cd build
ctest
```

### Kjøring
```bash
./build/src/bilsim
```

### Kontroller
- **W / Pil Opp**: Akseler fremover
- **S / Pil Ned**: Akseler bakover
- **A / Pil Venstre**: Sving venstre
- **D / Pil Høyre**: Sving høyre
- **R**: Reset (starter på nytt)

## Arkitektur

### UML Klassediagram

```
┌─────────────────┐
│   GameObject    │ (abstract)
├─────────────────┤
│ - position_     │
│ - rotation_     │
│ - size_         │
│ - active_       │
├─────────────────┤
│ + update()      │ (pure virtual)
│ + intersects()  │
│ + getPosition() │
│ + setPosition() │
└────────┬────────┘
         │
         ├──────────────────┬──────────────────┐
         │                  │                  │
    ┌────▼─────┐      ┌────▼─────┐      ┌────▼──────┐
    │ Vehicle  │      │ Powerup  │      │ Obstacle  │
    ├──────────┤      ├──────────┤      ├───────────┤
    │-velocity_│      │ - type_  │      │           │
    │          │      │-collected│      │           │
    └──────────┘      └──────────┘      └───────────┘

┌──────────────────────┐
│ GameObjectRenderer   │ (abstract)
├──────────────────────┤
│ - scene_             │
│ - gameObject_        │
│ - objectGroup_       │
│ - bodyMesh_          │
├──────────────────────┤
│ + update()           │
│ + createModel()      │ (virtual)
│ + setVisible()       │
└──────────┬───────────┘
           │
           ├─────────────────┬──────────────────┐
           │                 │                  │
    ┌──────▼────────┐  ┌─────▼────────┐  ┌────▼──────────┐
    │VehicleRenderer│  │PowerupRenderer│  │ObstacleRenderer│
    └───────────────┘  └───────────────┘  └────────────────┘

Støtte-klasser:
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│SceneManager  │  │InputHandler  │  │AudioManager  │  │UIManager     │
└──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
```

### Modulstruktur
```
src/
├── core/           # Game logic
│   ├── gameObject  # Base class for all entities
│   ├── vehicle     # Player-controlled vehicle
│   ├── powerup     # Collectible power-ups
│   └── obstacle    # Static obstacles
├── graphics/       # Rendering
│   ├── gameObjectRenderer  # Generic renderer
│   ├── vehicleRenderer     # Vehicle-specific
│   ├── powerupRenderer     # Powerup-specific
│   └── sceneManager        # Scene/camera management
├── input/          # User input
│   └── inputHandler
├── audio/          # Sound system
│   └── audioManager
└── ui/             # User interface
    └── uiManager
```

## Design Prinsipper

### Separation of Concerns
- **Core**: Ren spillogikk uten avhengigheter til grafikk
- **Graphics**: Håndterer kun rendering, ingen spillogikk
- **Input/Audio/UI**: Isolerte moduler

### Object-Oriented Design
- **Arv**: GameObject er base class for alle entiteter
- **Polymorfi**: GameObjectRenderer kan rendre alle GameObject-typer
- **Innkapsling**: Private members med public getters/setters

### Low Coupling, High Cohesion
- Hver modul har et klart ansvar
- Minimal avhengighet mellom moduler
- CMake moduler for hver komponent

## Refleksjon

### Hva jeg er fornøyd med

1. **Arkitektur**
   - GameObject/GameObjectRenderer system gjør det trivielt å legge til nye objekttyper
   - Klar separasjon mellom logikk og rendering
   - Modulær CMake struktur

2. **Kode kvalitet**
   - Bruker moderne C++ (smart pointers, std::array, anonymous namespaces)
   - Ingen globale variabler
   - Konsistent navnekonvensjon og formatering

3. **Kreativitet**
   - 7-segment speedometer display (ikke bare tekst)
   - Dynamisk motor-lyd som respons på hastighet
   - Smooth camera following med interpolasjon

4. **Utvidbarhet**
   - Enkelt å legge til nye powerup typer
   - Enkel kollisjonsdeteksjon som "bare fungerer"
   - Collision response kan forbedres uten å endre arkitektur

### Forbedringsmuligheter

1. **Fysikk**
   - Kunne brukt en dedikert fysikkmotor (Bullet, PhysX)
   - Mer realistisk kollisjonshåndtering (momentum, bounce)
   - Terrenghåndtering (ikke bare flat bakke)

2. **Rendering**
   - Kunne lastet 3D modeller istedenfor simple bokser
   - Hjulrotasjon og andre animasjoner
   - Partikkeleffekter for visuell feedback

3. **Gameplay**
   - Score system
   - Flere nivåer eller baner
   - AI-kontrollerte biler
   - Mer varierte power-up effekter

4. **Kode Organisering**
   - Kunne laget en Game-klasse for å flytte logikk ut av main.cpp
   - CollisionManager for å sentralisere kollisjonslogikk
   - Object pooling for bedre performance med mange objekter

5. **Testing**
   - Flere enhetstester for edge cases
   - Integrasjonstester
   - Performance testing

### Teknisk Innsikt

**GameObject Pattern**: Bruk av en felles base class med virtuelle funksjoner var veldig effektivt. Det gjorde det mulig å behandle alle objekter uniformt (collision detection, rendering) samtidig som hver type kan ha sin egen oppførsel.

**Smart Pointers**: Bruk av `std::unique_ptr` og `std::shared_ptr` eliminerte memory leaks og gjorde eierskap eksplisitt. Spesielt i rendering systemet hvor threepp bruker shared_ptr.

**Anonymous Namespaces**: For å unngå globale variabler brukte jeg anonymous namespaces for fil-lokale konstanter. Dette gir compile-time optimization samtidig som det holder global scope ren.

**AABB Collision**: Valgte AABB (Axis-Aligned Bounding Box) fordi det er enkelt, raskt, og tilstrekkelig for dette prosjektet. For et større spill ville jeg vurdert:
- Bounding spheres for runde objekter
- Spatial partitioning (quadtree/octree) for mange objekter
- Sweep-and-prune for continuous collision detection

## Avhengigheter

- **threepp**: 3D rendering library
- **miniaudio**: Audio playback
- **Catch2**: Unit testing framework (test build only)
- **CMake**: Build system (≥3.21)
- **C++20**: Compiler support required

## Lisens
[Your chosen license]

## Forfatter
Kandidat: [YOUR NUMBER]
```

### Phase 2: IMPORTANT - Should Complete (2-3 days)

#### Day 6: Add Movement Visualization
```cpp
// In VehicleRenderer, add wheels
void VehicleRenderer::createModel() {
    // ...existing body mesh code...
    
    // Add wheels
    auto wheelGeometry = CylinderGeometry::create(0.2f, 0.2f, 0.15f);
    auto wheelMaterial = MeshPhongMaterial::create();
    wheelMaterial->color = Color::black;
    
    // Front left wheel
    auto wheelFL = Mesh::create(wheelGeometry, wheelMaterial);
    wheelFL->rotation.z = math::PI / 2;
    wheelFL->position.set(-0.4f, 0.2f, 0.6f);
    objectGroup_->add(wheelFL);
    
    // Store wheel references for rotation in update()
    wheels_.push_back(wheelFL);
    // ...add other 3 wheels...
}

void VehicleRenderer::update() {
    GameObjectRenderer::update();
    
    // Rotate wheels based on velocity
    float velocity = vehicle_.getVelocity();
    float wheelRotation = velocity * 0.1f;
    for (auto& wheel : wheels_) {
        wheel->rotation.x += wheelRotation;
    }
}
```

#### Day 7: Error Handling
```cpp
// In audioManager.cpp initialize()
if (engineSoundPath.empty()) {
    std::cerr << "Error: Empty sound path provided" << std::endl;
    return false;
}

// Check file exists before loading
std::ifstream file(engineSoundPath);
if (file.good() == false) {
    std::cerr << "Error: Sound file not found: " << engineSoundPath << std::endl;
    return false;
}

// In vehicle.cpp
void Vehicle::setPosition(float x, float y, float z) {
    // Validate position is within bounds
    const float MAX_POSITION = 100.0f;
    if (std::abs(x) > MAX_POSITION || std::abs(z) > MAX_POSITION) {
        std::cerr << "Warning: Position out of bounds, clamping" << std::endl;
        x = std::clamp(x, -MAX_POSITION, MAX_POSITION);
        z = std::clamp(z, -MAX_POSITION, MAX_POSITION);
    }
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
}
```

### Phase 3: BONUS - If Time Permits

#### Optional: CI/CD Setup
```yaml
# .github/workflows/build.yml
name: Build and Test

on: [push, pull_request]

jobs:
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Dependencies
        run: sudo apt-get update && sudo apt-get install -y cmake g++ xorg-dev libgl1-mesa-dev
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      - name: Build
        run: cmake --build build
      - name: Test
        run: cd build && ctest --output-on-failure

  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v3
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      - name: Build
        run: cmake --build build
      - name: Test
        run: cd build && ctest --output-on-failure
```

---

## ✅ What You Have (Strengths)

### Architecture (A+ Level)
- ✅ GameObject base class with inheritance
- ✅ Generic GameObjectRenderer system
- ✅ Clear separation: core/graphics/input/audio/ui
- ✅ Low coupling, high cohesion
- ✅ Easy to extend with new object types

### Code Quality (A Level)
- ✅ Modern C++20 (smart pointers, std::array, anonymous namespaces)
- ✅ No global variables
- ✅ Consistent naming and formatting
- ✅ Well-commented code
- ✅ Modular CMake structure

### Creativity (A Level)
- ✅ 7-segment display speedometer (unique!)
- ✅ Dynamic engine sound
- ✅ Smooth camera interpolation
- ✅ Minimap view

---

## 📊 Time Estimate Summary

| Task | Days | Priority | Status |
|------|------|----------|--------|
| Unit tests | 1-2 | CRITICAL | ❌ Must do |
| Powerups + Obstacles | 2-3 | CRITICAL | ❌ Must do |
| README + UML | 1 | CRITICAL | ❌ Must do |
| Collision response | 0.5 | HIGH | ⚠️ Included in above |
| Movement visualization | 1 | MEDIUM | ⚠️ Should do |
| Error handling | 0.5 | MEDIUM | ⚠️ Should do |
| CI/CD | 0.5 | BONUS | ⚠️ If time |
| **TOTAL** | **6-8** | | |

---

## 📝 Before Submission Checklist

- [ ] Unit tests implemented and passing
- [ ] At least 5 powerups in the world
- [ ] At least 4-5 obstacles in the world
- [ ] Collision response works (vehicle stops/bounces)
- [ ] README complete with:
  - [ ] Candidate number
  - [ ] UML diagram
  - [ ] Usage instructions
  - [ ] Reflection section
- [ ] Code compiles without warnings
- [ ] Remove `cmake-build-*` directories
- [ ] Remove `.idea` directory
- [ ] Remove other machine-specific files
- [ ] Test build on clean machine
- [ ] Create .zip file with correct structure

---

## 🎯 Expected Grade Outcome

| Implementation Level | Grade | Reasoning |
|---------------------|-------|-----------|
| Current (no tests/objects) | D/E | Missing mandatory requirements |
| With tests + basic objects | C | Meets minimum requirements |
| + Good documentation | B | Complete implementation |
| + Movement viz + polish | A | Excellent implementation |

Your architecture is already **A-level quality**. You just need to add the required game objects and tests to achieve the grade you deserve!

---

## 💡 Quick Win Strategy

**Week 1 Focus:** Get it working
1. Add Catch2 (30 min)
2. Write 5-6 basic tests (3 hours)
3. Copy-paste Powerup/Obstacle classes from above (2 hours)
4. Add objects to main.cpp (1 hour)
5. Test collision response (1 hour)

**Week 2 Focus:** Polish and document
1. Write README (3 hours)
2. Create UML diagram (1 hour)
3. Add wheel rotation (2 hours)
4. Error handling (2 hours)
5. Final testing and cleanup (2 hours)

**Total: ~17 hours of focused work = Solid A-grade project**

Your groundwork is excellent - now execute on the requirements! 🚀
