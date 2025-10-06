#pragma once

#include <array>
#include <cmath>

class Vehicle {
public:
    // Constructor - takes x, y, z starting position
    Vehicle(float x = 0.0f, float y = 0.0f, float z = 0.0f);

    // Control methods
    void accelerateForward();
    void accelerateBackward();
    void turn(float amount);
    void update(float deltaTime);
    void reset();

    // Getters
    const std::array<float, 3>& getPosition() const;
    float getRotation() const;
    const std::array<float, 3>& getSize() const;
    float getVelocity() const;
    float getMaxSpeed() const { return MAX_SPEED; }

    // Constants - public for use by other systems (e.g., audio, UI)
    static const float MAX_SPEED;              // Maximum forward speed (units/sec)
    static const float MAX_REVERSE_SPEED;      // Maximum reverse speed (units/sec)

private:
    // Calculate turn rate based on current speed
    float calculateTurnRate() const;

    // Physics constants
    static const float TURN_SPEED;              // Base turn rate (radians/sec)
    static const float FORWARD_ACCELERATION;    // Forward acceleration (units/sec²)
    static const float BACKWARD_ACCELERATION;   // Reverse acceleration (units/sec²)
    static const float FRICTION_COEFFICIENT;    // Per-frame friction multiplier
    static const float MIN_TURN_SPEED;          // Minimum speed required to turn

    // Vehicle dimensions
    static const float VEHICLE_WIDTH;
    static const float VEHICLE_HEIGHT;
    static const float VEHICLE_LENGTH;

    std::array<float, 3> position_;           // x, y, z coordinates
    std::array<float, 3> initialPosition_;    // For reset functionality
    float rotation_;                          // In radians
    float velocity_;                          // Current speed
    float acceleration_;                      // Current acceleration
};
