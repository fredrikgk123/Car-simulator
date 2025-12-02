#include "core/vehicle.hpp"
#include "core/vehicle_tuning.hpp"
#include <algorithm>
#include <cmath>

Vehicle::Vehicle(float x, float y, float z)
    : GameObject(x, y, z),
      velocity_(0.0f),
      acceleration_(0.0f),
      steeringInput_(0.0f),
      isDrifting_(false),
      driftAngle_(0.0f),
      hasNitrous_(false),
      nitrousActive_(false),
      nitrousTimeRemaining_(0.0f),
      currentGear_(1),
      rpm_(VehicleTuning::IDLE_RPM),
      scale_(VehicleTuning::DEFAULT_SCALE),
      accelMultiplier_(1.0f),
      resetCameraCallback_() {
    size_[0] = VehicleTuning::VEHICLE_WIDTH;
    size_[1] = VehicleTuning::VEHICLE_HEIGHT;
    size_[2] = VehicleTuning::VEHICLE_LENGTH;

    // Start facing down in minimap (180 degrees)
    rotation_ = VehicleTuning::INITIAL_ROTATION_RADIANS;
    initialRotation_ = VehicleTuning::INITIAL_ROTATION_RADIANS;
}

void Vehicle::accelerateForward() noexcept {
    accelerateForward(accelMultiplier_);
}

void Vehicle::accelerateForward(float multiplier) noexcept {
    float baseAcceleration = nitrousActive_ ? VehicleTuning::NITROUS_ACCELERATION : VehicleTuning::FORWARD_ACCELERATION;
    acceleration_ = baseAcceleration * getGearAccelerationMultiplier() * multiplier;
}

void Vehicle::accelerateBackward() noexcept {
    acceleration_ = VehicleTuning::BACKWARD_ACCELERATION;
}

void Vehicle::turn(float amount) noexcept {
    steeringInput_ = amount;

    float turnRate = calculateTurnRate();

    // Reverse the steering when going backwards for a realistic feel
    float turnDirection = (velocity_ >= 0.0f) ? 1.0f : -1.0f;
    rotation_ += amount * VehicleTuning::TURN_SPEED * turnRate * turnDirection;

    if (isDrifting_) {
        // Build up drift angle for bigger slides
        driftAngle_ += amount * VehicleTuning::TURN_SPEED * turnRate * VehicleTuning::DRIFT_ANGLE_MULTIPLIER * turnDirection;
        driftAngle_ = (std::clamp)(driftAngle_, -VehicleTuning::DRIFT_ANGLE_MAX_RADIANS, VehicleTuning::DRIFT_ANGLE_MAX_RADIANS);
    }

    // Keep rotation in [0, 2π]
    rotation_ = std::fmod(rotation_, VehicleTuning::TWO_PI);
    if (rotation_ < 0.0f) {
        rotation_ += VehicleTuning::TWO_PI;
    }
}

float Vehicle::calculateTurnRate() const noexcept {
    const float absoluteVelocity = std::abs(velocity_);

    if (absoluteVelocity < VehicleTuning::MIN_SPEED_THRESHOLD) {
        return 0.0f;
    }

    // Piecewise linear curve - different turn rates at different speeds
    if (absoluteVelocity < VehicleTuning::TURN_RATE_MIN_SPEED) {
        return VehicleTuning::TURN_RATE_EXTREMELY_LOW_BASE +
               ((absoluteVelocity - VehicleTuning::MIN_SPEED_THRESHOLD) / VehicleTuning::TURN_RATE_EXTREMELY_LOW_DIVISOR) *
               VehicleTuning::TURN_RATE_EXTREMELY_LOW_RANGE;
    }

    if (absoluteVelocity < VehicleTuning::TURN_RATE_LOW_SPEED) {
        return VehicleTuning::TURN_RATE_VERY_LOW_BASE +
               ((absoluteVelocity - VehicleTuning::TURN_RATE_MIN_SPEED) / VehicleTuning::TURN_RATE_VERY_LOW_DIVISOR) *
               VehicleTuning::TURN_RATE_VERY_LOW_RANGE;
    }

    if (absoluteVelocity < VehicleTuning::TURN_RATE_MEDIUM_SPEED) {
        return VehicleTuning::TURN_RATE_LOW_MEDIUM_BASE +
               ((absoluteVelocity - VehicleTuning::TURN_RATE_LOW_SPEED) / VehicleTuning::TURN_RATE_LOW_MEDIUM_DIVISOR) *
               VehicleTuning::TURN_RATE_LOW_MEDIUM_RANGE;
    }

    // Less responsive at high speed
    const float speedRatio = (absoluteVelocity - VehicleTuning::TURN_RATE_MEDIUM_SPEED) / (VehicleTuning::MAX_SPEED - VehicleTuning::TURN_RATE_MEDIUM_SPEED);
    const float turnRate = VehicleTuning::TURN_RATE_HIGH_SPEED_BASE - (speedRatio * VehicleTuning::TURN_RATE_HIGH_SPEED_REDUCTION);

    return (std::clamp)(turnRate, VehicleTuning::TURN_RATE_HIGH_SPEED_MIN, VehicleTuning::TURN_RATE_HIGH_SPEED_MAX);
}

void Vehicle::activateNitrous() noexcept {
    if (hasNitrous_ && !nitrousActive_) {
        nitrousActive_ = true;
        nitrousTimeRemaining_ = VehicleTuning::NITROUS_DURATION;
        hasNitrous_ = false;
    }
}

void Vehicle::startDrift() noexcept {
    isDrifting_ = true;
}

void Vehicle::stopDrift() noexcept {
    isDrifting_ = false;
    driftAngle_ *= VehicleTuning::DRIFT_EXIT_RETENTION;
}

bool Vehicle::isDrifting() const noexcept {
    return isDrifting_;
}

void Vehicle::pickupNitrous() noexcept {
    hasNitrous_ = true;
}

bool Vehicle::hasNitrous() const noexcept {
    return hasNitrous_;
}

bool Vehicle::isNitrousActive() const noexcept {
    return nitrousActive_;
}

float Vehicle::getNitrousTimeRemaining() const noexcept {
    return nitrousTimeRemaining_;
}

void Vehicle::update(float deltaTime) {
    updateNitrous(deltaTime);
    updateGearShifting();
    updateVelocity(deltaTime);
    updateRPM();
    updateDrift(deltaTime);
    updatePosition(deltaTime);
    decayAcceleration();
}

void Vehicle::updateNitrous(float deltaTime) noexcept {
    if (nitrousActive_) {
        nitrousTimeRemaining_ -= deltaTime;
        if (nitrousTimeRemaining_ <= 0.0f) {
            nitrousActive_ = false;
            nitrousTimeRemaining_ = 0.0f;
        }
    }
}

void Vehicle::updateVelocity(float deltaTime) noexcept {
    velocity_ += acceleration_ * deltaTime;

    // Logarithmic friction - stronger at low speeds, weaker at high speeds
    float baseFriction = isDrifting_ ? VehicleTuning::DRIFT_FRICTION_COEFFICIENT : VehicleTuning::FRICTION_COEFFICIENT;

    float speedRatio = std::abs(velocity_) / VehicleTuning::MAX_SPEED;
    speedRatio = (std::clamp)(speedRatio, VehicleTuning::FRICTION_MIN_CLAMP, 1.0f);

    float logValue = std::log(speedRatio);
    float frictionRange = VehicleTuning::FRICTION_COEFFICIENT - VehicleTuning::FRICTION_BASE_VALUE;
    float frictionMultiplier = VehicleTuning::FRICTION_BASE_VALUE + ((logValue + VehicleTuning::FRICTION_LOG_OFFSET) / VehicleTuning::FRICTION_LOG_OFFSET) * frictionRange;
    frictionMultiplier = (std::clamp)(frictionMultiplier, VehicleTuning::FRICTION_BASE_VALUE, VehicleTuning::FRICTION_COEFFICIENT);

    float frictionCoefficient = isDrifting_ ? baseFriction : frictionMultiplier;
    velocity_ *= frictionCoefficient;

    float currentMaxSpeed = nitrousActive_ ? VehicleTuning::NITROUS_MAX_SPEED : VehicleTuning::MAX_SPEED;
    velocity_ = std::clamp(velocity_, -VehicleTuning::MAX_REVERSE_SPEED, currentMaxSpeed);
}

void Vehicle::updateRPM() noexcept {
    float absoluteVelocity = std::abs(velocity_);
    if (absoluteVelocity < VehicleTuning::MIN_SPEED_THRESHOLD) {
        rpm_ = VehicleTuning::IDLE_RPM;
    } else if (currentGear_ > 0 && currentGear_ <= VehicleTuning::NUM_GEARS) {
        // RPM based on where we are in the current gear's speed range
        float gearMinSpeed = VehicleTuning::GEAR_SPEEDS[currentGear_ - 1];
        float gearMaxSpeed = VehicleTuning::GEAR_SPEEDS[currentGear_];
        float speedRatio = (absoluteVelocity - gearMinSpeed) / (gearMaxSpeed - gearMinSpeed);
        speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);

        rpm_ = VehicleTuning::GEAR_SHIFT_DOWN_RPM + speedRatio * (VehicleTuning::MAX_RPM - VehicleTuning::GEAR_SHIFT_DOWN_RPM);
    }
}

void Vehicle::updateDrift(float deltaTime) noexcept {
    if (isDrifting_) {
        driftAngle_ *= VehicleTuning::DRIFT_DECAY_RATE;
    }
}

void Vehicle::updatePosition(float deltaTime) noexcept {
    // Car slides at an angle while drifting
    float movementAngle = rotation_;
    if (isDrifting_) {
        movementAngle = rotation_ - driftAngle_;
    }

    const float deltaX = std::sin(movementAngle) * velocity_ * deltaTime;
    const float deltaZ = std::cos(movementAngle) * velocity_ * deltaTime;
    position_[0] += deltaX;
    position_[2] += deltaZ;
}

void Vehicle::decayAcceleration() noexcept {
    acceleration_ = 0.0f;

    // Steering wheel returns to center
    steeringInput_ *= VehicleTuning::STEERING_DECAY_RATE;
    if (std::abs(steeringInput_) < VehicleTuning::STEERING_ZERO_THRESHOLD) {
        steeringInput_ = 0.0f;
    }
}

void Vehicle::reset() noexcept {
    GameObject::reset();
    velocity_ = 0.0f;
    acceleration_ = 0.0f;
    steeringInput_ = 0.0f;
    isDrifting_ = false;
    driftAngle_ = 0.0f;
    hasNitrous_ = false;
    nitrousActive_ = false;
    nitrousTimeRemaining_ = 0.0f;
    currentGear_ = 1;
    rpm_ = VehicleTuning::IDLE_RPM;

    if (resetCameraCallback_) {
        resetCameraCallback_();
    }
}

void Vehicle::setResetCameraCallback(std::function<void()>&& callback) noexcept {
    resetCameraCallback_ = std::move(callback);
}

float Vehicle::getVelocity() const noexcept {
    return velocity_;
}

void Vehicle::setVelocity(float velocity) noexcept {
    const float MAX_VELOCITY = VehicleTuning::MAX_SPEED * VehicleTuning::MAX_VELOCITY_MULTIPLIER;
    velocity_ = std::clamp(velocity, -MAX_VELOCITY, MAX_VELOCITY);
}

float Vehicle::getDriftAngle() const noexcept {
    return driftAngle_;
}

int Vehicle::getCurrentGear() const noexcept {
    return currentGear_;
}

float Vehicle::getRPM() const noexcept {
    return rpm_;
}

float Vehicle::getSteeringInput() const noexcept {
    return steeringInput_;
}

void Vehicle::updateGearShifting() noexcept {
    float absoluteVelocity = std::abs(velocity_);

    if (velocity_ < 0.0f) {
        currentGear_ = 0;
        return;
    }

    // Simple automatic transmission based on speed
    if (absoluteVelocity < VehicleTuning::MIN_SPEED_THRESHOLD) {
        currentGear_ = 1;
        return;
    }

    for (int gear = 1; gear <= VehicleTuning::NUM_GEARS; ++gear) {
        if (absoluteVelocity < VehicleTuning::GEAR_SPEEDS[gear]) {
            currentGear_ = gear;
            return;
        }
    }

    currentGear_ = VehicleTuning::NUM_GEARS;
}

float Vehicle::getGearAccelerationMultiplier() const noexcept {
    if (currentGear_ > 0 && currentGear_ <= VehicleTuning::NUM_GEARS) {
        return VehicleTuning::GEAR_ACCELERATION_MULTIPLIERS[currentGear_ - 1];
    }
    return 1.0f;
}

void Vehicle::setScale(float scale) noexcept {
    scale_ = scale;
}

float Vehicle::getScale() const noexcept {
    return scale_;
}

