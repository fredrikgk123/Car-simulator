#include "core/obstacle_manager.hpp"
#include "core/game_config.hpp"
#include "core/random_position_generator.hpp"
#include <cmath>


ObstacleManager::ObstacleManager(float playAreaSize, int treeCount) {
    const int segmentsPerSide = static_cast<int>(playAreaSize / GameConfig::Obstacle::WALL_SEGMENT_LENGTH);
    obstacles_.reserve(segmentsPerSide * 4 + treeCount);

    generateWalls(playAreaSize);
    generateTrees(treeCount, playAreaSize);
}

void ObstacleManager::update(float deltaTime) {
    // Nothing to do - obstacles don't move
}

void ObstacleManager::generateWalls(float playAreaSize) {
    const float halfSize = playAreaSize / 2.0f;
    const int segmentsPerSide = static_cast<int>(playAreaSize / GameConfig::Obstacle::WALL_SEGMENT_LENGTH);

    // Build walls around the perimeter
    for (int i = 0; i < segmentsPerSide; ++i) {
        float offset = -halfSize + (static_cast<float>(i) * GameConfig::Obstacle::WALL_SEGMENT_LENGTH) + (GameConfig::Obstacle::WALL_SEGMENT_LENGTH / 2.0f);

        obstacles_.push_back(std::make_unique<Obstacle>(offset, GameConfig::Obstacle::WALL_HEIGHT, -halfSize, ObstacleType::WALL, WallOrientation::HORIZONTAL));
        obstacles_.push_back(std::make_unique<Obstacle>(offset, GameConfig::Obstacle::WALL_HEIGHT, halfSize, ObstacleType::WALL, WallOrientation::HORIZONTAL));
        obstacles_.push_back(std::make_unique<Obstacle>(-halfSize, GameConfig::Obstacle::WALL_HEIGHT, offset, ObstacleType::WALL, WallOrientation::VERTICAL));
        obstacles_.push_back(std::make_unique<Obstacle>(halfSize, GameConfig::Obstacle::WALL_HEIGHT, offset, ObstacleType::WALL, WallOrientation::VERTICAL));
    }
}

void ObstacleManager::generateTrees(int count, float playAreaSize) {
    RandomPositionGenerator posGen(playAreaSize, GameConfig::Obstacle::MIN_TREE_DISTANCE_FROM_WALL);

    std::vector<std::array<float, 2>> treePositions;

    int treesPlaced = 0;
    int totalAttempts = 0;
    const int maxTotalAttempts = count * 20;

    while (treesPlaced < count && totalAttempts < maxTotalAttempts) {
        totalAttempts++;

        auto pos = posGen.getRandomPosition();

        // Don't spawn too close to the center (player spawn)
        float distanceFromCenter = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
        if (distanceFromCenter < GameConfig::Obstacle::MIN_TREE_DISTANCE_FROM_CENTER) {
            continue;
        }

        // Make sure there's space between trees
        bool validPosition = true;
        for (const auto& existingPos : treePositions) {
            float dx = pos[0] - existingPos[0];
            float dz = pos[1] - existingPos[1];
            float distance = std::sqrt(dx * dx + dz * dz);

            if (distance < GameConfig::Obstacle::MIN_DISTANCE_BETWEEN_TREES) {
                validPosition = false;
                break;
            }
        }

        if (validPosition) {
            obstacles_.push_back(std::make_unique<Obstacle>(
                pos[0],
                GameConfig::Obstacle::TREE_HEIGHT,
                pos[1],
                ObstacleType::TREE
            ));
            treePositions.push_back(pos);
            treesPlaced++;
        }
    }
}

void ObstacleManager::handleCollisions(Vehicle& vehicle) {
    for (const auto& obstacle : obstacles_) {
        float overlapDistance, normalX, normalZ;

        if (vehicle.checkCircleCollision(*obstacle, overlapDistance, normalX, normalZ)) {
            // Push the vehicle out
            const auto& vehiclePos = vehicle.getPosition();
            vehicle.setPosition(
                vehiclePos[0] - normalX * overlapDistance,
                vehiclePos[1],
                vehiclePos[2] - normalZ * overlapDistance
            );

            vehicle.setVelocity(0.0f);

            // Only handle one collision per frame to avoid weird jitter
            break;
        }
    }
}

void ObstacleManager::reset() noexcept {
    // Static obstacles maintain their state
}

const std::vector<std::unique_ptr<Obstacle>>& ObstacleManager::getObstacles() const noexcept {
    return obstacles_;
}

size_t ObstacleManager::getCount() const noexcept {
    return obstacles_.size();
}
