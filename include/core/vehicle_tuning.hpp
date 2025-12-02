#pragma once

#include <numbers>
#include "object_sizes.hpp"

/**
 * Physics constants for vehicle behavior.
 * Tuned empirically for realistic feel.
 */
namespace VehicleTuning {

inline constexpr float PI = std::numbers::pi_v<float>;
inline constexpr float TWO_PI = 2.0f * PI;
inline constexpr float INITIAL_ROTATION_RADIANS = PI;

// Speed and acceleration (m/s)
inline constexpr float MAX_SPEED = 55.56f;
inline constexpr float MAX_REVERSE_SPEED = 13.9f;
inline constexpr float TURN_SPEED = 1.5f;
inline constexpr float FORWARD_ACCELERATION = 7.8f;
inline constexpr float BACKWARD_ACCELERATION = -4.0f;
inline constexpr float FRICTION_COEFFICIENT = 0.9982f;
inline constexpr float DRIFT_FRICTION_COEFFICIENT = 0.992f;
inline constexpr float MIN_SPEED_THRESHOLD = 0.1f;

// Steering
inline constexpr float STEERING_DECAY_RATE = 0.85f;
inline constexpr float STEERING_ZERO_THRESHOLD = 0.01f;

// Drift mechanics
inline constexpr float DRIFT_ANGLE_MAX_RADIANS = PI / 3.0f;  // 60 degrees
inline constexpr float DRIFT_ANGLE_MULTIPLIER = 1.2f;
inline constexpr float DRIFT_EXIT_RETENTION = 0.5f;
inline constexpr float DRIFT_DECAY_RATE = 0.95f;

inline constexpr float MAX_VELOCITY_MULTIPLIER = 1.5f;
inline constexpr float DEFAULT_SCALE = 1.0f;

// Logarithmic friction for realistic deceleration
inline constexpr float FRICTION_MIN_CLAMP = 0.01f;
inline constexpr float FRICTION_BASE_VALUE = 0.994f;
inline constexpr float FRICTION_LOG_OFFSET = 4.6f;

// Nitrous
inline constexpr float NITROUS_DURATION = 5.0f;
inline constexpr float NITROUS_ACCELERATION = 14.0f;
inline constexpr float NITROUS_MAX_SPEED = 69.44f;

// Vehicle size
inline constexpr float VEHICLE_WIDTH = ObjectSizes::VEHICLE_WIDTH;
inline constexpr float VEHICLE_HEIGHT = ObjectSizes::VEHICLE_HEIGHT;
inline constexpr float VEHICLE_LENGTH = ObjectSizes::VEHICLE_LENGTH;

// Turn rate varies with speed
inline constexpr float TURN_RATE_MIN_SPEED = 0.3f;
inline constexpr float TURN_RATE_LOW_SPEED = 3.0f;
inline constexpr float TURN_RATE_MEDIUM_SPEED = 15.0f;

// Piecewise curve parameters
inline constexpr float TURN_RATE_EXTREMELY_LOW_BASE = 0.05f;
inline constexpr float TURN_RATE_EXTREMELY_LOW_RANGE = 0.1f;
inline constexpr float TURN_RATE_EXTREMELY_LOW_DIVISOR = 0.2f;

inline constexpr float TURN_RATE_VERY_LOW_BASE = 0.15f;
inline constexpr float TURN_RATE_VERY_LOW_RANGE = 0.35f;
inline constexpr float TURN_RATE_VERY_LOW_DIVISOR = 2.7f;

inline constexpr float TURN_RATE_LOW_MEDIUM_BASE = 0.5f;
inline constexpr float TURN_RATE_LOW_MEDIUM_RANGE = 0.5f;
inline constexpr float TURN_RATE_LOW_MEDIUM_DIVISOR = 12.0f;

inline constexpr float TURN_RATE_HIGH_SPEED_BASE = 1.0f;
inline constexpr float TURN_RATE_HIGH_SPEED_REDUCTION = 0.4f;
inline constexpr float TURN_RATE_HIGH_SPEED_MIN = 0.6f;
inline constexpr float TURN_RATE_HIGH_SPEED_MAX = 1.0f;

// Gear system
inline constexpr int NUM_GEARS = 4;
inline constexpr float GEAR_SHIFT_UP_RPM = 6000.0f;
inline constexpr float GEAR_SHIFT_DOWN_RPM = 2500.0f;
inline constexpr float IDLE_RPM = 1000.0f;
inline constexpr float MAX_RPM = 7000.0f;

inline constexpr float GEAR_SPEEDS[NUM_GEARS + 1] = {
    0.0f, 10.0f, 20.0f, 45.0f, 70.0f
};

inline constexpr float GEAR_ACCELERATION_MULTIPLIERS[NUM_GEARS] = {
    1.5f, 1.2f, 1.0f, 0.8f
};

static_assert(sizeof(GEAR_SPEEDS) / sizeof(GEAR_SPEEDS[0]) == NUM_GEARS + 1);
static_assert(sizeof(GEAR_ACCELERATION_MULTIPLIERS) / sizeof(GEAR_ACCELERATION_MULTIPLIERS[0]) == NUM_GEARS);

} // namespace VehicleTuning
