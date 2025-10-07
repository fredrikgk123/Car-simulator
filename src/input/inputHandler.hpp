#pragma once

#include <threepp/threepp.hpp>
#include <functional>
#include "vehicle.hpp"
#include "../graphics/sceneManager.hpp"

class InputHandler : public threepp::KeyListener {
public:
    explicit InputHandler(Vehicle& vehicle, SceneManager& sceneManager);

    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

    // Apply input to vehicle
    void update(float deltaTime);

    // Set callback for reset event
    void setResetCallback(std::function<void()> callback);

private:
    void onReset();

    Vehicle& vehicle_;
    SceneManager& sceneManager_;

    // Key state tracking
    bool upPressed_;
    bool downPressed_;
    bool leftPressed_;
    bool rightPressed_;
    bool shiftPressed_;

    // Reset callback
    std::function<void()> resetCallback_;
};
