#include "vehicle.hpp"
#include <cmath>
#include <numbers>

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Physics constants (realistic tuning)
    constexpr float MAX_SPEED = 55.56f;                  // ~200 km/h (increased top speed)
    constexpr float MAX_REVERSE_SPEED = 13.9f;           // ~50 km/h reverse
    constexpr float TURN_SPEED = 1.5f;                   // 1.5 rad/sec (~86°/sec) - sharper turning
    constexpr float FORWARD_ACCELERATION = 8.0f;         // Balanced acceleration
    constexpr float BACKWARD_ACCELERATION = -4.0f;       // Slower reverse acceleration
    constexpr float FRICTION_COEFFICIENT = 0.9985f;      // Lower friction for higher top speed
    constexpr float DRIFT_FRICTION_COEFFICIENT = 0.992f; // Less friction while drifting
    constexpr float MIN_SPEED_THRESHOLD = 0.1f;          // Minimum speed for any turning

    // Nitrous constants
    constexpr float NITROUS_DURATION = 5.0f;             // 5 seconds of boost
    constexpr float NITROUS_ACCELERATION = 14.0f;        // Moderate boost acceleration
    constexpr float NITROUS_MAX_SPEED = 69.44f;          // ~250 km/h during boost (increased)

    // Vehicle dimensions
    constexpr float VEHICLE_WIDTH = 1.0f;
    constexpr float VEHICLE_HEIGHT = 0.5f;
    constexpr float VEHICLE_LENGTH = 2.0f;

    // Turn rate calculation constants
    constexpr float TURN_RATE_MIN_SPEED = 0.3f;          // Speed threshold for minimal turning
    constexpr float TURN_RATE_LOW_SPEED = 3.0f;          // Speed threshold for low-speed turning
    constexpr float TURN_RATE_MEDIUM_SPEED = 15.0f;      // Speed threshold for medium-speed turning
    constexpr float DRIFT_ANGLE_MULTIPLIER = 1.2f;       // Drift angle accumulation rate
    constexpr float DRIFT_EXIT_RETENTION = 0.5f;         // How much drift angle to keep when exiting drift
    constexpr float DRIFT_DECAY_RATE = 0.95f;            // Drift angle decay per frame

    // Gear system constants
    constexpr int NUM_GEARS = 5;                         // 5-speed transmission
    constexpr float GEAR_SHIFT_UP_RPM = 6000.0f;         // Shift up at 6000 RPM
    constexpr float GEAR_SHIFT_DOWN_RPM = 2500.0f;       // Shift down at 2500 RPM
    constexpr float IDLE_RPM = 1000.0f;                  // Engine idle RPM
    constexpr float MAX_RPM = 7000.0f;                   // Redline RPM

    // Speed ranges for each gear (in m/s)
    constexpr float GEAR_SPEEDS[NUM_GEARS + 1] = {
        0.0f,    // Gear 1 starts at 0
        12.0f,   // Gear 2 at ~43 km/h (increased)
        22.0f,   // Gear 3 at ~79 km/h (increased)
        35.0f,   // Gear 4 at ~126 km/h (increased)
        48.0f,   // Gear 5 at ~173 km/h (increased)
        70.0f    // Max speed in gear 5 (~252 km/h)
    };

    // Acceleration multipliers per gear (lower gears = more torque)
    constexpr float GEAR_ACCELERATION_MULTIPLIERS[NUM_GEARS] = {
        1.5f,    // Gear 1: 150% acceleration (lots of torque)
        1.2f,    // Gear 2: 120% acceleration
        1.0f,    // Gear 3: 100% acceleration (base)
        0.8f,    // Gear 4: 80% acceleration
        0.6f     // Gear 5: 60% acceleration (high speed, low torque)
    };

    constexpr float PI = std::numbers::pi_v<float>;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float INITIAL_ROTATION_RADIANS = PI;       // 180 degrees (π radians) for minimap alignment
}

Vehicle::Vehicle(float x, float y, float z)
    : GameObject(x, y, z),
      velocity_(0.0f),
      acceleration_(0.0f),
      isDrifting_(false),
      driftAngle_(0.0f),
      hasNitrous_(false),
      nitrousActive_(false),
      nitrousTimeRemaining_(0.0f),
      currentGear_(1),
      rpm_(IDLE_RPM) {
    // Set vehicle-specific size
    size_[0] = VEHICLE_WIDTH;
    size_[1] = VEHICLE_HEIGHT;
    size_[2] = VEHICLE_LENGTH;

    // Set initial rotation to 180 degrees (π radians) so minimap direction matches
    rotation_ = INITIAL_ROTATION_RADIANS;
    initialRotation_ = INITIAL_ROTATION_RADIANS;
}

void Vehicle::accelerateForward() noexcept {
    float base_acceleration = nitrousActive_ ? NITROUS_ACCELERATION : FORWARD_ACCELERATION;
    // Apply gear acceleration multiplier for more realistic acceleration
    acceleration_ = base_acceleration * getGearAccelerationMultiplier();
}

void Vehicle::accelerateBackward() noexcept {
    acceleration_ = BACKWARD_ACCELERATION;
}

void Vehicle::turn(float amount) noexcept {
    float turn_rate = calculateTurnRate();
    rotation_ += amount * TURN_SPEED * turn_rate;

    // When drifting, allow the car to build up a drift angle
    if (isDrifting_) {
        // Accumulate drift angle more aggressively
        driftAngle_ += amount * TURN_SPEED * turn_rate * DRIFT_ANGLE_MULTIPLIER;

        // Increased max drift angle to ~60 degrees for more dramatic slides
        const float MAX_DRIFT_ANGLE = PI / 3.0f;  // 60 degrees
        driftAngle_ = std::clamp(driftAngle_, -MAX_DRIFT_ANGLE, MAX_DRIFT_ANGLE);
    }

    // Normalize rotation to [0, 2π]
    rotation_ = std::fmod(rotation_, TWO_PI);
    if (rotation_ < 0.0f) {
        rotation_ += TWO_PI;
    }
}

float Vehicle::calculateTurnRate() const noexcept {
    const float absolute_velocity = std::abs(velocity_);

    // Don't turn if completely stopped
    if (absolute_velocity < MIN_SPEED_THRESHOLD) {
        return 0.0f;
    }

    // Extremely low speeds (0.1-0.3 m/s / ~0.4-1.1 km/h): very minimal turning
    if (absolute_velocity < TURN_RATE_MIN_SPEED) {
        return 0.05f + ((absolute_velocity - MIN_SPEED_THRESHOLD) / 0.2f) * 0.1f;
    }

    // Very low speeds (0.3-3 m/s / ~1.1-11 km/h): minimal but usable turning
    if (absolute_velocity < TURN_RATE_LOW_SPEED) {
        return 0.15f + ((absolute_velocity - TURN_RATE_MIN_SPEED) / 2.7f) * 0.35f;
    }

    // Low to medium speeds (3-15 m/s / ~11-54 km/h): good turning capability
    if (absolute_velocity < TURN_RATE_MEDIUM_SPEED) {
        return 0.5f + ((absolute_velocity - TURN_RATE_LOW_SPEED) / 12.0f) * 0.5f;
    }

    // High speeds (15+ m/s / 54+ km/h): reduced turn rate for realism
    const float speed_ratio = (absolute_velocity - TURN_RATE_MEDIUM_SPEED) / (MAX_SPEED - TURN_RATE_MEDIUM_SPEED);
    const float turn_rate = 1.0f - (speed_ratio * 0.4f);  // Reduces to 60% at max speed

    return std::clamp(turn_rate, 0.6f, 1.0f);
}

void Vehicle::activateNitrous() noexcept {
    if (hasNitrous_ && !nitrousActive_) {
        nitrousActive_ = true;
        nitrousTimeRemaining_ = NITROUS_DURATION;
        hasNitrous_ = false; // Consumed when activated
    }
}

void Vehicle::startDrift() noexcept {
    isDrifting_ = true;
}

void Vehicle::stopDrift() noexcept {
    isDrifting_ = false;
    // Keep more of the drift angle when exiting for a smoother transition
    driftAngle_ *= DRIFT_EXIT_RETENTION;
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
    // Update nitrous timer
    if (nitrousActive_) {
        nitrousTimeRemaining_ -= deltaTime;
        if (nitrousTimeRemaining_ <= 0.0f) {
            nitrousActive_ = false;
            nitrousTimeRemaining_ = 0.0f;
        }
    }

    // Update gear shifting based on current speed
    updateGearShifting();

    // Update velocity based on acceleration
    velocity_ += acceleration_ * deltaTime;

    // Apply friction (less friction while drifting)
    // Logarithmic friction curve: more friction at low speeds, less at high speeds
    float baseFriction = isDrifting_ ? DRIFT_FRICTION_COEFFICIENT : FRICTION_COEFFICIENT;

    // Calculate friction multiplier using logarithmic curve
    // At low speeds: higher friction (slower deceleration from friction)
    // At high speeds: lower friction (allows reaching top speed)
    float speedRatio = std::abs(velocity_) / MAX_SPEED;
    speedRatio = std::clamp(speedRatio, 0.01f, 1.0f); // Prevent log(0)

    // Logarithmic curve: friction increases as speed decreases
    // log(0.01) = -4.6, log(1.0) = 0
    // Map to range [0.994, 0.9985] for normal driving
    float logValue = std::log(speedRatio);
    float frictionRange = 0.9985f - 0.994f; // 0.0045
    float frictionMultiplier = 0.994f + ((logValue + 4.6f) / 4.6f) * frictionRange;
    frictionMultiplier = std::clamp(frictionMultiplier, 0.994f, 0.9985f);

    float friction_coefficient = isDrifting_ ? baseFriction : frictionMultiplier;
    velocity_ *= friction_coefficient;

    // Clamp velocity to max speeds (higher during nitrous)
    float current_max_speed = nitrousActive_ ? NITROUS_MAX_SPEED : MAX_SPEED;
    velocity_ = std::clamp(velocity_, -MAX_REVERSE_SPEED, current_max_speed);

    // Update RPM based on speed and current gear
    float absolute_velocity = std::abs(velocity_);
    if (absolute_velocity < 0.1f) {
        // Idle RPM when stopped
        rpm_ = IDLE_RPM;
    } else if (currentGear_ > 0 && currentGear_ <= NUM_GEARS) {
        // Calculate RPM based on speed within current gear's range
        float gear_min_speed = GEAR_SPEEDS[currentGear_ - 1];
        float gear_max_speed = GEAR_SPEEDS[currentGear_];
        float speed_ratio = (absolute_velocity - gear_min_speed) / (gear_max_speed - gear_min_speed);
        speed_ratio = std::clamp(speed_ratio, 0.0f, 1.0f);

        // Map speed ratio to RPM range (shift point to max RPM)
        rpm_ = GEAR_SHIFT_DOWN_RPM + speed_ratio * (MAX_RPM - GEAR_SHIFT_DOWN_RPM);
    }

    // When drifting, car moves in a direction between facing and drift angle
    float movement_angle = rotation_;
    if (isDrifting_) {
        movement_angle = rotation_ - driftAngle_;
        // Gradually reduce drift angle over time (self-correcting)
        driftAngle_ *= DRIFT_DECAY_RATE;
    }

    // Update position based on velocity and movement angle
    const float delta_x = std::sin(movement_angle) * velocity_ * deltaTime;
    const float delta_z = std::cos(movement_angle) * velocity_ * deltaTime;
    position_[0] += delta_x;
    position_[2] += delta_z;

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
    currentGear_ = 1;
    rpm_ = IDLE_RPM;

    // Reset camera to follow mode
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

float Vehicle::getMaxSpeed() const noexcept {
    return MAX_SPEED;
}

void Vehicle::setVelocity(float velocity) noexcept {
    velocity_ = velocity;
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

void Vehicle::updateGearShifting() noexcept {
    float absolute_velocity = std::abs(velocity_);

    // Don't shift during reverse
    if (velocity_ < 0.0f) {
        currentGear_ = 0; // Reverse gear
        return;
    }

    // Start in gear 1 when moving forward from stop
    if (absolute_velocity < 0.1f) {
        currentGear_ = 1;
        return;
    }

    // Automatic gear shifting based on speed
    // Shift up when reaching the upper speed threshold for current gear
    if (currentGear_ < NUM_GEARS && absolute_velocity >= GEAR_SPEEDS[currentGear_]) {
        currentGear_++;
    }
    // Shift down when falling below the lower speed threshold
    else if (currentGear_ > 1 && absolute_velocity < GEAR_SPEEDS[currentGear_ - 1]) {
        currentGear_--;
    }
}

float Vehicle::getGearAccelerationMultiplier() const noexcept {
    // Return multiplier based on current gear (lower gears = more torque)
    if (currentGear_ >= 1 && currentGear_ <= NUM_GEARS) {
        return GEAR_ACCELERATION_MULTIPLIERS[currentGear_ - 1];
    }
    return 1.0f; // Default multiplier for reverse or invalid gear
}
