#pragma once

#include <array>

/**
 * Base for all game entities.
 * Handles position, rotation, collision, and active/inactive state.
 */
class GameObject {
public:
    GameObject(float x, float y, float z);
    virtual ~GameObject() = default;

    // Update method - must be implemented by derived classes
    virtual void update(float deltaTime) = 0;

    // Reset to initial state
    virtual void reset() noexcept;

    // Getters
    [[nodiscard]] virtual const std::array<float, 3>& getPosition() const noexcept;
    [[nodiscard]] virtual float getRotation() const noexcept;
    [[nodiscard]] const std::array<float, 3>& getSize() const noexcept;
    [[nodiscard]] virtual bool isActive() const noexcept;

    // Setters
    void setPosition(float x, float y, float z) noexcept;
    void setRotation(float rotation) noexcept;
    void setActive(bool active) noexcept;

    // Circle collision with detailed info
    [[nodiscard]] bool checkCircleCollision(const GameObject& other, float& overlapDistance, float& normalX, float& normalZ) const noexcept;

    // Quick collision check
    [[nodiscard]] bool intersects(const GameObject& other) const noexcept;

protected:
    // Transform
    std::array<float, 3> position_;
    std::array<float, 3> initialPosition_;
    float rotation_;
    float initialRotation_;

    // Collision
    std::array<float, 3> size_;
    float collisionRadius_;

    // State
    bool active_;

    // Recalculate collision radius when size changes
    void updateCollisionRadius() noexcept;
};
