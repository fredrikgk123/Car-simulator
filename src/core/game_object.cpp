#include "game_object.hpp"
#include <cmath>
#include <algorithm>

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

const std::array<float, 3>& GameObject::getPosition() const {
    return position_;
}

float GameObject::getRotation() const {
    return rotation_;
}

const std::array<float, 3>& GameObject::getSize() const {
    return size_;
}

bool GameObject::isActive() const {
    return active_;
}

void GameObject::setPosition(float x, float y, float z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
}

void GameObject::setRotation(float rotation) {
    rotation_ = rotation;
}

void GameObject::setActive(bool active) {
    active_ = active;
}

bool GameObject::checkCircleCollision(const GameObject& other, float& overlapDistance, float& normalX, float& normalZ) const {
    // Simple circle-to-circle collision using the average of width and length as radius
    // This works well for both square-ish objects (trees) and rectangular objects (vehicles)

    float thisRadius = (size_[0] + size_[2]) / 4.0f;  // Average of width and length, divided by 2
    float otherRadius = (other.size_[0] + other.size_[2]) / 4.0f;

    // Calculate distance between centers
    float dx = other.position_[0] - position_[0];
    float dz = other.position_[2] - position_[2];
    float distanceSquared = dx * dx + dz * dz;
    float distance = std::sqrt(distanceSquared);

    // Check collision
    float radiusSum = thisRadius + otherRadius;

    if (distance < radiusSum && distance > 0.001f) {
        // Calculate overlap and normal
        overlapDistance = radiusSum - distance;
        normalX = dx / distance;
        normalZ = dz / distance;
        return true;
    }

    // Handle case where objects are at same position
    if (distance <= 0.001f) {
        overlapDistance = radiusSum;
        normalX = 1.0f;
        normalZ = 0.0f;
        return true;
    }

    return false;
}

bool GameObject::intersects(const GameObject& other) const {
    // Simple wrapper around checkCircleCollision for convenience
    float overlapDistance, normalX, normalZ;
    return checkCircleCollision(other, overlapDistance, normalX, normalZ);
}
