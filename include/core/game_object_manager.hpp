#pragma once

#include <memory>
#include <vector>
#include "core/game_object.hpp"

/**
 * Abstract interface for managing collections of game objects.
 * Provides common operations for updating, collision handling, and resetting.
 */
class GameObjectManager {
public:
    virtual ~GameObjectManager() = default;

    virtual void update(float deltaTime) = 0;
    virtual void handleCollisions(class Vehicle& vehicle) = 0;
    virtual void reset() noexcept = 0;
    virtual size_t getCount() const = 0;
};
