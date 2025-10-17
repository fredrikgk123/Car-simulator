#include "game_object.hpp"
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
      active_(true) {
}

void GameObject::reset() {
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
    // Simple circle-to-circle collision using the average of width and length as radius
    // This works well for both square-ish objects (trees) and rectangular objects (vehicles)

    float this_radius = (size_[0] + size_[2]) / 4.0f;  // Average of width and length, divided by 2
    float other_radius = (other.size_[0] + other.size_[2]) / 4.0f;

    // Calculate distance between centers
    float distance_x = other.position_[0] - position_[0];
    float distance_z = other.position_[2] - position_[2];
    float distance_squared = distance_x * distance_x + distance_z * distance_z;
    float distance = std::sqrt(distance_squared);

    // Check collision
    float radius_sum = this_radius + other_radius;

    if (distance < radius_sum && distance > MIN_DISTANCE_EPSILON) {
        // Calculate overlap and normal
        overlapDistance = radius_sum - distance;
        normalX = distance_x / distance;
        normalZ = distance_z / distance;
        return true;
    }

    // Handle case where objects are at same position
    if (distance <= MIN_DISTANCE_EPSILON) {
        overlapDistance = radius_sum;
        normalX = 1.0f;
        normalZ = 0.0f;
        return true;
    }

    return false;
}

bool GameObject::intersects(const GameObject& other) const noexcept {
    // Simple wrapper around checkCircleCollision for convenience
    float overlap_distance, normal_x, normal_z;
    return checkCircleCollision(other, overlap_distance, normal_x, normal_z);
}
