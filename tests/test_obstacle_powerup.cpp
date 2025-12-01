#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/obstacle.hpp"
#include "core/powerup.hpp"

using Catch::Approx;

// ==================== Obstacle Tests ====================

TEST_CASE("Obstacle creation - walls", "[obstacle]") {
    SECTION("Horizontal wall creation") {
        Obstacle wall(0.0f, 2.5f, -50.0f, ObstacleType::WALL, WallOrientation::HORIZONTAL);

        REQUIRE(wall.getType() == ObstacleType::WALL);
        REQUIRE(wall.getOrientation() == WallOrientation::HORIZONTAL);
        REQUIRE(wall.isActive());

        auto pos = wall.getPosition();
        REQUIRE(pos[0] == 0.0f);
        REQUIRE(pos[1] == 2.5f);
        REQUIRE(pos[2] == -50.0f);
    }

    SECTION("Vertical wall creation") {
        Obstacle wall(-50.0f, 2.5f, 0.0f, ObstacleType::WALL, WallOrientation::VERTICAL);

        REQUIRE(wall.getType() == ObstacleType::WALL);
        REQUIRE(wall.getOrientation() == WallOrientation::VERTICAL);
    }

    SECTION("Wall has correct size") {
        Obstacle wall(0.0f, 2.5f, 0.0f, ObstacleType::WALL, WallOrientation::HORIZONTAL);

        auto size = wall.getSize();
        REQUIRE(size[0] > 0.0f);
        REQUIRE(size[1] > 0.0f);
        REQUIRE(size[2] > 0.0f);
    }
}

TEST_CASE("Obstacle creation - trees", "[obstacle]") {
    SECTION("Tree creation") {
        Obstacle tree(10.0f, 0.0f, 20.0f, ObstacleType::TREE);

        REQUIRE(tree.getType() == ObstacleType::TREE);
        REQUIRE(tree.isActive());

        auto pos = tree.getPosition();
        REQUIRE(pos[0] == 10.0f);
        REQUIRE(pos[1] == 0.0f);
        REQUIRE(pos[2] == 20.0f);
    }

    SECTION("Tree has collision size") {
        Obstacle tree(0.0f, 0.0f, 0.0f, ObstacleType::TREE);

        auto size = tree.getSize();
        REQUIRE(size[0] > 0.0f);
        REQUIRE(size[1] > 0.0f);
        REQUIRE(size[2] > 0.0f);
    }
}

TEST_CASE("Obstacle update", "[obstacle]") {
    SECTION("Update doesn't throw (obstacles are static)") {
        Obstacle wall(0.0f, 2.5f, 0.0f, ObstacleType::WALL, WallOrientation::HORIZONTAL);

        REQUIRE_NOTHROW(wall.update(0.016f));
    }

    SECTION("Position doesn't change on update") {
        Obstacle tree(5.0f, 0.0f, 10.0f, ObstacleType::TREE);

        auto posBefore = tree.getPosition();
        tree.update(0.016f);
        auto posAfter = tree.getPosition();

        REQUIRE(posBefore[0] == posAfter[0]);
        REQUIRE(posBefore[1] == posAfter[1]);
        REQUIRE(posBefore[2] == posAfter[2]);
    }
}

TEST_CASE("Obstacle reset", "[obstacle]") {
    SECTION("Reset returns to initial state") {
        Obstacle wall(5.0f, 2.5f, 10.0f, ObstacleType::WALL, WallOrientation::HORIZONTAL);

        auto initialPos = wall.getPosition();

        // Modify position
        wall.setPosition(20.0f, 5.0f, 30.0f);
        REQUIRE(wall.getPosition()[0] == 20.0f);

        // Reset
        wall.reset();
        auto resetPos = wall.getPosition();

        REQUIRE(resetPos[0] == Approx(initialPos[0]));
        REQUIRE(resetPos[1] == Approx(initialPos[1]));
        REQUIRE(resetPos[2] == Approx(initialPos[2]));
    }
}

// ==================== Powerup Tests ====================

TEST_CASE("Powerup creation", "[powerup]") {
    SECTION("Nitrous powerup creation") {
        Powerup powerup(5.0f, 0.4f, 10.0f, PowerupType::NITROUS);

        REQUIRE(powerup.getType() == PowerupType::NITROUS);
        REQUIRE(powerup.isActive());

        auto pos = powerup.getPosition();
        REQUIRE(pos[0] == 5.0f);
        REQUIRE(pos[1] == 0.4f);
        REQUIRE(pos[2] == 10.0f);
    }

    SECTION("Powerup has size") {
        Powerup powerup(0.0f, 0.0f, 0.0f, PowerupType::NITROUS);

        auto size = powerup.getSize();
        REQUIRE(size[0] > 0.0f);
        REQUIRE(size[1] > 0.0f);
        REQUIRE(size[2] > 0.0f);
    }
}

TEST_CASE("Powerup rotation animation", "[powerup]") {
    Powerup powerup(0.0f, 0.4f, 0.0f, PowerupType::NITROUS);

    SECTION("Powerup rotates over time") {
        float initialRotation = powerup.getRotation();

        // Update multiple times
        for (int i = 0; i < 10; ++i) {
            powerup.update(0.016f);
        }

        float finalRotation = powerup.getRotation();

        REQUIRE(finalRotation != Approx(initialRotation));
    }

    SECTION("Rotation is continuous") {
        float rotation1 = powerup.getRotation();
        powerup.update(0.016f);
        float rotation2 = powerup.getRotation();
        powerup.update(0.016f);
        float rotation3 = powerup.getRotation();

        // Each update should change rotation
        REQUIRE(rotation2 != Approx(rotation1));
        REQUIRE(rotation3 != Approx(rotation2));
    }
}

TEST_CASE("Powerup active state", "[powerup]") {
    Powerup powerup(0.0f, 0.4f, 0.0f, PowerupType::NITROUS);

    SECTION("Can be deactivated") {
        REQUIRE(powerup.isActive());

        powerup.setActive(false);
        REQUIRE_FALSE(powerup.isActive());
    }

    SECTION("Can be reactivated") {
        powerup.setActive(false);
        powerup.setActive(true);
        REQUIRE(powerup.isActive());
    }

    SECTION("Still rotates when inactive") {
        powerup.setActive(false);

        float initialRotation = powerup.getRotation();

        // Update multiple times to ensure visible rotation
        for (int i = 0; i < 100; ++i) {
            powerup.update(0.016f);
        }

        float finalRotation = powerup.getRotation();

        // Should still rotate for visual effect
        REQUIRE(finalRotation != Approx(initialRotation));
    }
}

TEST_CASE("Powerup reset", "[powerup]") {
    Powerup powerup(10.0f, 0.4f, 20.0f, PowerupType::NITROUS);

    SECTION("Reset returns to initial position") {
        auto initialPos = powerup.getPosition();

        powerup.setPosition(50.0f, 1.0f, 60.0f);
        powerup.reset();

        auto resetPos = powerup.getPosition();
        REQUIRE(resetPos[0] == Approx(initialPos[0]));
        REQUIRE(resetPos[1] == Approx(initialPos[1]));
        REQUIRE(resetPos[2] == Approx(initialPos[2]));
    }

    SECTION("Reset reactivates powerup") {
        powerup.setActive(false);
        powerup.reset();

        REQUIRE(powerup.isActive());
    }
}

// ==================== GameObject-derived class interactions ====================

TEST_CASE("Obstacle and Powerup collision", "[obstacle][powerup][collision]") {
    SECTION("Tree and powerup can detect collision") {
        Obstacle tree(0.0f, 0.0f, 0.0f, ObstacleType::TREE);
        Powerup powerup(0.0f, 0.4f, 0.0f, PowerupType::NITROUS);

        // Both at same X,Z position - should collide
        REQUIRE(tree.intersects(powerup));
        REQUIRE(powerup.intersects(tree));
    }

    SECTION("Distant objects don't collide") {
        Obstacle tree(0.0f, 0.0f, 0.0f, ObstacleType::TREE);
        Powerup powerup(100.0f, 0.4f, 100.0f, PowerupType::NITROUS);

        REQUIRE_FALSE(tree.intersects(powerup));
        REQUIRE_FALSE(powerup.intersects(tree));
    }
}

TEST_CASE("Obstacle types", "[obstacle][types]") {
    SECTION("Wall orientation affects dimensions") {
        Obstacle horizontalWall(0.0f, 2.5f, 0.0f, ObstacleType::WALL, WallOrientation::HORIZONTAL);
        Obstacle verticalWall(0.0f, 2.5f, 0.0f, ObstacleType::WALL, WallOrientation::VERTICAL);

        auto hSize = horizontalWall.getSize();
        auto vSize = verticalWall.getSize();

        // Dimensions should be swapped for different orientations
        REQUIRE(hSize[0] != vSize[0]);
        REQUIRE(hSize[2] != vSize[2]);
    }
}
