#include "input_handler.hpp"

using namespace threepp;

InputHandler::InputHandler(Vehicle& vehicle, SceneManager& sceneManager)
    : vehicle_(vehicle),
      sceneManager_(sceneManager),
      wPressed_(false),
      sPressed_(false),
      aPressed_(false),
      dPressed_(false),
      steerLeftPressed_(false),
      steerRightPressed_(false),
      leftArrowPressed_(false),
      rightArrowPressed_(false),
      downArrowPressed_(false),
      shiftPressed_(false),
      resetCallback_(nullptr) {
}

void InputHandler::setResetCallback(std::function<void()> callback) {
    resetCallback_ = std::move(callback);
}

void InputHandler::onReset() {
    if (resetCallback_) {
        resetCallback_();
    }
}

void InputHandler::onKeyPressed(KeyEvent evt) {
    // WASD for steering, arrows for camera snaps
    switch (evt.key) {
        case Key::W:
            wPressed_ = true;
            break;
        case Key::S:
            sPressed_ = true;
            break;
        case Key::A:
            aPressed_ = true;
            steerLeftPressed_ = true;
            break;
        case Key::D:
            dPressed_ = true;
            steerRightPressed_ = true;
            break;
        case Key::LEFT:
            leftArrowPressed_ = true;
            updateCamera();
            break;
        case Key::RIGHT:
            rightArrowPressed_ = true;
            updateCamera();
            break;
        case Key::UP:
            // Reset camera forward
            sceneManager_.setCameraYaw(0.0f);
            break;
        case Key::DOWN:
            downArrowPressed_ = true;
            updateCamera();
            break;
        case Key::SPACE:
            vehicle_.startDrift();
            break;
        case Key::F:
            if (!shiftPressed_) {
                vehicle_.activateNitrous();
                shiftPressed_ = true;
            }
            break;
        case Key::C:
            // Toggle camera mode
            sceneManager_.toggleCameraMode();
            break;
        case Key::R:
            vehicle_.reset();
            // Trigger reset callback to respawn powerups
            onReset();
            break;
        default:
            break;
    }
}

void InputHandler::onKeyReleased(KeyEvent evt) {
    switch (evt.key) {
        case Key::W:
            wPressed_ = false;
            break;
        case Key::S:
            sPressed_ = false;
            break;
        case Key::A:
            aPressed_ = false;
            steerLeftPressed_ = false;
            break;
        case Key::D:
            dPressed_ = false;
            steerRightPressed_ = false;
            break;
        case Key::LEFT:
            leftArrowPressed_ = false;
            updateCamera();
            break;
        case Key::RIGHT:
            rightArrowPressed_ = false;
            updateCamera();
            break;
        case Key::DOWN:
            downArrowPressed_ = false;
            updateCamera();
            break;
        case Key::SPACE:
            vehicle_.stopDrift();
            break;
        case Key::F:
            shiftPressed_ = false;
            break;
        default:
            break;
    }
}

void InputHandler::update(float deltaTime) {
    // Handle acceleration
    if (wPressed_) {
        // Vehicle now owns the acceleration multiplier; use the simple API
        vehicle_.accelerateForward();
    } else if (sPressed_) {
        vehicle_.accelerateBackward();
    }

    // Handle turning (reverse when moving backwards)
    float velocity = vehicle_.getVelocity();
    float turnDirection = (velocity >= 0.0f) ? 1.0f : -1.0f;

    if (aPressed_) {
        vehicle_.turn(deltaTime * turnDirection);
    }
    if (dPressed_) {
        vehicle_.turn(-deltaTime * turnDirection);
    }
}

void InputHandler::updateCamera() {
    if (leftArrowPressed_) {
        sceneManager_.setCameraYawTarget(1.0f);
    } else if (rightArrowPressed_) {
        sceneManager_.setCameraYawTarget(-1.0f);
    } else if (downArrowPressed_) {
        sceneManager_.setCameraYaw(3.14159f);
    } else {
        sceneManager_.setCameraYaw(0.0f);
    }
}
