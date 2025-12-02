#pragma once

#include <memory>
#include <vector>
#include <threepp/threepp.hpp>
#include "core/vehicle.hpp"
#include "core/powerup_manager.hpp"
#include "core/obstacle_manager.hpp"
#include "graphics/vehicle_renderer.hpp"
#include "graphics/powerup_renderer.hpp"
#include "graphics/obstacle_renderer.hpp"
#include "graphics/scene_manager.hpp"
#include "input/input_handler.hpp"
#include "audio/audio_manager.hpp"
#include "ui/imgui_layer.hpp"

/**
 * Main game coordinator.
 * Ties together all the subsystems and runs the game loop.
 */
class Game {
public:
    explicit Game(threepp::Canvas& canvas);
    ~Game() = default;

    void initialize();
    void update(float deltaTime);
    void render();

    [[nodiscard]] threepp::Clock& getClock() noexcept { return clock_; }

private:
    void initializeScene();
    void initializeVehicle();
    void initializeObstacles();
    void initializePowerups();
    void initializeInput();
    void initializeAudio();
    void initializeUI();

    void updateGameState(float deltaTime);
    void updateCamera();
    void updateAudio();

    void renderMainView();
    void renderMinimap();
    void renderUI();

    threepp::Canvas& canvas_;

    std::unique_ptr<SceneManager> sceneManager_;
    std::unique_ptr<Vehicle> vehicle_;
    std::unique_ptr<VehicleRenderer> vehicleRenderer_;

    std::unique_ptr<ObstacleManager> obstacleManager_;
    std::unique_ptr<PowerupManager> powerupManager_;

    std::vector<std::unique_ptr<ObstacleRenderer>> obstacleRenderers_;
    std::vector<std::unique_ptr<PowerupRenderer>> powerupRenderers_;

    std::unique_ptr<InputHandler> inputHandler_;
    std::unique_ptr<AudioManager> audioManager_;
    std::unique_ptr<ImGuiLayer> imguiLayer_;

    bool audioEnabled_;
    bool shouldExit_;
    threepp::Clock clock_;

    int lastWindowWidth_;
    int lastWindowHeight_;

public:
    [[nodiscard]] bool shouldExit() const noexcept { return shouldExit_; }
    void requestExit() noexcept { shouldExit_ = true; }
};
