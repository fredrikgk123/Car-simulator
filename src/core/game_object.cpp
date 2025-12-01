#include "core/game_object.hpp"
#include <cmath>
#include <algorithm>

namespace {
    // Collision detection constants
    constexpr float MIN_DISTANCE_EPSILON = 0.001f;  // Minimum distance threshold for collision calculations
}

GameObject::GameObject(float x, float y, float z)
    : position_({x, y, z}),
      initialPosition_({x, y, z}),
      rotation_(0.0f),
      initialRotation_(0.0f),
      size_({1.0f, 1.0f, 1.0f}),
      collisionRadius_(0.0f),
      active_(true) {
    updateCollisionRadius();
}

void GameObject::updateCollisionRadius() noexcept {
    // Calculate radius = sqrt((width/2)^2 + (length/2)^2) to account for rotation
    const float halfWidth = size_[0] / 2.0f;
    const float halfLength = size_[2] / 2.0f;
    collisionRadius_ = std::sqrt(halfWidth * halfWidth + halfLength * halfLength);
}

void GameObject::reset() noexcept {
    position_ = initialPosition_;
    rotation_ = initialRotation_;
    active_ = true;
}

const std::array<float, 3>& GameObject::getPosition() const noexcept {
    return position_;
}

float GameObject::getRotation() const noexcept {
    return rotation_;
}

const std::array<float, 3>& GameObject::getSize() const noexcept {
    return size_;
}

bool GameObject::isActive() const noexcept {
    return active_;
}

void GameObject::setPosition(float x, float y, float z) noexcept {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
}

void GameObject::setRotation(float rotation) noexcept {
    rotation_ = rotation;
}

void GameObject::setActive(bool active) noexcept {
    active_ = active;
}

bool GameObject::checkCircleCollision(const GameObject& other, float& overlapDistance, float& normalX, float& normalZ) const noexcept {
    // Use cached collision radii instead of recalculating
    const float radiusSum = collisionRadius_ + other.collisionRadius_;

    // Calculate distance between centers
    const float distanceX = other.position_[0] - position_[0];
    const float distanceZ = other.position_[2] - position_[2];
    const float distanceSquared = distanceX * distanceX + distanceZ * distanceZ;

    // Early exit using squared distance to avoid unnecessary sqrt
    const float radiusSumSquared = radiusSum * radiusSum;
    if (distanceSquared > radiusSumSquared) {
        return false;  // No collision
    }

    // Only calculate sqrt when we know there's a collision
    const float distance = std::sqrt(distanceSquared);

    // Handle case where objects are at same position
    if (distance <= MIN_DISTANCE_EPSILON) {
        overlapDistance = radiusSum;
        normalX = 1.0f;
        normalZ = 0.0f;
        return true;
    }

    // Calculate overlap and normal
    overlapDistance = radiusSum - distance;
    normalX = distanceX / distance;
    normalZ = distanceZ / distance;
    return true;
}

bool GameObject::intersects(const GameObject& other) const noexcept {
    // Simple wrapper around checkCircleCollision for convenience
    float overlapDistance, normalX, normalZ;
    return checkCircleCollision(other, overlapDistance, normalX, normalZ);
}
