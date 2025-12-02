#pragma once

#include <array>
#include <vector>
#include <random>
#include <cmath>

/**
 * Generates random positions with spacing constraints.
 * Used for placing trees and powerups.
 */
class RandomPositionGenerator {
public:
    RandomPositionGenerator(float playAreaSize, float margin)
        : randomEngine_(std::random_device{}()),
          minPos_(-(playAreaSize / 2.0f) + margin),
          maxPos_((playAreaSize / 2.0f) - margin),
          distribution_(minPos_, maxPos_) {
    }

    std::array<float, 2> getRandomPosition() {
        return {distribution_(randomEngine_), distribution_(randomEngine_)};
    }

    // Keep minimum distance from existing positions
    std::array<float, 2> getRandomPositionWithMinDistance(
        const std::vector<std::array<float, 2>>& existingPositions,
        float minDistance,
        int maxAttempts = 100) {

        int attempts = 0;
        while (attempts < maxAttempts) {
            attempts++;
            auto pos = getRandomPosition();

            if (isPositionValid(pos, existingPositions, minDistance)) {
                return pos;
            }
        }

        return getRandomPosition();
    }

    // Distance from both center and other positions
    std::array<float, 2> getRandomPositionWithConstraints(
        const std::vector<std::array<float, 2>>& existingPositions,
        float minDistanceFromCenter,
        float minDistanceFromOthers,
        int maxAttempts = 100) {

        int attempts = 0;
        while (attempts < maxAttempts) {
            attempts++;
            auto pos = getRandomPosition();

            float distFromCenter = std::sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
            if (distFromCenter < minDistanceFromCenter) {
                continue;
            }

            if (isPositionValid(pos, existingPositions, minDistanceFromOthers)) {
                return pos;
            }
        }

        return getRandomPosition();
    }

private:
    bool isPositionValid(
        const std::array<float, 2>& pos,
        const std::vector<std::array<float, 2>>& existingPositions,
        float minDistance) const {

        for (const auto& existing : existingPositions) {
            float dx = pos[0] - existing[0];
            float dz = pos[1] - existing[1];
            float distance = std::sqrt(dx * dx + dz * dz);

            if (distance < minDistance) {
                return false;
            }
        }

        return true;
    }

    std::mt19937 randomEngine_;
    float minPos_;
    float maxPos_;
    std::uniform_real_distribution<float> distribution_;
};
