#include <threepp/threepp.hpp>
#include <memory>
#include <filesystem>
#include <iostream>
#include "vehicle.hpp"
#include "vehicleRenderer.hpp"
#include "sceneManager.hpp"
#include "inputHandler.hpp"
#include "audioManager.hpp"
#include "uiManager.hpp"

using namespace threepp;

int main() {
    // Create window
    Canvas canvas("Bilsimulator");

    // Create scene manager
    SceneManager sceneManager;
    sceneManager.setupCamera(canvas.aspect());
    sceneManager.setupMinimapCamera(1.0f);  // Setup minimap with square aspect ratio
    sceneManager.setupRenderer(canvas.size());
    sceneManager.setupLighting();
    sceneManager.setupGround();

    // Create vehicle (backend - pure math)
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    // Create vehicle renderer (frontend - visualization)
    VehicleRenderer vehicleRenderer(sceneManager, vehicle);

    // Create input handler
    auto inputHandler = std::make_unique<InputHandler>(vehicle);
    canvas.addKeyListener(*inputHandler);

    // Create audio manager and initialize engine sound
    AudioManager audioManager;
    bool audioEnabled = audioManager.initialize("carnoise.wav");

    if (!audioEnabled) {
        std::cout << "Audio file 'carnoise.wav' not found. Continuing without audio..." << std::endl;
    }

    // Create UI manager for speedometer overlay (pass renderer reference)
    UIManager uiManager(sceneManager.getRenderer());

    // Handle window resize
    canvas.onWindowResize([&](WindowSize size) {
        sceneManager.resize(size);
    });

    // Animation loop
    Clock clock;
    canvas.animate([&] {
        float deltaTime = clock.getDelta();

        // Update input and vehicle physics
        inputHandler->update(deltaTime);
        vehicle.update(deltaTime);

        // Update renderer
        vehicleRenderer.update();

        // Update camera to follow vehicle
        auto pos = vehicle.getPosition();
        sceneManager.updateCameraFollowTarget(pos[0], pos[1], pos[2], vehicle.getRotation());
        sceneManager.updateMinimapCamera(pos[0], pos[2]);  // Update minimap to follow vehicle

        // Update audio based on vehicle state
        if (audioEnabled) {
            audioManager.update(vehicle);
        }

        // Get renderer and canvas size for viewport manipulation
        auto& renderer = sceneManager.getRenderer();
        auto size = canvas.size();

        // Render the main scene first
        renderer.setViewport(0, 0, size.width(), size.height());
        renderer.setScissor(0, 0, size.width(), size.height());
        renderer.setScissorTest(false);
        sceneManager.getRenderer().render(sceneManager.getScene(), sceneManager.getCamera());

        // Render minimap in top-left corner (150x150 pixels)
        int minimapSize = 150;
        renderer.setViewport(10, size.height() - minimapSize - 10, minimapSize, minimapSize);
        renderer.setScissor(10, size.height() - minimapSize - 10, minimapSize, minimapSize);
        renderer.setScissorTest(true);
        sceneManager.renderMinimap();
        renderer.setScissorTest(false);

        // Restore full viewport
        renderer.setViewport(0, 0, size.width(), size.height());

        // Render speedometer HUD overlay on top of everything
        uiManager.render(vehicle, size);
    });

    return 0;
}
