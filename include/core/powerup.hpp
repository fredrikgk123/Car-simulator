#pragma once

#include "core/game_object.hpp"

enum class PowerupType {
    NITROUS
};

/**
 * Collectible powerups (nitrous boost).
 * Static cylindrical objects that can be collected by the vehicle.
 */
class Powerup : public GameObject {
public:
    Powerup(float x, float y, float z, PowerupType type);

    void update(float deltaTime) override;

    [[nodiscard]] PowerupType getType() const noexcept;

private:
    PowerupType type_;
};
