#pragma once

#include <threepp/threepp.hpp>
#include <functional>
#include "core/interfaces/IControllable.hpp"
#include "graphics/scene_manager.hpp"

/**
 * Handles keyboard input and translates it to game controls.
 * Implements WASD movement, space for drift, arrow keys for camera, and action keys.
 */
class InputHandler : public threepp::KeyListener {
public:
    explicit InputHandler(IControllable& controllable, SceneManager& sceneManager);

    void onKeyPressed(threepp::KeyEvent evt) override;
    void onKeyReleased(threepp::KeyEvent evt) override;

    // Apply continuous input (e.g., acceleration)
    void update(float deltaTime);

    // Set callback for reset event
    void setResetCallback(std::function<void()> callback);

    // Current steering input state for visual feedback
    [[nodiscard]] bool isLeftPressed() const noexcept { return steerLeftPressed_; }
    [[nodiscard]] bool isRightPressed() const noexcept { return steerRightPressed_; }

private:
    void onReset();
    void updateCamera();

    IControllable& controllable_;
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

    std::function<void()> resetCallback_;
};
