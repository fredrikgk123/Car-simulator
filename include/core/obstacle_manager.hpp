#pragma once

#include "core/obstacle.hpp"
#include "core/vehicle.hpp"
#include "core/game_object_manager.hpp"
#include "core/random_position_generator.hpp"
#include <vector>
#include <memory>

/**
 * Manages all obstacles in the scene.
 * Generates perimeter walls and randomly positioned trees with proper spacing.
 */
class ObstacleManager : public GameObjectManager {
public:
    ObstacleManager(float playAreaSize, int treeCount);

    void update(float deltaTime) override;
    void handleCollisions(Vehicle& vehicle) override;
    void reset() noexcept override;

    [[nodiscard]] const std::vector<std::unique_ptr<Obstacle>>& getObstacles() const noexcept;
    [[nodiscard]] size_t getCount() const noexcept override;

private:
    void generateWalls(float playAreaSize);
    void generateTrees(int count, float playAreaSize);

    std::vector<std::unique_ptr<Obstacle>> obstacles_;
};
