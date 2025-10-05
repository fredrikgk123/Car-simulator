#include <threepp/threepp.hpp>
#include <memory>
#include "vehicle.hpp"
#include "vehicleRenderer.hpp"
#include "sceneManager.hpp"
#include "inputHandler.hpp"

using namespace threepp;

int main() {
    // Create window
    Canvas canvas("Bilsimulator");

    // Create scene manager
    SceneManager sceneManager;
    sceneManager.setupCamera(canvas.aspect());
    sceneManager.setupLighting();
    sceneManager.setupGround();

    // Create vehicle (backend - pure math)
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    // Create vehicle renderer (frontend - visualization)
    VehicleRenderer vehicleRenderer(sceneManager, vehicle);

    // Create input handler
    auto inputHandler = std::make_unique<InputHandler>(vehicle);
    canvas.addKeyListener(*inputHandler);

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

        // Render the scene
        sceneManager.getRenderer().render(sceneManager.getScene(), sceneManager.getCamera());
    });

    return 0;
}
