#pragma once

#include <threepp/threepp.hpp>
#include <memory>

class SceneManager {
public:
    SceneManager();

    // Scene access
    threepp::Scene& getScene();
    threepp::Camera& getCamera();
    threepp::GLRenderer& getRenderer();

    // Setup methods
    void setupLighting();
    void setupGround();
    void setupCamera(float aspectRatio);
    void setupRenderer(const threepp::WindowSize& size);
    void setupMinimapCamera(float aspectRatio);  // Setup overhead minimap camera

    // Camera control
    void updateCameraFollowTarget(float targetX, float targetY, float targetZ, float targetRotation);
    void updateMinimapCamera(float targetX, float targetZ);  // Update minimap to follow vehicle

    // Rendering
    void render();
    void renderMinimap();  // Render minimap view
    void resize(const threepp::WindowSize& size);

    // Getters
    threepp::PerspectiveCamera& getMinimapCamera();

private:
    std::unique_ptr<threepp::GLRenderer> renderer_;
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera_;
    std::shared_ptr<threepp::OrthographicCamera> minimapCamera_;  // Top-down camera for minimap
    std::shared_ptr<threepp::Mesh> groundMesh_;

    // Camera follow parameters
    float cameraDistance_;
    float cameraHeight_;
    float cameraLerpSpeed_;        // Smoothing factor for camera movement

    // Current camera target for smooth interpolation
    float currentCameraX_;
    float currentCameraY_;
    float currentCameraZ_;
    float currentLookAtX_;
    float currentLookAtY_;
    float currentLookAtZ_;
};
