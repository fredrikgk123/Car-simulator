#include "vehicle.hpp"
#include <cmath>

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Physics constants
    const float MAX_SPEED = 25.0f;                  // 25 units/sec - balanced for responsive but controllable gameplay
    const float MAX_REVERSE_SPEED = 12.5f;          // Half of forward - vehicles reverse slower
    const float TURN_SPEED = 2.0f;                  // 2 radians/sec (~115°/sec) - arcade-style handling
    const float FORWARD_ACCELERATION = 12.0f;       // 12 units/sec² - reaches max speed in ~2 seconds
    const float BACKWARD_ACCELERATION = -6.0f;      // Half of forward - vehicles reverse slower than they accelerate
    const float FRICTION_COEFFICIENT = 0.994f;      // 0.994^60 ≈ 0.74 after 1 second at 60fps - natural deceleration
    const float MIN_TURN_SPEED = 0.1f;              // 0.1 units/sec minimum - prevents spinning in place (realistic)

    // Vehicle dimensions
    const float VEHICLE_WIDTH = 1.0f;
    const float VEHICLE_HEIGHT = 0.5f;
    const float VEHICLE_LENGTH = 2.0f;
}

Vehicle::Vehicle(float x, float y, float z)
    : GameObject(x, y, z),
      velocity_(0.0f),
      acceleration_(0.0f) {
    // Set vehicle-specific size
    size_[0] = VEHICLE_WIDTH;
    size_[1] = VEHICLE_HEIGHT;
    size_[2] = VEHICLE_LENGTH;
}

void Vehicle::accelerateForward() {
    acceleration_ = FORWARD_ACCELERATION;
}

void Vehicle::accelerateBackward() {
    acceleration_ = BACKWARD_ACCELERATION;
}

void Vehicle::turn(float amount) {
    float turnRate = calculateTurnRate();
    rotation_ = rotation_ + (amount * TURN_SPEED * turnRate);

    // Normalize rotation to [0, 2π]
    rotation_ = std::fmod(rotation_, 2.0f * static_cast<float>(M_PI));
    if (rotation_ < 0.0f) {
        rotation_ = rotation_ + (2.0f * static_cast<float>(M_PI));
    }
}

float Vehicle::calculateTurnRate() const {
    float absVelocity = std::abs(velocity_);

    // Don't turn if nearly stopped
    if (absVelocity < MIN_TURN_SPEED) {
        return 0.0f;
    }

    // Turn rate scales with speed (using square root for smooth curve)
    float speedRatio = absVelocity / MAX_SPEED;
    float turnRate = std::sqrt(speedRatio);

    // Clamp to maximum of 1.0
    if (turnRate > 1.0f) {
        turnRate = 1.0f;
    }

    return turnRate;
}

void Vehicle::update(float deltaTime) {
    // Update velocity based on acceleration
    velocity_ = velocity_ + (acceleration_ * deltaTime);

    // Apply friction
    velocity_ = velocity_ * FRICTION_COEFFICIENT;

    // Clamp velocity to max speeds
    if (velocity_ > MAX_SPEED) {
        velocity_ = MAX_SPEED;
    }
    if (velocity_ < -MAX_REVERSE_SPEED) {
        velocity_ = -MAX_REVERSE_SPEED;
    }

    // Update position based on velocity and rotation
    float dx = std::sin(rotation_) * velocity_ * deltaTime;
    float dz = std::cos(rotation_) * velocity_ * deltaTime;
    position_[0] = position_[0] + dx;
    position_[2] = position_[2] + dz;

    // Reset acceleration (must be reapplied each frame)
    acceleration_ = 0.0f;
}

void Vehicle::reset() {
    GameObject::reset();
    velocity_ = 0.0f;
    acceleration_ = 0.0f;
}

float Vehicle::getVelocity() const {
    return velocity_;
}

float Vehicle::getMaxSpeed() const {
    return MAX_SPEED;
}
