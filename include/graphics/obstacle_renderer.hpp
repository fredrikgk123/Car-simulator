#pragma once

#include "graphics/game_object_renderer.hpp"
#include "core/obstacle.hpp"

/**
 * Renders obstacles (walls and trees) with appropriate 3D models.
 */
class ObstacleRenderer : public GameObjectRenderer {
public:
    ObstacleRenderer(threepp::Scene& scene, const Obstacle& obstacle);

    void update() override;

protected:
    void createModel() override;

private:
    void createWallMesh();
    void createTreeMesh();

    const Obstacle& obstacle_;
};
