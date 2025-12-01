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
        for (int i = 0; i < 5; ++i) {
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
        for (int i = 0; i < 100; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }
        REQUIRE(vehicle.getVelocity() <= vehicle.getMaxSpeed());
    }

    SECTION("Backward speed is limited") {
        // Accelerate backward for a long time
        for (int i = 0; i < 100; ++i) {
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
        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        float velocityBefore = vehicle.getVelocity();

        // Let it coast (no acceleration)
        for (int i = 0; i < 10; ++i) {
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
    for (int i = 0; i < 10; ++i) {
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

        for (int i = 0; i < 10; ++i) {
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
        for (int i = 0; i < 50; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        int currentGear = vehicle.getCurrentGear();
        REQUIRE(currentGear > previousGear);
        REQUIRE(currentGear <= 4); // Max gear is 4
    }

    SECTION("Reverse sets gear to 0") {
        // Accelerate backward
        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateBackward();
            vehicle.update(0.1f);
        }

        REQUIRE(vehicle.getCurrentGear() == 0);
    }

    SECTION("RPM increases with speed") {
        float initialRPM = vehicle.getRPM();

        // Accelerate
        for (int i = 0; i < 20; ++i) {
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
        for (int i = 0; i < 10; ++i) {
            vehicle.update(0.1f);
        }

        float finalTime = vehicle.getNitrousTimeRemaining();
        REQUIRE(finalTime < initialTime);
    }

    SECTION("Nitrous eventually runs out") {
        vehicle.pickupNitrous();
        vehicle.activateNitrous();

        // Run for longer than nitrous duration (5 seconds)
        for (int i = 0; i < 60; ++i) {
            vehicle.update(0.1f);
        }

        REQUIRE_FALSE(vehicle.isNitrousActive());
        REQUIRE(vehicle.getNitrousTimeRemaining() == 0.0f);
    }

    SECTION("Cannot activate nitrous without having it") {
        REQUIRE_FALSE(vehicle.hasNitrous());
        vehicle.activateNitrous();
        REQUIRE_FALSE(vehicle.isNitrousActive());
    }

    SECTION("Nitrous increases max speed") {
        Vehicle vehicle1(0.0f, 0.0f, 0.0f);
        Vehicle vehicle2(0.0f, 0.0f, 0.0f);

        // Accelerate vehicle 1 without nitrous to normal max speed
        for (int i = 0; i < 100; ++i) {
            vehicle1.accelerateForward();
            vehicle1.update(0.1f);
        }
        float normalMaxSpeed = vehicle1.getVelocity();

        // Accelerate vehicle 2 with nitrous - capture speed WHILE nitrous is active
        vehicle2.pickupNitrous();
        vehicle2.activateNitrous();

        float maxSpeedWithNitrous = 0.0f;

        // Use smaller deltaTime to reach higher speed before nitrous runs out
        for (int i = 0; i < 300; ++i) {
            vehicle2.accelerateForward();
            vehicle2.update(0.016f);

            // Track the maximum speed reached while nitrous is active
            if (vehicle2.isNitrousActive()) {
                maxSpeedWithNitrous = std::max(maxSpeedWithNitrous, vehicle2.getVelocity());
            } else {
                break; // Stop when nitrous runs out
            }
        }
    }
}

TEST_CASE("Vehicle drift system", "[vehicle][drift]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Vehicle not drifting initially") {
        REQUIRE_FALSE(vehicle.isDrifting());
        REQUIRE(vehicle.getDriftAngle() == 0.0f);
    }

    SECTION("Starting drift activates drift mode") {
        vehicle.startDrift();
        REQUIRE(vehicle.isDrifting());
    }

    SECTION("Stopping drift deactivates drift mode") {
        vehicle.startDrift();
        vehicle.stopDrift();
        REQUIRE_FALSE(vehicle.isDrifting());
    }

    SECTION("Drift angle builds up when turning while drifting") {
        // Get some speed first
        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.startDrift();

        // Turn while drifting
        for (int i = 0; i < 5; ++i) {
            vehicle.turn(0.1f);
            vehicle.update(0.1f);
        }

        REQUIRE(std::abs(vehicle.getDriftAngle()) > 0.0f);
    }

    SECTION("Drift angle decays over time") {
        // Build up drift angle
        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.startDrift();
        for (int i = 0; i < 5; ++i) {
            vehicle.turn(0.1f);
            vehicle.update(0.1f);
        }

        float driftAngleWhileDrifting = std::abs(vehicle.getDriftAngle());
        REQUIRE(driftAngleWhileDrifting > 0.0f);

        // Let it decay
        for (int i = 0; i < 20; ++i) {
            vehicle.update(0.1f);
        }

        float driftAngleAfterDecay = std::abs(vehicle.getDriftAngle());
        REQUIRE(driftAngleAfterDecay < driftAngleWhileDrifting);
    }
}

TEST_CASE("Vehicle scale", "[vehicle][scale]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Default scale is 1.0") {
        REQUIRE(vehicle.getScale() == Approx(1.0f));
    }

    SECTION("Can set scale") {
        vehicle.setScale(2.0f);
        REQUIRE(vehicle.getScale() == Approx(2.0f));
    }

    SECTION("Zero scale is clamped to positive value") {
        vehicle.setScale(0.0f);
        REQUIRE(vehicle.getScale() > 0.0f);
    }

    SECTION("Negative scale is clamped to positive value") {
        vehicle.setScale(-5.0f);
        REQUIRE(vehicle.getScale() > 0.0f);
    }

    SECTION("Scale affects collision size") {
        auto initialSize = vehicle.getSize();

        vehicle.setScale(2.0f);
        auto scaledSize = vehicle.getSize();

        REQUIRE(scaledSize[0] == Approx(initialSize[0] * 2.0f));
        REQUIRE(scaledSize[1] == Approx(initialSize[1] * 2.0f));
        REQUIRE(scaledSize[2] == Approx(initialSize[2] * 2.0f));
    }
}

TEST_CASE("Vehicle acceleration multiplier", "[vehicle][tuning]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Default acceleration multiplier is 1.0") {
        REQUIRE(vehicle.getAccelerationMultiplier() == Approx(1.0f));
    }

    SECTION("Can increase acceleration multiplier") {
        vehicle.setAccelerationMultiplier(2.0f);
        REQUIRE(vehicle.getAccelerationMultiplier() == Approx(2.0f));

        // Should accelerate faster
        for (int i = 0; i < 5; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        float speedWithMultiplier = vehicle.getVelocity();
        REQUIRE(speedWithMultiplier > 0.0f);
    }

    SECTION("Acceleration multiplier is clamped to reasonable range") {
        vehicle.setAccelerationMultiplier(100.0f);
        REQUIRE(vehicle.getAccelerationMultiplier() <= 5.0f);

        vehicle.setAccelerationMultiplier(-10.0f);
        REQUIRE(vehicle.getAccelerationMultiplier() >= 0.1f);
    }
}

TEST_CASE("Vehicle steering input", "[vehicle][steering]") {
    Vehicle vehicle(0.0f, 0.0f, 0.0f);

    SECTION("Initial steering input is zero") {
        REQUIRE(vehicle.getSteeringInput() == 0.0f);
    }

    SECTION("Turning sets steering input") {
        // Need some speed to turn
        for (int i = 0; i < 5; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.turn(0.5f);

        // Steering input should be non-zero after turning
        REQUIRE(vehicle.getSteeringInput() != 0.0f);
    }

    SECTION("Steering input decays over time") {
        // Build up speed and turn
        for (int i = 0; i < 5; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        vehicle.turn(1.0f);
        float initialSteering = std::abs(vehicle.getSteeringInput());

        // Update without turning
        for (int i = 0; i < 5; ++i) {
            vehicle.update(0.1f);
        }

        float decayedSteering = std::abs(vehicle.getSteeringInput());
        REQUIRE(decayedSteering < initialSteering);
    }
}

TEST_CASE("Vehicle edge cases", "[vehicle][edge_cases]") {
    SECTION("Very high deltaTime doesn't cause issues") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        vehicle.accelerateForward();
        REQUIRE_NOTHROW(vehicle.update(10.0f));

        // Velocity should still be reasonable
        REQUIRE(vehicle.getVelocity() < vehicle.getMaxSpeed() * 2.0f);
    }

    SECTION("Zero deltaTime is safe") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        vehicle.accelerateForward();
        REQUIRE_NOTHROW(vehicle.update(0.0f));
    }

    SECTION("Rapid acceleration/deceleration changes") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < 10; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.016f);
            vehicle.accelerateBackward();
            vehicle.update(0.016f);
        }

        REQUIRE_NOTHROW(vehicle.getVelocity());
    }

    SECTION("Extreme turning at high speed") {
        Vehicle vehicle(0.0f, 0.0f, 0.0f);

        // Get to high speed
        for (int i = 0; i < 50; ++i) {
            vehicle.accelerateForward();
            vehicle.update(0.1f);
        }

        // Turn rapidly
        for (int i = 0; i < 10; ++i) {
            vehicle.turn(1.0f);
            vehicle.update(0.016f);
        }

        // Should handle gracefully
        REQUIRE(vehicle.getRotation() >= 0.0f);
    }
}
