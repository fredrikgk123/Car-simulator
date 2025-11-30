#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "core/vehicle.hpp"

using Catch::Approx;

TEST_CASE("Vehicle initialization", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Initial position is correct") {
        auto position = vehicle.getPosition();
        REQUIRE(position[0] == 0.0f);
        REQUIRE(position[1] == 0.0f);
        REQUIRE(position[2] == 0.0f);
    }

    SECTION("Initial velocity is zero") {
        REQUIRE(vehicle.getVelocity() == 0.0f);
    }

    SECTION("Initial rotation is 180 degrees (PI radians)") {
        REQUIRE(vehicle.getRotation() == Approx(3.14159f).epsilon(0.0001));
    }
}

TEST_CASE("Vehicle acceleration", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Forward acceleration increases velocity") {
        vehicle.accelerateForward();
        vehicle.update(0.1f);
        REQUIRE(vehicle.getVelocity() > 0.0f);
    }

    SECTION("Backward acceleration decreases velocity") {
        vehicle.accelerateBackward();
        vehicle.update(0.1f);
        REQUIRE(vehicle.getVelocity() < 0.0f);
    }

    SECTION("Multiple updates continue acceleration") {
        for (int i = 0; i < 5; i = i + 1) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }
        REQUIRE(vehicle.getVelocity() > 1.0f);
    }
}

TEST_CASE("Vehicle max speed limits", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Forward speed is clamped to max speed") {
        // Accelerate for a long time
        for (int i = 0; i < 100; i = i + 1) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }
        REQUIRE(vehicle.getVelocity() <= vehicle.getMaxSpeed());
    }

    SECTION("Backward speed is limited") {
        // Accelerate backward for a long time
        for (int i = 0; i < 100; i = i + 1) {
            vehicle.accelerateBackward();
            vehicle.update(0.1f);
        }
        // Reverse speed should be less than forward max speed
        REQUIRE(vehicle.getVelocity() >= -vehicle.getMaxSpeed());
    }
}

TEST_CASE("Vehicle friction", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Vehicle slows down without acceleration") {
        // Accelerate first
        for (int i = 0; i < 10; i = i + 1) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        float velocityBefore = vehicle.getVelocity();

        // Let it coast (no acceleration)
        for (int i = 0; i < 10; i = i + 1) {
            vehicle.update(0.1f);
        }

        float velocityAfter = vehicle.getVelocity();
        REQUIRE(velocityAfter < velocityBefore);
    }
}

TEST_CASE("Vehicle turning", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Turning changes rotation") {
        float initialRotation = vehicle.getRotation();

        // Accelerate until velocity exceeds MIN_TURN_SPEED
        while (vehicle.getVelocity() < 0.51f) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.turn(0.1f);

        REQUIRE(vehicle.getRotation() != initialRotation);
    }

    SECTION("Cannot turn when stationary") {
        // Try to turn without velocity
        float initialRotation = vehicle.getRotation();
        vehicle.turn(1.0f);
        REQUIRE(vehicle.getRotation() == Approx(initialRotation));
    }
}

TEST_CASE("Vehicle reset", "[vehicle]") {
    Vehicle vehicle(5.0f, 0.0f, 10.0f);

    // Move and accelerate
    for (int i = 0; i < 10; i = i + 1) {
        vehicle.accelerateForward();
        vehicle.update(0.1f);
    }

    // Reset
    vehicle.reset();

    SECTION("Position returns to initial") {
        auto position = vehicle.getPosition();
        REQUIRE(position[0] == 5.0f);
        REQUIRE(position[1] == 0.0f);
        REQUIRE(position[2] == 10.0f);
    }

    SECTION("Velocity returns to zero") {
        REQUIRE(vehicle.getVelocity() == 0.0f);
    }

    SECTION("Rotation returns to initial 180 degrees (PI radians)") {
        REQUIRE(vehicle.getRotation() == Approx(3.14159f).epsilon(0.0001));
    }
}

TEST_CASE("Vehicle movement", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Forward movement changes position") {
        auto initialPos = vehicle.getPosition();

        for (int i = 0; i < 10; i = i + 1) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        auto finalPos = vehicle.getPosition();

        // Position should have changed (exact values depend on rotation)
        bool positionChanged = (initialPos[0] != finalPos[0]) ||
                              (initialPos[2] != finalPos[2]);
        REQUIRE(positionChanged == true);
    }
}

TEST_CASE("Vehicle gear system", "[vehicle][gears]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Vehicle starts in gear 1") {
        REQUIRE(vehicle.getCurrentGear() == 1);
    }

    SECTION("Gears shift up as speed increases") {
        int previousGear = vehicle.getCurrentGear();

        // Accelerate to high speed
        for (int i = 0; i < 50; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        int currentGear = vehicle.getCurrentGear();
        REQUIRE(currentGear > previousGear);
        REQUIRE(currentGear <= 5); // Max gear is 5
    }

    SECTION("Reverse sets gear to 0") {
        // Accelerate backward
        for (int i = 0; i < 10; i++) {
            vehicle.accelerateBackward();
            vehicle.update(0.1f);
        }

        REQUIRE(vehicle.getCurrentGear() == 0);
    }

    SECTION("RPM increases with speed") {
        float initialRPM = vehicle.getRPM();

        // Accelerate
        for (int i = 0; i < 20; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        REQUIRE(vehicle.getRPM() > initialRPM);
    }
}

TEST_CASE("Vehicle nitrous system", "[vehicle][nitrous]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Vehicle has no nitrous initially") {
        REQUIRE_FALSE(vehicle.hasNitrous());
        REQUIRE_FALSE(vehicle.isNitrousActive());
    }

    SECTION("Picking up nitrous gives nitrous") {
        vehicle.pickupNitrous();
        REQUIRE(vehicle.hasNitrous());
        REQUIRE_FALSE(vehicle.isNitrousActive());
    }

    SECTION("Activating nitrous consumes it") {
        vehicle.pickupNitrous();
        vehicle.activateNitrous();

        REQUIRE_FALSE(vehicle.hasNitrous());
        REQUIRE(vehicle.isNitrousActive());
        REQUIRE(vehicle.getNitrousTimeRemaining() > 0.0f);
    }

    SECTION("Nitrous depletes over time") {
        vehicle.pickupNitrous();
        vehicle.activateNitrous();

        float initialTime = vehicle.getNitrousTimeRemaining();

        // Update several times
        for (int i = 0; i < 10; i++) {
            vehicle.update(0.1f);
        }

        float finalTime = vehicle.getNitrousTimeRemaining();
        REQUIRE(finalTime < initialTime);
    }

    SECTION("Nitrous eventually runs out") {
        vehicle.pickupNitrous();
        vehicle.activateNitrous();

        // Update for long time
        for (int i = 0; i < 100; i++) {
            vehicle.update(0.1f);
        }

        REQUIRE_FALSE(vehicle.isNitrousActive());
        REQUIRE(vehicle.getNitrousTimeRemaining() == 0.0f);
    }

    SECTION("Cannot activate nitrous without pickup") {
        vehicle.activateNitrous();
        REQUIRE_FALSE(vehicle.isNitrousActive());
    }

    SECTION("Nitrous increases acceleration") {
        Vehicle v1(0.0f, 0.0f, 0.0f);
        Vehicle v2(0.0f, 0.0f, 0.0f);

        v2.pickupNitrous();
        v2.activateNitrous();

        // Accelerate both
        for (int i = 0; i < 10; i++) {
            v1.accelerateForward();
            v2.accelerateForward();
            v1.update(0.1f);
            v2.update(0.1f);
        }

        REQUIRE(v2.getVelocity() > v1.getVelocity());
    }
}

TEST_CASE("Vehicle drift mechanics", "[vehicle][drift]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Vehicle is not drifting initially") {
        REQUIRE_FALSE(vehicle.isDrifting());
    }

    SECTION("Start drift sets drift flag") {
        vehicle.startDrift();
        REQUIRE(vehicle.isDrifting());
    }

    SECTION("Stop drift clears drift flag") {
        vehicle.startDrift();
        vehicle.stopDrift();
        REQUIRE_FALSE(vehicle.isDrifting());
    }

    SECTION("Drift angle accumulates during drift") {
        // Accelerate to good speed
        for (int i = 0; i < 20; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.startDrift();
        float initialDriftAngle = vehicle.getDriftAngle();

        // Turn while drifting
        for (int i = 0; i < 5; i++) {
            vehicle.turn(0.5f);
            vehicle.update(0.1f);
        }

        float finalDriftAngle = vehicle.getDriftAngle();
        REQUIRE(std::abs(finalDriftAngle) >= std::abs(initialDriftAngle));
    }

    SECTION("Drift angle decays when not turning") {
        // Build up drift angle
        for (int i = 0; i < 20; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.startDrift();
        for (int i = 0; i < 5; i++) {
            vehicle.turn(0.5f);
            vehicle.update(0.1f);
        }

        float driftAngleAfterTurn = vehicle.getDriftAngle();

        // Let it decay (no turning)
        for (int i = 0; i < 10; i++) {
            vehicle.update(0.1f);
        }

        float driftAngleAfterDecay = vehicle.getDriftAngle();
        REQUIRE(std::abs(driftAngleAfterDecay) < std::abs(driftAngleAfterTurn));
    }
}

TEST_CASE("Vehicle scale", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Default scale is 1.0") {
        REQUIRE(vehicle.getScale() == Approx(1.0f));
    }

    SECTION("Can set custom scale") {
        vehicle.setScale(2.0f);
        REQUIRE(vehicle.getScale() == Approx(2.0f));
    }

    SECTION("Scale affects collision size") {
        auto initialSize = vehicle.getSize();
        vehicle.setScale(2.0f);
        auto scaledSize = vehicle.getSize();

        REQUIRE(scaledSize[0] == Approx(initialSize[0] * 2.0f));
        REQUIRE(scaledSize[2] == Approx(initialSize[2] * 2.0f));
    }

    SECTION("Invalid scale is clamped to 1.0") {
        vehicle.setScale(-5.0f);
        REQUIRE(vehicle.getScale() == Approx(1.0f));

        vehicle.setScale(0.0f);
        REQUIRE(vehicle.getScale() == Approx(1.0f));
    }
}

TEST_CASE("Vehicle steering input", "[vehicle]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Steering input is initially zero") {
        REQUIRE(vehicle.getSteeringInput() == Approx(0.0f));
    }

    SECTION("Turning sets steering input") {
        // Need speed for turning
        for (int i = 0; i < 10; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.turn(0.5f);
        REQUIRE(vehicle.getSteeringInput() == Approx(0.5f));
    }

    SECTION("Steering input decays over time") {
        // Get to speed and turn
        for (int i = 0; i < 10; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.turn(1.0f);
        float initialInput = vehicle.getSteeringInput();

        // Update without turning
        for (int i = 0; i < 5; i++) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        float decayedInput = vehicle.getSteeringInput();
        REQUIRE(std::abs(decayedInput) < std::abs(initialInput));
    }
}
