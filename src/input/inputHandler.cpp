//
// Created by Fredrik Karlsaune on 05/10/2025.
//

#include "inputHandler.hpp"
#include <cmath>

using namespace threepp;

InputHandler::InputHandler(Vehicle& vehicle) : vehicle_(vehicle) {}

void InputHandler::onKeyPressed(KeyEvent evt) {
    switch (evt.key) {
        case Key::UP:
        case Key::W:
            upPressed_ = true;
            break;
        case Key::DOWN:
        case Key::S:
            downPressed_ = true;
            break;
        case Key::LEFT:
        case Key::A:
            leftPressed_ = true;
            break;
        case Key::RIGHT:
        case Key::D:
            rightPressed_ = true;
            break;
        default:
            break;
    }
}

void InputHandler::onKeyReleased(KeyEvent evt) {
    switch (evt.key) {
        case Key::UP:
        case Key::W:
            upPressed_ = false;
            break;
        case Key::DOWN:
        case Key::S:
            downPressed_ = false;
            break;
        case Key::LEFT:
        case Key::A:
            leftPressed_ = false;
            break;
        case Key::RIGHT:
        case Key::D:
            rightPressed_ = false;
            break;
        default:
            break;
    }
}

void InputHandler::update(float deltaTime) {
    // Handle acceleration
    if (upPressed_) {
        vehicle_.accelerateForward();
    } else if (downPressed_) {
        vehicle_.accelerateBackward();
    }

    // Handle turning - speed-dependent turning is handled inside Vehicle class
    // Reverse steering when moving backwards
    float turnDirection;
    if (vehicle_.getVelocity() >= 0) {
        turnDirection = 1.0f;
    } else {
        turnDirection = -1.0f;
    }

    if (leftPressed_) {
        vehicle_.turn(deltaTime * turnDirection);
    }
    if (rightPressed_) {
        vehicle_.turn(-deltaTime * turnDirection);
    }
}
