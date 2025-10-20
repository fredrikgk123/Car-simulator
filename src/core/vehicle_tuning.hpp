// filepath: /Users/fredrikkarlsaune/Desktop/dev/bilsim/src/core/vehicle_tuning.hpp
#pragma once

#include <numbers>

namespace VehicleTuning {

// Physics constants (realistic tuning)
inline constexpr float MAX_SPEED = 55.56f;                  // ~200 km/h (increased top speed)
inline constexpr float MAX_REVERSE_SPEED = 13.9f;           // ~50 km/h reverse
inline constexpr float TURN_SPEED = 1.5f;                   // 1.5 rad/sec (~86°/sec) - sharper turning
inline constexpr float FORWARD_ACCELERATION = 8.0f;         // Balanced acceleration
inline constexpr float BACKWARD_ACCELERATION = -4.0f;       // Slower reverse acceleration
inline constexpr float FRICTION_COEFFICIENT = 0.9985f;      // Lower friction for higher top speed
inline constexpr float DRIFT_FRICTION_COEFFICIENT = 0.992f; // Less friction while drifting
inline constexpr float MIN_SPEED_THRESHOLD = 0.1f;          // Minimum speed for any turning

// Nitrous constants
inline constexpr float NITROUS_DURATION = 5.0f;             // 5 seconds of boost
inline constexpr float NITROUS_ACCELERATION = 14.0f;        // Moderate boost acceleration
inline constexpr float NITROUS_MAX_SPEED = 69.44f;          // ~180 km/h during boost (increased)

// Vehicle dimensions
inline constexpr float VEHICLE_WIDTH = 1.0f;
inline constexpr float VEHICLE_HEIGHT = 0.5f;
inline constexpr float VEHICLE_LENGTH = 2.0f;

// Turn rate calculation constants
inline constexpr float TURN_RATE_MIN_SPEED = 0.3f;          // Speed threshold for minimal turning
inline constexpr float TURN_RATE_LOW_SPEED = 3.0f;          // Speed threshold for low-speed turning
inline constexpr float TURN_RATE_MEDIUM_SPEED = 15.0f;      // Speed threshold for medium-speed turning
inline constexpr float DRIFT_ANGLE_MULTIPLIER = 1.2f;       // Drift angle accumulation rate
inline constexpr float DRIFT_EXIT_RETENTION = 0.5f;         // How much drift angle to keep when exiting drift
inline constexpr float DRIFT_DECAY_RATE = 0.95f;            // Drift angle decay per frame

// Gear system constants
inline constexpr int NUM_GEARS = 5;                         // 5-speed transmission
inline constexpr float GEAR_SHIFT_UP_RPM = 6000.0f;         // Shift up at 6000 RPM
inline constexpr float GEAR_SHIFT_DOWN_RPM = 2500.0f;       // Shift down at 2500 RPM
inline constexpr float IDLE_RPM = 1000.0f;                  // Engine idle RPM
inline constexpr float MAX_RPM = 7000.0f;                   // Redline RPM

// Speed ranges for each gear (in m/s)
inline constexpr float GEAR_SPEEDS[NUM_GEARS + 1] = {
    0.0f,    // Gear 1 starts at 0
    12.0f,   // Gear 2 at ~43 km/h (increased)
    22.0f,   // Gear 3 at ~79 km/h (increased)
    35.0f,   // Gear 4 at ~126 km/h (increased)
    48.0f,   // Gear 5 at ~173 km/h (increased)
    70.0f    // Max speed in gear 5 (~252 km/h)
};

// Acceleration multipliers per gear (lower gears = more torque)
inline constexpr float GEAR_ACCELERATION_MULTIPLIERS[NUM_GEARS] = {
    1.5f,    // Gear 1: 150% acceleration (lots of torque)
    1.2f,    // Gear 2: 120% acceleration
    1.0f,    // Gear 3: 100% acceleration (base)
    0.8f,    // Gear 4: 80% acceleration
    0.6f     // Gear 5: 60% acceleration (high speed, low torque)
};

inline constexpr float PI = std::numbers::pi_v<float>;
inline constexpr float TWO_PI = 2.0f * PI;
inline constexpr float INITIAL_ROTATION_RADIANS = PI;       // 180 degrees (π radians) for minimap alignment

} // namespace VehicleTuning

