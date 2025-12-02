#pragma once

/**
 * Read-only access to vehicle state.
 * Lets renderers/UI/audio query the vehicle without direct coupling.
 */
class IVehicleState {
public:
    virtual ~IVehicleState() = default;

    [[nodiscard]] virtual float getScale() const noexcept = 0;

    [[nodiscard]] virtual float getVelocity() const noexcept = 0;
    [[nodiscard]] virtual float getSteeringInput() const noexcept = 0;

    [[nodiscard]] virtual bool isDrifting() const noexcept = 0;
    [[nodiscard]] virtual float getDriftAngle() const noexcept = 0;

    [[nodiscard]] virtual bool hasNitrous() const noexcept = 0;
    [[nodiscard]] virtual bool isNitrousActive() const noexcept = 0;
    [[nodiscard]] virtual float getNitrousTimeRemaining() const noexcept = 0;

    [[nodiscard]] virtual int getCurrentGear() const noexcept = 0;
    [[nodiscard]] virtual float getRPM() const noexcept = 0;
};
