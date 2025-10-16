#include "obstacle_manager.hpp"
#include <cmath>

namespace {
    // Wall configuration
    constexpr float WALL_HEIGHT = 2.5f;  // Height above ground
    constexpr float WALL_THICKNESS = 2.0f;  // Thickness of the continuous wall
    constexpr float WALL_SEGMENT_LENGTH = 5.0f;  // Smaller segments for smoother continuous wall

    // Tree configuration
    constexpr float TREE_HEIGHT = 0.0f;  // Trees sit on ground
    constexpr float MIN_TREE_DISTANCE_FROM_WALL = 15.0f;  // Keep trees away from walls
    constexpr float MIN_TREE_DISTANCE_FROM_CENTER = 10.0f;  // Keep trees away from spawn
    constexpr float MIN_DISTANCE_BETWEEN_TREES = 8.0f;  // Prevent trees from overlapping
}

ObstacleManager::ObstacleManager(float playAreaSize, int treeCount)
    : randomEngine_(std::random_device{}()) {
    generateWalls(playAreaSize);
    generateTrees(treeCount, playAreaSize);
}

void ObstacleManager::generateWalls(float playAreaSize) {
    const float halfSize = playAreaSize / 2.0f;
    const int segmentsPerSide = static_cast<int>(playAreaSize / WALL_SEGMENT_LENGTH);

    // Generate continuous walls by placing segments with proper orientation
    for (int i = 0; i < segmentsPerSide; ++i) {
        float offset = -halfSize + (i * WALL_SEGMENT_LENGTH) + (WALL_SEGMENT_LENGTH / 2.0f);

        // North wall (z = -halfSize) - horizontal orientation
        obstacles_.push_back(std::make_unique<Obstacle>(offset, WALL_HEIGHT, -halfSize, ObstacleType::WALL, WallOrientation::HORIZONTAL));

        // South wall (z = +halfSize) - horizontal orientation
        obstacles_.push_back(std::make_unique<Obstacle>(offset, WALL_HEIGHT, halfSize, ObstacleType::WALL, WallOrientation::HORIZONTAL));

        // West wall (x = -halfSize) - vertical orientation
        obstacles_.push_back(std::make_unique<Obstacle>(-halfSize, WALL_HEIGHT, offset, ObstacleType::WALL, WallOrientation::VERTICAL));

        // East wall (x = +halfSize) - vertical orientation
        obstacles_.push_back(std::make_unique<Obstacle>(halfSize, WALL_HEIGHT, offset, ObstacleType::WALL, WallOrientation::VERTICAL));
    }
}

void ObstacleManager::generateTrees(int count, float playAreaSize) {
    const float halfSize = playAreaSize / 2.0f;
    const float minPos = -halfSize + MIN_TREE_DISTANCE_FROM_WALL;
    const float maxPos = halfSize - MIN_TREE_DISTANCE_FROM_WALL;

    std::uniform_real_distribution<float> posDistribution(minPos, maxPos);

    int treesPlaced = 0;
    int attempts = 0;
    const int maxAttempts = count * 10;  // Prevent infinite loop

    while (treesPlaced < count && attempts < maxAttempts) {
        attempts++;

        float x = posDistribution(randomEngine_);
        float z = posDistribution(randomEngine_);

        // Check distance from center (spawn point)
        float distanceFromCenter = std::sqrt(x * x + z * z);
        if (distanceFromCenter < MIN_TREE_DISTANCE_FROM_CENTER) {
            continue;
        }

        // Check distance from other trees
        bool tooClose = false;
        for (const auto& obstacle : obstacles_) {
            if (obstacle->getType() == ObstacleType::TREE) {
                const auto& pos = obstacle->getPosition();
                float dx = x - pos[0];
                float dz = z - pos[2];
                float distance = std::sqrt(dx * dx + dz * dz);

                if (distance < MIN_DISTANCE_BETWEEN_TREES) {
                    tooClose = true;
                    break;
                }
            }
        }

        if (!tooClose) {
            obstacles_.push_back(std::make_unique<Obstacle>(x, TREE_HEIGHT, z, ObstacleType::TREE));
            treesPlaced++;
        }
    }
}

void ObstacleManager::handleCollisions(Vehicle& vehicle) {
    const auto& vehiclePos = vehicle.getPosition();
    const auto& vehicleSize = vehicle.getSize();

    constexpr float COLLISION_BUFFER = 0.05f;  // Small buffer to prevent re-collision jitter

    for (const auto& obstacle : obstacles_) {
        if (vehicle.intersects(*obstacle)) {
            // Simple collision response: push vehicle away from obstacle
            const auto& obstaclePos = obstacle->getPosition();
            const auto& obstacleSize = obstacle->getSize();

            // Calculate overlap on each axis
            float dx = vehiclePos[0] - obstaclePos[0];
            float dz = vehiclePos[2] - obstaclePos[2];

            // Calculate penetration depth
            float overlapX = (vehicleSize[0] + obstacleSize[0]) / 2.0f - std::abs(dx);
            float overlapZ = (vehicleSize[2] + obstacleSize[2]) / 2.0f - std::abs(dz);

            // Only resolve collision if there's actual overlap
            if (overlapX > 0 && overlapZ > 0) {
                // Push along axis with smallest overlap (+ buffer to prevent jitter)
                if (overlapX < overlapZ) {
                    // Push along X axis
                    float pushX = (dx > 0) ? (overlapX + COLLISION_BUFFER) : -(overlapX + COLLISION_BUFFER);
                    vehicle.setPosition(vehiclePos[0] + pushX, vehiclePos[1], vehiclePos[2]);
                } else {
                    // Push along Z axis
                    float pushZ = (dz > 0) ? (overlapZ + COLLISION_BUFFER) : -(overlapZ + COLLISION_BUFFER);
                    vehicle.setPosition(vehiclePos[0], vehiclePos[1], vehiclePos[2] + pushZ);
                }

                // Reduce velocity instead of stopping completely for smoother collision
                float currentVelocity = vehicle.getVelocity();
                vehicle.setVelocity(currentVelocity * 0.3f);  // Reduce to 30% on collision
            }
        }
    }
}

void ObstacleManager::reset() {
    // Obstacles are static, no reset needed
    // But method exists for consistency with other managers
}

const std::vector<std::unique_ptr<Obstacle>>& ObstacleManager::getObstacles() const {
    return obstacles_;
}
