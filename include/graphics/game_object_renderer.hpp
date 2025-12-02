#pragma once

#include <threepp/threepp.hpp>
#include <memory>
#include "core/game_object.hpp"

/**
 * Base renderer for game objects.
 * Synchronizes 3D visual representation with logical game object state.
 */
class GameObjectRenderer {
public:
    GameObjectRenderer(threepp::Scene& scene, const GameObject& gameObject);
    virtual ~GameObjectRenderer();

    // Update visual representation to match game object state
    virtual void update();

    void setVisible(bool visible);

protected:
    // Override to create custom 3D models
    virtual void createModel();

    threepp::Scene& scene_;
    const GameObject& gameObject_;
    std::shared_ptr<threepp::Group> objectGroup_;
    std::shared_ptr<threepp::Mesh> bodyMesh_;
};
