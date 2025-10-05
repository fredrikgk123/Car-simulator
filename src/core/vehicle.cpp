#include "vehicle.hpp"
#include <cmath>

Vehicle::Vehicle(float x, float y, float z)
    : position_({x, y, z}),
      rotation_(0.0f),
      velocity_(0.0f),
      acceleration_(0.0f),
      maxSpeed_(25.0f),
      turnSpeed_(2.0f),
      forwardAcceleration_(12.0f),
      backwardAcceleration_(-6.0f),
      size_({1.0f, 0.5f, 2.0f}) {  // Default size: width, height, length
}

void Vehicle::accelerateForward() {
    acceleration_ = forwardAcceleration_;
}

void Vehicle::accelerateBackward() {
    acceleration_ = backwardAcceleration_;
}

void Vehicle::turn(float amount) {
    // Calculate speed-dependent turn rate
    float turnRate = calculateTurnRate();
    rotation_ += amount * turnSpeed_ * turnRate;

    // Normalize rotation to [0, 2π]
    while (rotation_ >= 2 * M_PI) {
        rotation_ -= 2 * M_PI;
    }
    while (rotation_ < 0) {
        rotation_ += 2 * M_PI;
    }
}

float Vehicle::calculateTurnRate() const {
    // Get absolute velocity
    float absVelocity = std::abs(velocity_);

    // If nearly stopped, don't turn at all
    const float minTurnSpeed = 0.1f;
    if (absVelocity < minTurnSpeed) {
        return 0.0f;
    }

    // Calculate turn rate as a proportion of current speed to max speed
    // At low speeds: turns slowly (e.g., 0.2x normal turn rate)
    // At high speeds: turns at full rate (1.0x normal turn rate)
    float speedRatio = absVelocity / maxSpeed_;

    // Use a curve that gives some turning at low speeds but full turning at high speeds
    // This uses a square root curve for smooth transition
    float turnRate = std::sqrt(speedRatio);

    // Clamp between 0 and 1
    if (turnRate > 1.0f) turnRate = 1.0f;

    return turnRate;
}

void Vehicle::update(float deltaTime) {
    // Update velocity based on acceleration
    velocity_ += acceleration_ * deltaTime;

    // Apply basic friction
    velocity_ *= 0.994f;

    // Clamp velocity
    if (velocity_ > maxSpeed_) {
        velocity_ = maxSpeed_;
    } else if (velocity_ < -maxSpeed_ / 2) {
        velocity_ = -maxSpeed_ / 2;
    }

    // Update position based on velocity and rotation
    float dx = std::sin(rotation_) * velocity_ * deltaTime;
    float dz = std::cos(rotation_) * velocity_ * deltaTime;
    position_[0] += dx;
    position_[2] += dz;

    // Reset acceleration
    acceleration_ = 0.0f;
}

std::vector<float> Vehicle::getPosition() const {
    return position_;
}

float Vehicle::getRotation() const {
    return rotation_;
}

std::vector<float> Vehicle::getSize() const {
    return size_;
}

float Vehicle::getVelocity() const {
    return velocity_;
}

bool Vehicle::checkCollision(const std::vector<float>& objectPosition, float objectRadius) const {
    float dx = position_[0] - objectPosition[0];
    float dy = position_[1] - objectPosition[1];
    float dz = position_[2] - objectPosition[2];
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    float boundingSphere = std::sqrt(size_[0]*size_[0] + size_[1]*size_[1] + size_[2]*size_[2]) / 2;
    return distance < (boundingSphere + objectRadius);
}
