#include "vehicle.hpp"
#include <cmath>

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Physics constants (realistic tuning)
    const float MAX_SPEED = 41.67f;                  // ~150 km/h (realistic for a small car)
    const float MAX_REVERSE_SPEED = 13.9f;           // ~50 km/h reverse
    const float TURN_SPEED = 1.5f;                   // 1.5 rad/sec (~86°/sec) - sharper turning
    const float FORWARD_ACCELERATION = 8.0f;        // Balanced acceleration
    const float BACKWARD_ACCELERATION = -4.0f;      // Slower reverse acceleration
    const float FRICTION_COEFFICIENT = 0.997f;      // More friction for tighter control
    const float MIN_TURN_SPEED = 0.5f;              // Prevents spinning at very low speed

    // Nitrous constants
    const float NITROUS_DURATION = 5.0f;            // 5 seconds of boost
    const float NITROUS_ACCELERATION = 14.0f;       // Moderate boost acceleration
    const float NITROUS_MAX_SPEED = 50.0f;          // ~180 km/h during boost

    // Vehicle dimensions
    const float VEHICLE_WIDTH = 1.0f;
    const float VEHICLE_HEIGHT = 0.5f;
    const float VEHICLE_LENGTH = 2.0f;
}

Vehicle::Vehicle(float x, float y, float z)
    : GameObject(x, y, z),
      velocity_(0.0f),
      acceleration_(0.0f),
      isDrifting_(false),
      driftAngle_(0.0f),
      hasNitrous_(false),
      nitrousActive_(false),
      nitrousTimeRemaining_(0.0f) {
    // Set vehicle-specific size
    size_[0] = VEHICLE_WIDTH;
    size_[1] = VEHICLE_HEIGHT;
    size_[2] = VEHICLE_LENGTH;

    // Set initial rotation to 180 degrees (π radians) so minimap direction matches
    rotation_ = static_cast<float>(M_PI);
    initialRotation_ = static_cast<float>(M_PI);
}

void Vehicle::accelerateForward() {
    if (nitrousActive_ == true) {
        acceleration_ = NITROUS_ACCELERATION;
    } else {
        acceleration_ = FORWARD_ACCELERATION;
    }
}

void Vehicle::accelerateBackward() {
    acceleration_ = BACKWARD_ACCELERATION;
}

void Vehicle::turn(float amount) {
    float turnRate = calculateTurnRate();
    rotation_ = rotation_ + (amount * TURN_SPEED * turnRate);

    // When drifting, allow the car to build up a drift angle
    if (isDrifting_ == true) {
        // Accumulate drift angle more aggressively (increased multiplier from 0.5 to 1.2)
        driftAngle_ = driftAngle_ + (amount * TURN_SPEED * turnRate * 1.2f);

        // Increased max drift angle to ~60 degrees for more dramatic slides
        const float MAX_DRIFT_ANGLE = static_cast<float>(M_PI) / 3.0f;  // 60° instead of 45°
        if (driftAngle_ > MAX_DRIFT_ANGLE) {
            driftAngle_ = MAX_DRIFT_ANGLE;
        }
        if (driftAngle_ < -MAX_DRIFT_ANGLE) {
            driftAngle_ = -MAX_DRIFT_ANGLE;
        }
    }

    // Normalize rotation to [0, 2π]
    rotation_ = std::fmod(rotation_, 2.0f * static_cast<float>(M_PI));
    if (rotation_ < 0.0f) {
        rotation_ = rotation_ + (2.0f * static_cast<float>(M_PI));
    }
}

float Vehicle::calculateTurnRate() const {
    float absVelocity = std::abs(velocity_);

    // Don't turn if completely stopped
    if (absVelocity < 0.1f) {
        return 0.0f;
    }

    // Extremely low speeds (0.1-0.3 m/s / ~0.4-1.1 km/h): very minimal turning
    // Almost stopped - barely any turning ability
    if (absVelocity < 0.3f) {
        // Linear scaling from 0.05 at 0.1 m/s to 0.15 at 0.3 m/s
        float turnRate = 0.05f + ((absVelocity - 0.1f) / 0.2f) * 0.1f;
        return turnRate;
    }

    // Very low speeds (0.3-3 m/s / ~1.1-11 km/h): minimal but usable turning
    // This allows parking-speed maneuvers
    if (absVelocity < 3.0f) {
        // Linear scaling from 0.15 at 0.3 m/s to 0.5 at 3 m/s
        float turnRate = 0.15f + ((absVelocity - 0.3f) / 2.7f) * 0.35f;
        return turnRate;
    }

    // Low to medium speeds (3-15 m/s / ~11-54 km/h): good turning capability
    if (absVelocity < 15.0f) {
        // Linear scaling from 0.5 at 3 m/s to 1.0 at 15 m/s
        float turnRate = 0.5f + ((absVelocity - 3.0f) / 12.0f) * 0.5f;
        return turnRate;
    }

    // High speeds (15+ m/s / 54+ km/h): reduced turn rate for realism
    float speedRatio = (absVelocity - 15.0f) / (MAX_SPEED - 15.0f);
    float turnRate = 1.0f - (speedRatio * 0.4f);  // Reduces to 60% at max speed

    // Clamp to reasonable range
    if (turnRate < 0.6f) {
        turnRate = 0.6f;
    }
    if (turnRate > 1.0f) {
        turnRate = 1.0f;
    }

    return turnRate;
}

void Vehicle::activateNitrous() {
    if (hasNitrous_ == true && nitrousActive_ == false) {
        nitrousActive_ = true;
        nitrousTimeRemaining_ = NITROUS_DURATION;
        hasNitrous_ = false; // Consumed when activated
    }
}

void Vehicle::startDrift() {
    isDrifting_ = true;
}

void Vehicle::stopDrift() {
    isDrifting_ = false;
    // Keep more of the drift angle when exiting for a smoother transition
    driftAngle_ = driftAngle_ * 0.5f;  // Changed from 0.3 to 0.5
}

bool Vehicle::isDrifting() const {
    return isDrifting_;
}

void Vehicle::pickupNitrous() {
    hasNitrous_ = true;
}

bool Vehicle::hasNitrous() const {
    return hasNitrous_;
}

bool Vehicle::isNitrousActive() const {
    return nitrousActive_;
}

float Vehicle::getNitrousTimeRemaining() const {
    return nitrousTimeRemaining_;
}

void Vehicle::update(float deltaTime) {
    // Update nitrous timer
    if (nitrousActive_ == true) {
        nitrousTimeRemaining_ = nitrousTimeRemaining_ - deltaTime;
        if (nitrousTimeRemaining_ <= 0.0f) {
            nitrousActive_ = false;
            nitrousTimeRemaining_ = 0.0f;
        }
    }

    // Update velocity based on acceleration
    velocity_ = velocity_ + (acceleration_ * deltaTime);

    // Apply friction (less friction while drifting)
    float frictionCoefficient = isDrifting_ ? 0.992f : FRICTION_COEFFICIENT;
    velocity_ = velocity_ * frictionCoefficient;

    // Clamp velocity to max speeds (higher during nitrous)
    float currentMaxSpeed = (nitrousActive_ == true) ? NITROUS_MAX_SPEED : MAX_SPEED;

    if (velocity_ > currentMaxSpeed) {
        velocity_ = currentMaxSpeed;
    }
    if (velocity_ < -MAX_REVERSE_SPEED) {
        velocity_ = -MAX_REVERSE_SPEED;
    }

    // When drifting, car moves in a direction between facing and drift angle
    // When not drifting, car moves in facing direction
    float movementAngle = rotation_;
    if (isDrifting_ == true) {
        // Blend between facing direction and drift angle
        movementAngle = rotation_ - driftAngle_;

        // Gradually reduce drift angle over time (self-correcting)
        driftAngle_ = driftAngle_ * 0.95f;
    }

    // Update position based on velocity and movement angle
    float dx = std::sin(movementAngle) * velocity_ * deltaTime;
    float dz = std::cos(movementAngle) * velocity_ * deltaTime;
    position_[0] = position_[0] + dx;
    position_[2] = position_[2] + dz;

    // Reset acceleration (must be reapplied each frame)
    acceleration_ = 0.0f;
}

void Vehicle::reset() {
    GameObject::reset();
    velocity_ = 0.0f;
    acceleration_ = 0.0f;
    isDrifting_ = false;
    driftAngle_ = 0.0f;
    hasNitrous_ = false;
    nitrousActive_ = false;
    nitrousTimeRemaining_ = 0.0f;

    // Reset camera to orbit mode
    if (resetCameraCallback_) {
        resetCameraCallback_();
    }
}

void Vehicle::setResetCameraCallback(std::function<void()> callback) {
    resetCameraCallback_ = callback;
}

float Vehicle::getVelocity() const {
    return velocity_;
}

float Vehicle::getMaxSpeed() const {
    return MAX_SPEED;
}
