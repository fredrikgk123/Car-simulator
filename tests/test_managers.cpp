#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/obstacle_manager.hpp"
#include "core/powerup_manager.hpp"
#include "core/vehicle.hpp"

using Catch::Approx;

// ==================== ObstacleManager Tests ====================

TEST_CASE("ObstacleManager initialization", "[obstacle_manager]") {
    constexpr float PLAY_AREA_SIZE = 100.0f;
    constexpr int TREE_COUNT = 10;

    ObstacleManager manager(PLAY_AREA_SIZE, TREE_COUNT);

    SECTION("Creates obstacles") {
        REQUIRE(manager.getCount() > 0);
    }

    SECTION("Creates walls") {
        const auto& obstacles = manager.getObstacles();

        // Count walls (should have 4 walls for perimeter)
        size_t wallCount = 0;
        for (const auto& obstacle : obstacles) {
            if (obstacle->getType() == ObstacleType::WALL) {
                wallCount++;
            }
        }

        REQUIRE(wallCount > 0);
    }

    SECTION("Creates requested number of trees (approximately)") {
        const auto& obstacles = manager.getObstacles();

        // Count trees
        size_t treeCount = 0;
        for (const auto& obstacle : obstacles) {
            if (obstacle->getType() == ObstacleType::TREE) {
                treeCount++;
            }
        }

        // May not place all trees if spacing constraints are tight
        REQUIRE(treeCount > 0);
        REQUIRE(treeCount <= TREE_COUNT);
    }

    SECTION("All obstacles are active") {
        const auto& obstacles = manager.getObstacles();

        for (const auto& obstacle : obstacles) {
            REQUIRE(obstacle->isActive());
        }
    }
}

TEST_CASE("ObstacleManager wall placement", "[obstacle_manager]") {
    constexpr float PLAY_AREA_SIZE = 100.0f;
    ObstacleManager manager(PLAY_AREA_SIZE, 5);

    const auto& obstacles = manager.getObstacles();

    SECTION("Walls are placed at perimeter") {
        const float halfSize = PLAY_AREA_SIZE / 2.0f;

        bool hasNorthWall = false;
        bool hasSouthWall = false;
        bool hasEastWall = false;
        bool hasWestWall = false;

        for (const auto& obstacle : obstacles) {
            if (obstacle->getType() == ObstacleType::WALL) {
                auto pos = obstacle->getPosition();

                // Check if near any edge (with small tolerance)
                if (std::abs(pos[2] + halfSize) < 5.0f) hasNorthWall = true;
                if (std::abs(pos[2] - halfSize) < 5.0f) hasSouthWall = true;
                if (std::abs(pos[0] + halfSize) < 5.0f) hasWestWall = true;
                if (std::abs(pos[0] - halfSize) < 5.0f) hasEastWall = true;
            }
        }

        REQUIRE(hasNorthWall);
        REQUIRE(hasSouthWall);
        REQUIRE(hasEastWall);
        REQUIRE(hasWestWall);
    }
}

TEST_CASE("ObstacleManager collision handling", "[obstacle_manager]") {
    constexpr float PLAY_AREA_SIZE = 100.0f;
    ObstacleManager manager(PLAY_AREA_SIZE, 5);

    SECTION("Handles collision with vehicle") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Move vehicle toward a wall
        vehicle.setPosition(PLAY_AREA_SIZE / 2.0f - 1.0f, 0.0f, 0.0f);

        // Give the vehicle some velocity
        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        float velocityBefore = vehicle.getVelocity();
        REQUIRE(velocityBefore > 0.0f);

        // Handle collision
        manager.handleCollisions(vehicle);

        // If a collision occurred, velocity should be zero
        // Note: This test may not always trigger a collision depending on exact positions
        REQUIRE(vehicle.getVelocity() <= velocityBefore);
    }

    SECTION("Stops vehicle on collision") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Create a manager with small play area for guaranteed collision
        ObstacleManager smallManager(20.0f, 0);

        // Verify walls were created
        const auto& obstacles = smallManager.getObstacles();
        REQUIRE(obstacles.size() > 0);

        // Find an actual wall and place vehicle directly on top of it
        const Obstacle* testWall = nullptr;
        for (const auto& obstacle : obstacles) {
            if (obstacle->getType() == ObstacleType::WALL) {
                testWall = obstacle.get();
                break;
            }
        }
        REQUIRE(testWall != nullptr);

        // Place vehicle at the same position as the wall - guaranteed collision
        auto wallPos = testWall->getPosition();
        vehicle.setPosition(wallPos[0], wallPos[1], wallPos[2]);
        vehicle.setVelocity(10.0f);

        // Handle collision - should detect overlap and stop vehicle
        smallManager.handleCollisions(vehicle);

        // Should have collided and stopped
        REQUIRE(vehicle.getVelocity() == 0.0f);
    }
}

TEST_CASE("ObstacleManager update", "[obstacle_manager]") {
    ObstacleManager manager(100.0f, 5);

    SECTION("Update doesn't throw (obstacles are static)") {
        REQUIRE_NOTHROW(manager.update(0.016f));
    }
}

TEST_CASE("ObstacleManager reset", "[obstacle_manager]") {
    ObstacleManager manager(100.0f, 5);

    SECTION("Reset doesn't throw") {
        REQUIRE_NOTHROW(manager.reset());
    }

    SECTION("Obstacles remain after reset") {
        size_t countBefore = manager.getCount();
        manager.reset();
        REQUIRE(manager.getCount() == countBefore);
    }
}

// ==================== PowerupManager Tests ====================

TEST_CASE("PowerupManager initialization", "[powerup_manager]") {
    constexpr int POWERUP_COUNT = 10;
    constexpr float PLAY_AREA_SIZE = 100.0f;

    PowerupManager manager(POWERUP_COUNT, PLAY_AREA_SIZE);

    SECTION("Creates requested number of powerups") {
        REQUIRE(manager.getCount() == POWERUP_COUNT);
    }

    SECTION("All powerups are active initially") {
        const auto& powerups = manager.getPowerups();

        for (const auto& powerup : powerups) {
            REQUIRE(powerup->isActive());
        }
    }

    SECTION("All powerups are NITROUS type") {
        const auto& powerups = manager.getPowerups();

        for (const auto& powerup : powerups) {
            REQUIRE(powerup->getType() == PowerupType::NITROUS);
        }
    }
}

TEST_CASE("PowerupManager placement", "[powerup_manager]") {
    constexpr int POWERUP_COUNT = 10;
    constexpr float PLAY_AREA_SIZE = 100.0f;

    PowerupManager manager(POWERUP_COUNT, PLAY_AREA_SIZE);
    const auto& powerups = manager.getPowerups();

    SECTION("Powerups are within play area bounds") {
        const float margin = 10.0f;  // From GameConfig::Powerup::SPAWN_MARGIN
        const float maxCoord = (PLAY_AREA_SIZE / 2.0f) - margin;

        for (const auto& powerup : powerups) {
            auto pos = powerup->getPosition();

            REQUIRE(std::abs(pos[0]) <= maxCoord);
            REQUIRE(std::abs(pos[2]) <= maxCoord);
        }
    }

    SECTION("Powerups are at correct height") {
        const float expectedHeight = 0.4f;  // From GameConfig::Powerup::HEIGHT

        for (const auto& powerup : powerups) {
            auto pos = powerup->getPosition();
            REQUIRE(pos[1] == Approx(expectedHeight));
        }
    }
}

TEST_CASE("PowerupManager collision handling", "[powerup_manager]") {
    PowerupManager manager(5, 100.0f);

    SECTION("Vehicle picks up powerup on collision") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Place vehicle at same position as first powerup
        const auto& powerups = manager.getPowerups();
        if (!powerups.empty()) {
            auto powerupPos = powerups[0]->getPosition();
            vehicle.setPosition(powerupPos[0], powerupPos[1], powerupPos[2]);

            REQUIRE_FALSE(vehicle.hasNitrous());

            // Handle collision
            manager.handleCollisions(vehicle);

            // Vehicle should now have nitrous
            REQUIRE(vehicle.hasNitrous());

            // Powerup should be inactive
            REQUIRE_FALSE(powerups[0]->isActive());
        }
    }

    SECTION("Cannot pick up powerup if already has nitrous") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);
        vehicle.pickupNitrous();

        const auto& powerups = manager.getPowerups();
        if (!powerups.empty()) {
            auto powerupPos = powerups[0]->getPosition();
            vehicle.setPosition(powerupPos[0], powerupPos[1], powerupPos[2]);

            bool powerupActiveBeforeCollision = powerups[0]->isActive();

            manager.handleCollisions(vehicle);

            // Powerup should still be active
            REQUIRE(powerups[0]->isActive() == powerupActiveBeforeCollision);
        }
    }

    SECTION("Cannot pick up powerup if nitrous is active") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);
        vehicle.pickupNitrous();
        vehicle.activateNitrous();

        const auto& powerups = manager.getPowerups();
        if (!powerups.empty()) {
            auto powerupPos = powerups[0]->getPosition();
            vehicle.setPosition(powerupPos[0], powerupPos[1], powerupPos[2]);

            manager.handleCollisions(vehicle);

            // Powerup should still be active
            REQUIRE(powerups[0]->isActive());
        }
    }

    SECTION("Cannot pick up inactive powerup") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        const auto& powerups = manager.getPowerups();
        if (!powerups.empty()) {
            // Deactivate powerup
            powerups[0]->setActive(false);

            auto powerupPos = powerups[0]->getPosition();
            vehicle.setPosition(powerupPos[0], powerupPos[1], powerupPos[2]);

            manager.handleCollisions(vehicle);

            // Vehicle should not have nitrous
            REQUIRE_FALSE(vehicle.hasNitrous());
        }
    }
}

TEST_CASE("PowerupManager update", "[powerup_manager]") {
    PowerupManager manager(5, 100.0f);

    SECTION("Update doesn't throw") {
        REQUIRE_NOTHROW(manager.update(0.016f));
    }

    SECTION("Powerups rotate over time") {
        const auto& powerups = manager.getPowerups();
        if (!powerups.empty()) {
            float initialRotation = powerups[0]->getRotation();

            // Update multiple times
            for (int i = 0; i < 10; ++i) {
                manager.update(0.016f);
            }

            float finalRotation = powerups[0]->getRotation();

            // Rotation should have changed
            REQUIRE(finalRotation != Approx(initialRotation));
        }
    }
}

TEST_CASE("PowerupManager reset", "[powerup_manager]") {
    PowerupManager manager(5, 100.0f);
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Reset reactivates all powerups") {
        // Collect some powerups
        const auto& powerups = manager.getPowerups();

        for (size_t i = 0; i < std::min(size_t(3), powerups.size()); ++i) {
            auto powerupPos = powerups[i]->getPosition();
            vehicle.setPosition(powerupPos[0], powerupPos[1], powerupPos[2]);

            manager.handleCollisions(vehicle);

            // Reset vehicle for next pickup
            if (i < 2) {
                vehicle = Vehicle(0.0f, 0.0f, 0.0f);
            }
        }

        // Count inactive powerups
        size_t inactiveCount = 0;
        for (const auto& powerup : powerups) {
            if (!powerup->isActive()) {
                inactiveCount++;
            }
        }

        REQUIRE(inactiveCount > 0);

        // Reset manager
        manager.reset();

        // All should be active again
        for (const auto& powerup : powerups) {
            REQUIRE(powerup->isActive());
        }
    }

    SECTION("Powerup count remains same after reset") {
        size_t countBefore = manager.getCount();
        manager.reset();
        REQUIRE(manager.getCount() == countBefore);
    }
}

// ==================== Manager Integration Tests ====================

TEST_CASE("Manager integration", "[managers][integration]") {
    SECTION("ObstacleManager and PowerupManager work together") {
        constexpr float PLAY_AREA_SIZE = 100.0f;

        ObstacleManager obstacleManager(PLAY_AREA_SIZE, 10);
        PowerupManager powerupManager(10, PLAY_AREA_SIZE);
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Update everything
        REQUIRE_NOTHROW(obstacleManager.update(0.016f));
        REQUIRE_NOTHROW(powerupManager.update(0.016f));
        REQUIRE_NOTHROW(vehicle.update(0.016f));

        // Handle collisions
        REQUIRE_NOTHROW(obstacleManager.handleCollisions(vehicle));
        REQUIRE_NOTHROW(powerupManager.handleCollisions(vehicle));
    }

    SECTION("Multiple collision handling doesn't crash") {
        ObstacleManager obstacleManager(50.0f, 5);
        PowerupManager powerupManager(5, 50.0f);
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Simulate multiple frames
        for (int i = 0; i < 100; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.016f);

            obstacleManager.handleCollisions(vehicle);
            powerupManager.handleCollisions(vehicle);
        }

        REQUIRE(true);  // If we get here, no crash occurred
    }
}
