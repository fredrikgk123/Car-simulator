#pragma once

#include <threepp/threepp.hpp>
#include <memory>
#include "core/obstacle.hpp"

/**
 * ObstacleRenderer - renders obstacles (walls and trees) in the scene
 * Source: Inspired by threepp examples and three.js documentation
 * https://github.com/markaren/threepp
 */
class ObstacleRenderer {
public:
    ObstacleRenderer(threepp::Scene& scene, const Obstacle& obstacle);
    ~ObstacleRenderer() = default;

    // Update visual representation (called each frame if needed)
    void update();

private:
    void createWallMesh();
    void createTreeMesh();

    const Obstacle& obstacle_;
    threepp::Scene& scene_;
    std::shared_ptr<threepp::Group> obstacleGroup_;
};

