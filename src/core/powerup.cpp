#include "core/powerup.hpp"
#include "core/object_sizes.hpp"

Powerup::Powerup(float x, float y, float z, PowerupType type)
    : GameObject(x, y, z),
      type_(type) {
    size_[0] = ObjectSizes::POWERUP_SIZE;
    size_[1] = ObjectSizes::POWERUP_SIZE;
    size_[2] = ObjectSizes::POWERUP_SIZE;
}

void Powerup::update(float deltaTime) {
    // Powerups are static objects - no animation needed
}

PowerupType Powerup::getType() const noexcept {
    return type_;
}
