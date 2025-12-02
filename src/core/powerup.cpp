#include "core/powerup.hpp"
#include "core/object_sizes.hpp"
#include "core/vehicle_tuning.hpp"
#include <cmath>

namespace {
    constexpr float POWERUP_ROTATION_SPEED = 2.0f;
}

Powerup::Powerup(float x, float y, float z, PowerupType type)
    : GameObject(x, y, z),
      type_(type),
      rotationSpeed_(POWERUP_ROTATION_SPEED) {
    size_[0] = ObjectSizes::POWERUP_SIZE;
    size_[1] = ObjectSizes::POWERUP_SIZE;
    size_[2] = ObjectSizes::POWERUP_SIZE;
}

void Powerup::update(float deltaTime) {
    // Continuous rotation for visual appeal
    rotation_ += rotationSpeed_ * deltaTime;
    rotation_ = std::fmod(rotation_, VehicleTuning::TWO_PI);
}

PowerupType Powerup::getType() const noexcept {
    return type_;
}
