#pragma once

/**
 * Interface for player input.
 * Decouples input handling from the actual entity being controlled.
 */
class IControllable {
public:
    virtual ~IControllable() = default;

    virtual void accelerateForward() noexcept = 0;
    virtual void accelerateBackward() noexcept = 0;
    virtual void turn(float amount) noexcept = 0;

    virtual void startDrift() noexcept = 0;
    virtual void stopDrift() noexcept = 0;
    virtual void activateNitrous() noexcept = 0;

    virtual void reset() noexcept = 0;
};
