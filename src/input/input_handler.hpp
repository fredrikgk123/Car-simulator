#pragma once

#include <threepp/threepp.hpp>
#include <functional>
#include "../core/vehicle.hpp"
#include "../graphics/scene_manager.hpp"

class InputHandler : public threepp::KeyListener {
public:
    explicit InputHandler(Vehicle& vehicle, SceneManager& sceneManager);

    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

    // Apply input to vehicle
    void update(float deltaTime);

    // Set callback for reset event
    void setResetCallback(std::function<void()> callback);

    // Get current input state for steering wheel animation
    [[nodiscard]] bool isLeftPressed() const noexcept { return steerLeftPressed_; }
    [[nodiscard]] bool isRightPressed() const noexcept { return steerRightPressed_; }

private:
    void onReset();
    void updateCamera();

    Vehicle& vehicle_;
    SceneManager& sceneManager_;

    // Key state tracking
    bool wPressed_;
    bool sPressed_;
    bool aPressed_;
    bool dPressed_;
    bool steerLeftPressed_;
    bool steerRightPressed_;
    bool leftArrowPressed_;
    bool rightArrowPressed_;
    bool downArrowPressed_;
    bool shiftPressed_;

    // Reset callback
    std::function<void()> resetCallback_;
};
