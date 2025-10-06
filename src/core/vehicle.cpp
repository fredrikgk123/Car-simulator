#include "vehicle.hpp"
#include <cmath>

// Define static constants
const float Vehicle::MAX_SPEED = 25.0f;                  // 25 units/sec - balanced for responsive but controllable gameplay
const float Vehicle::MAX_REVERSE_SPEED = 12.5f;          // Half of forward - vehicles reverse slower
const float Vehicle::TURN_SPEED = 2.0f;                  // 2 radians/sec (~115°/sec) - arcade-style handling
const float Vehicle::FORWARD_ACCELERATION = 12.0f;       // 12 units/sec² - reaches max speed in ~2 seconds
const float Vehicle::BACKWARD_ACCELERATION = -6.0f;      // Half of forward - vehicles reverse slower than they accelerate
const float Vehicle::FRICTION_COEFFICIENT = 0.994f;      // 0.994^60 ≈ 0.74 after 1 second at 60fps - natural deceleration
const float Vehicle::MIN_TURN_SPEED = 0.1f;              // 0.1 units/sec minimum - prevents spinning in place (realistic)
const float Vehicle::VEHICLE_WIDTH = 1.0f;
const float Vehicle::VEHICLE_HEIGHT = 0.5f;
const float Vehicle::VEHICLE_LENGTH = 2.0f;

Vehicle::Vehicle(float x, float y, float z)
    : position_({x, y, z}),
      initialPosition_({x, y, z}),
      rotation_(0.0f),
      velocity_(0.0f),
      acceleration_(0.0f) {
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
    position_ = initialPosition_;
    rotation_ = 0.0f;
    velocity_ = 0.0f;
    acceleration_ = 0.0f;
}

const std::array<float, 3>& Vehicle::getPosition() const {
    return position_;
}

float Vehicle::getRotation() const {
    return rotation_;
}

const std::array<float, 3>& Vehicle::getSize() const {
    static const std::array<float, 3> size = {VEHICLE_WIDTH, VEHICLE_HEIGHT, VEHICLE_LENGTH};
    return size;
}

float Vehicle::getVelocity() const {
    return velocity_;
}
