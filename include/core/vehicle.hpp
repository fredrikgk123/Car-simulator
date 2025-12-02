#pragma once

#include <functional>
#include "core/game_object.hpp"
#include "core/interfaces/IVehicleState.hpp"
#include "core/interfaces/IControllable.hpp"
#include "core/vehicle_tuning.hpp"
#include <algorithm>

/**
 * Player's vehicle with realistic physics.
 * Features gear shifting, drift mechanics, nitrous boost, and RPM simulation.
 */
class Vehicle : public GameObject, public IVehicleState, public IControllable {
public:
    explicit Vehicle(float x = 0.0f, float y = 0.0f, float z = 0.0f);

    // Control interface
    void accelerateForward() noexcept override;
    void accelerateForward(float multiplier) noexcept; // with custom multiplier
    void accelerateBackward() noexcept override;
    void turn(float amount) noexcept override;

    // Drift controls
    void startDrift() noexcept override;
    void stopDrift() noexcept override;
    [[nodiscard]] bool isDrifting() const noexcept override;

    // Nitrous boost
    void activateNitrous() noexcept override;
    void pickupNitrous() noexcept;
    [[nodiscard]] bool hasNitrous() const noexcept override;
    [[nodiscard]] bool isNitrousActive() const noexcept override;
    [[nodiscard]] float getNitrousTimeRemaining() const noexcept override;

    void update(float deltaTime) override;
    void reset() noexcept override;

    // State getters
    [[nodiscard]] float getVelocity() const noexcept override;
    [[nodiscard]] static constexpr float getMaxSpeed() noexcept { return VehicleTuning::MAX_SPEED; }
    [[nodiscard]] float getDriftAngle() const noexcept override;
    [[nodiscard]] int getCurrentGear() const noexcept override;
    [[nodiscard]] float getRPM() const noexcept override;
    [[nodiscard]] float getSteeringInput() const noexcept override;

    void setVelocity(float velocity) noexcept;

    // Scaling affects both visuals and collision
    void setScale(float scale) noexcept;
    [[nodiscard]] float getScale() const noexcept override;

    // For UI tuning controls
    void setAccelerationMultiplier(float m) noexcept {
        accelMultiplier_ = std::clamp(m, 0.1f, 5.0f);
    }
    [[nodiscard]] float getAccelerationMultiplier() const noexcept { return accelMultiplier_; }

    void setResetCameraCallback(std::function<void()> &&callback) noexcept;

private:
    // Speed-dependent steering feel
    [[nodiscard]] float calculateTurnRate() const noexcept;

    // Automatic transmission
    void updateGearShifting() noexcept;
    [[nodiscard]] float getGearAccelerationMultiplier() const noexcept;

    // Update split into smaller pieces
    void updateNitrous(float deltaTime) noexcept;
    void updateVelocity(float deltaTime) noexcept;
    void updateRPM() noexcept;
    void updateDrift(float deltaTime) noexcept;
    void updatePosition(float deltaTime) noexcept;
    void decayAcceleration() noexcept;

    float velocity_;
    float acceleration_;
    float steeringInput_;

    bool isDrifting_;
    float driftAngle_;

    bool hasNitrous_;
    bool nitrousActive_;
    float nitrousTimeRemaining_;

    int currentGear_;
    float rpm_;

    float scale_ = 1.0f;
    float accelMultiplier_ = 1.0f;

    std::function<void()> resetCameraCallback_;
};
