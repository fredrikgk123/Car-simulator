#include "core/obstacle.hpp"
#include "core/object_sizes.hpp"
#include "core/game_config.hpp"


Obstacle::Obstacle(float x, float y, float z, ObstacleType type, WallOrientation orientation)
    : GameObject(x, y, z), type_(type), orientation_(orientation) {

    if (type_ == ObstacleType::WALL) {
        if (orientation_ == WallOrientation::HORIZONTAL) {
            size_ = {ObjectSizes::WALL_LENGTH, GameConfig::Obstacle::WALL_HEIGHT, ObjectSizes::WALL_THICKNESS};
        } else {
            size_ = {ObjectSizes::WALL_THICKNESS, GameConfig::Obstacle::WALL_HEIGHT, ObjectSizes::WALL_LENGTH};
        }
    } else if (type_ == ObstacleType::TREE) {
        size_ = {ObjectSizes::TREE_COLLISION_RADIUS * 2.0f, ObjectSizes::TREE_HEIGHT, ObjectSizes::TREE_COLLISION_RADIUS * 2.0f};
    }
}

ObstacleType Obstacle::getType() const noexcept {
    return type_;
}

WallOrientation Obstacle::getOrientation() const noexcept {
    return orientation_;
}

void Obstacle::update(float deltaTime) {
    // Static obstacles require no updates
}
