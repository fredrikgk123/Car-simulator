#pragma once

#include "core/game_object.hpp"

enum class ObstacleType {
    WALL,
    TREE
};

enum class WallOrientation {
    HORIZONTAL,  // X-axis (North/South)
    VERTICAL     // Z-axis (East/West)
};

/**
 * Static obstacles: walls around the perimeter and trees scattered inside.
 */
class Obstacle : public GameObject {
public:
    Obstacle(float x, float y, float z, ObstacleType type, WallOrientation orientation = WallOrientation::HORIZONTAL);

    [[nodiscard]] ObstacleType getType() const noexcept;
    [[nodiscard]] WallOrientation getOrientation() const noexcept;

    void update(float deltaTime) override; // no-op for static objects

private:
    ObstacleType type_;
    WallOrientation orientation_;
};
