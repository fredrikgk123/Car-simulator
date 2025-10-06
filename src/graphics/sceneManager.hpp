#pragma once

#include <threepp/threepp.hpp>
#include <memory>

class SceneManager {
public:
    SceneManager();

    // Scene component access
    threepp::Scene& getScene();
    threepp::Camera& getCamera();
    threepp::GLRenderer& getRenderer();

    // Setup methods
    void setupLighting();
    void setupGround();
    void setupCamera(float aspectRatio);
    void setupRenderer(const threepp::WindowSize& size);
    void setupMinimapCamera(float aspectRatio);

    // Camera control
    void updateCameraFollowTarget(float targetX, float targetY, float targetZ, float targetRotation);
    void updateMinimapCamera(float targetX, float targetZ);

    // Rendering
    void render();
    void renderMinimap();
    void resize(const threepp::WindowSize& size);

    // Camera constants - exposed for external configuration if needed
    static const float DEFAULT_CAMERA_DISTANCE;
    static const float DEFAULT_CAMERA_HEIGHT;
    static const float DEFAULT_CAMERA_LERP_SPEED;
    static const float MINIMAP_VIEW_SIZE;
    static const float MINIMAP_HEIGHT;

private:
    // Ground/Grid constants
    static const float GROUND_SIZE;
    static const int GRID_DIVISIONS;
    static const float GRID_Z_OFFSET;

    // Camera constants
    static const float CAMERA_FOV;
    static const float CAMERA_NEAR;
    static const float CAMERA_FAR;

    // Lighting constants
    static const unsigned int AMBIENT_COLOR;
    static const float AMBIENT_INTENSITY;
    static const unsigned int DIRECTIONAL_COLOR;
    static const float DIRECTIONAL_INTENSITY;
    static const float SHADOW_AREA_SIZE;

    std::unique_ptr<threepp::GLRenderer> renderer_;
    std::shared_ptr<threepp::Scene> scene_;
    std::shared_ptr<threepp::PerspectiveCamera> camera_;
    std::shared_ptr<threepp::OrthographicCamera> minimapCamera_;
    std::shared_ptr<threepp::Mesh> groundMesh_;

    // Camera follow parameters
    float cameraDistance_;
    float cameraHeight_;
    float cameraLerpSpeed_;

    // Current camera state for smooth interpolation
    float currentCameraX_;
    float currentCameraY_;
    float currentCameraZ_;
    float currentLookAtX_;
    float currentLookAtY_;
    float currentLookAtZ_;
};
