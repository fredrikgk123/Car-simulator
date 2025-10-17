#include "scene_manager.hpp"
#include <cmath>

using namespace threepp;

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Camera constants
    constexpr float DEFAULT_CAMERA_DISTANCE = 8.0f;   // 8 units behind - far enough to see surroundings
    constexpr float DEFAULT_CAMERA_HEIGHT = 4.0f;     // 4 units above - provides good viewing angle
    constexpr float DEFAULT_CAMERA_LERP_SPEED = 0.2f; // 0.2 = smooth but responsive
    constexpr float MINIMAP_VIEW_SIZE = 15.0f;        // 15 units - covers 30x30 area
    constexpr float MINIMAP_HEIGHT = 50.0f;           // 50 units high - bird's eye view

    // Hood cam constants
    constexpr float HOOD_CAM_FORWARD_OFFSET = 2.5f;  // 2.5 units in front of vehicle center (moved more forward)
    constexpr float HOOD_CAM_HEIGHT = 1.3f;          // 1.3 units above ground (0.5 car lift + 0.8 hood height)
    constexpr float HOOD_CAM_LOOK_DISTANCE = 10.0f;  // Look 10 units ahead

    // Drift camera constants - SUBTLE effect
    constexpr float DRIFT_SIDE_OFFSET_MAX = 1.5f;    // Maximum 1.5 units to the side (subtle)
    constexpr float DRIFT_ANGLE_THRESHOLD = 0.2f;    // Minimum drift angle to trigger (radians)
    constexpr float DRIFT_OFFSET_LERP_SPEED = 0.12f; // Smooth drift camera transition

    // Ground/Grid constants
    constexpr float GROUND_SIZE = 200.0f;             // 200x200 units - large play area
    constexpr int GRID_DIVISIONS = static_cast<int>(GROUND_SIZE); // Same as ground size - 1 unit per grid square
    constexpr float GRID_Z_OFFSET = 0.01f;            // 0.01 units above ground - prevents z-fighting flickering

    // Camera FOV constants - SPEED BASED
    constexpr float CAMERA_FOV_MIN = 75.0f;           // FOV=75° at idle/low speed
    constexpr float CAMERA_FOV_MAX = 95.0f;           // FOV=95° at max speed (much wider)
    constexpr float NITROUS_FOV_BOOST = 10.0f;        // Additional +10° during nitrous
    constexpr float FOV_LERP_SPEED = 0.08f;           // Smooth FOV transitions (slower for smoothness)
    constexpr float SPEED_FOR_MAX_FOV = 30.0f;        // Speed at which max FOV is reached
    constexpr float CAMERA_NEAR = 0.1f;
    constexpr float CAMERA_FAR = 1000.0f;

    // Lighting constants
    constexpr unsigned int AMBIENT_COLOR = 0x404040;  // Dim gray - prevents completely black shadows
    constexpr float AMBIENT_INTENSITY = 1.0f;
    constexpr unsigned int DIRECTIONAL_COLOR = 0xffffff;
    constexpr float DIRECTIONAL_INTENSITY = 0.8f;     // 80% intensity - bright but not blown out
    constexpr float SHADOW_AREA_SIZE = 100.0f;        // Shadow area: 200x200 units to match play area (GROUND_SIZE)
}

SceneManager::SceneManager()
    : cameraDistance_(DEFAULT_CAMERA_DISTANCE),
      cameraHeight_(DEFAULT_CAMERA_HEIGHT),
      cameraLerpSpeed_(DEFAULT_CAMERA_LERP_SPEED),
      baseFOV_(CAMERA_FOV_MIN),
      currentFOV_(CAMERA_FOV_MIN),
      targetFOV_(CAMERA_FOV_MIN),
      fovLerpSpeed_(FOV_LERP_SPEED),
      cameraMode_(CameraMode::FOLLOW),
      currentCameraX_(0.0f),
      currentCameraY_(DEFAULT_CAMERA_HEIGHT),
      currentCameraZ_(0.0f),
      currentLookAtX_(0.0f),
      currentLookAtY_(0.0f),
      currentLookAtZ_(0.0f),
      driftCameraOffset_(0.0f) {
    scene_ = std::make_shared<Scene>();
    renderer_ = std::make_unique<GLRenderer>();
    renderer_->shadowMap().enabled = true;
}

Scene& SceneManager::getScene() noexcept {
    return *scene_;
}

Camera& SceneManager::getCamera() noexcept {
    return *camera_;
}

GLRenderer& SceneManager::getRenderer() noexcept {
    return *renderer_;
}

void SceneManager::setupLighting() {
    // Ambient light for base illumination
    auto ambientLight = AmbientLight::create(AMBIENT_COLOR, AMBIENT_INTENSITY);
    scene_->add(ambientLight);
    
    // Directional light for main lighting and shadows - CENTERED above play area
    auto directionalLight = DirectionalLight::create(DIRECTIONAL_COLOR, DIRECTIONAL_INTENSITY);
    directionalLight->position.set(0, 50, 0);  // Centered directly above (0,0) at high altitude
    directionalLight->castShadow = true;

    // Configure shadow camera with increased resolution
    auto shadowCamera = directionalLight->shadow->camera->as<OrthographicCamera>();
    shadowCamera->left = -SHADOW_AREA_SIZE;
    shadowCamera->right = SHADOW_AREA_SIZE;
    shadowCamera->top = SHADOW_AREA_SIZE;
    shadowCamera->bottom = -SHADOW_AREA_SIZE;
    shadowCamera->updateProjectionMatrix();

    // Increase shadow map resolution for better quality
    directionalLight->shadow->mapSize.set(4096, 4096);

    scene_->add(directionalLight);
}

void SceneManager::setupGround() {
    // Create ground plane with grass-like green color
    auto groundGeometry = PlaneGeometry::create(GROUND_SIZE, GROUND_SIZE);
    auto groundMaterial = MeshPhongMaterial::create();
    groundMaterial->color = Color(0x3a7d44);  // Grass green color

    groundMesh_ = Mesh::create(groundGeometry, groundMaterial);
    groundMesh_->rotation.x = -math::PI / 2;
    groundMesh_->receiveShadow = true;
    scene_->add(groundMesh_);

    // Add grid helper for visual reference (darker to match grass)
    auto grid = GridHelper::create(GROUND_SIZE, GRID_DIVISIONS, 0x2d5a33, 0x2d5a33);
    grid->position.y = GRID_Z_OFFSET;
    scene_->add(grid);
}

void SceneManager::setupCamera(float aspectRatio) {
    camera_ = std::make_shared<PerspectiveCamera>(CAMERA_FOV_MIN, aspectRatio, CAMERA_NEAR, CAMERA_FAR);
    camera_->position.set(currentCameraX_, currentCameraY_, currentCameraZ_);
}

void SceneManager::setupRenderer(const WindowSize& size) {
    renderer_->setSize(size);
    renderer_->setClearColor(Color::aliceblue);
}

void SceneManager::updateCameraFollowTarget(float targetX, float targetY, float targetZ, float targetRotation, bool nitrousActive, float vehicleVelocity, float driftAngle) {
    float desiredCameraX, desiredCameraY, desiredCameraZ;
    float desiredLookAtX, desiredLookAtY, desiredLookAtZ;

    if (cameraMode_ == CameraMode::HOOD) {
        // Hood cam - position camera at hood level, slightly forward
        desiredCameraX = targetX + (std::sin(targetRotation) * HOOD_CAM_FORWARD_OFFSET);
        desiredCameraY = targetY + HOOD_CAM_HEIGHT;
        desiredCameraZ = targetZ + (std::cos(targetRotation) * HOOD_CAM_FORWARD_OFFSET);

        // Look ahead in the direction vehicle is facing
        desiredLookAtX = targetX + (std::sin(targetRotation) * HOOD_CAM_LOOK_DISTANCE);
        desiredLookAtY = targetY + HOOD_CAM_HEIGHT;
        desiredLookAtZ = targetZ + (std::cos(targetRotation) * HOOD_CAM_LOOK_DISTANCE);

        // Reset drift offset in hood cam
        driftCameraOffset_ = 0.0f;
    } else {
        // Calculate drift camera offset based on drift angle
        float absDriftAngle = std::abs(driftAngle);
        float targetDriftOffset = 0.0f;

        if (absDriftAngle > DRIFT_ANGLE_THRESHOLD) {
            // Scale offset based on drift intensity, clamped to max
            float driftIntensity = std::min(absDriftAngle / 1.0f, 1.0f);  // Normalize to 0-1
            targetDriftOffset = driftIntensity * DRIFT_SIDE_OFFSET_MAX;
            // Apply direction (left or right drift)
            targetDriftOffset *= (driftAngle > 0) ? 1.0f : -1.0f;
        }

        // Smoothly interpolate drift offset
        driftCameraOffset_ += (targetDriftOffset - driftCameraOffset_) * DRIFT_OFFSET_LERP_SPEED;

        // Calculate perpendicular direction for side offset
        float sideAngle = targetRotation + (math::PI / 2.0f);  // 90 degrees to the right

        // Follow cam - position camera behind and above vehicle, with subtle side offset
        desiredCameraX = targetX - (std::sin(targetRotation) * cameraDistance_) + (std::sin(sideAngle) * driftCameraOffset_);
        desiredCameraY = targetY + cameraHeight_;
        desiredCameraZ = targetZ - (std::cos(targetRotation) * cameraDistance_) + (std::cos(sideAngle) * driftCameraOffset_);

        // Look at the vehicle
        desiredLookAtX = targetX;
        desiredLookAtY = targetY;
        desiredLookAtZ = targetZ;
    }

    // Smooth camera interpolation with IMPROVED velocity-adaptive speed to fix jitter at high speeds
    float speedFactor = std::min(std::abs(vehicleVelocity) / 30.0f, 1.0f);  // 0.0 to 1.0
    float adaptiveLerpSpeed = cameraLerpSpeed_ + (speedFactor * 0.12f);  // 0.15 to 0.27 (reduced for more sway)

    // Smoothly interpolate camera position
    currentCameraX_ += (desiredCameraX - currentCameraX_) * adaptiveLerpSpeed;
    currentCameraY_ += (desiredCameraY - currentCameraY_) * adaptiveLerpSpeed;
    currentCameraZ_ += (desiredCameraZ - currentCameraZ_) * adaptiveLerpSpeed;

    // Smoothly interpolate look-at target with slightly different speed for more dynamic feel
    float lookAtLerpSpeed = adaptiveLerpSpeed * 1.15f;  // Look-at follows 15% faster for subtle lag effect
    currentLookAtX_ += (desiredLookAtX - currentLookAtX_) * lookAtLerpSpeed;
    currentLookAtY_ += (desiredLookAtY - currentLookAtY_) * lookAtLerpSpeed;
    currentLookAtZ_ += (desiredLookAtZ - currentLookAtZ_) * lookAtLerpSpeed;

    // Apply smoothed position and look-at
    camera_->position.set(currentCameraX_, currentCameraY_, currentCameraZ_);
    camera_->lookAt(currentLookAtX_, currentLookAtY_, currentLookAtZ_);
}

void SceneManager::updateMinimapCamera(float targetX, float targetZ) {
    // Keep minimap centered on vehicle
    minimapCamera_->position.set(targetX, MINIMAP_HEIGHT, targetZ);
    minimapCamera_->lookAt(targetX, 0.0f, targetZ);
}

void SceneManager::updateCameraFOV(bool nitrousActive, float vehicleVelocity) {
    // Calculate target FOV based on speed and nitrous
    float targetFOV = CAMERA_FOV_MIN + ((std::min(vehicleVelocity, SPEED_FOR_MAX_FOV) / SPEED_FOR_MAX_FOV) * (CAMERA_FOV_MAX - CAMERA_FOV_MIN));
    if (nitrousActive) {
        targetFOV += NITROUS_FOV_BOOST;  // Add nitrous boost
    }

    // Smoothly interpolate current FOV to target
    currentFOV_ += (targetFOV - currentFOV_) * fovLerpSpeed_;

    // Apply FOV to camera
    camera_->fov = currentFOV_;
    camera_->updateProjectionMatrix();
}

void SceneManager::render() {
    renderer_->render(*scene_, *camera_);
}

void SceneManager::resize(const WindowSize& size) {
    camera_->aspect = size.aspect();
    camera_->updateProjectionMatrix();
    renderer_->setSize(size);
}

void SceneManager::setupMinimapCamera(float aspectRatio) {
    // Orthographic camera for top-down minimap view
    minimapCamera_ = std::make_shared<OrthographicCamera>(
        -MINIMAP_VIEW_SIZE * aspectRatio, MINIMAP_VIEW_SIZE * aspectRatio,
        MINIMAP_VIEW_SIZE, -MINIMAP_VIEW_SIZE,
        CAMERA_NEAR, CAMERA_FAR
    );

    // Position camera above scene looking down
    minimapCamera_->position.set(0, MINIMAP_HEIGHT, 0);
}

void SceneManager::setCameraMode(CameraMode mode) noexcept {
    cameraMode_ = mode;
}

CameraMode SceneManager::getCameraMode() const noexcept {
    return cameraMode_;
}

void SceneManager::toggleCameraMode() noexcept {
    cameraMode_ = (cameraMode_ == CameraMode::FOLLOW) ? CameraMode::HOOD : CameraMode::FOLLOW;
}

void SceneManager::renderMinimap() {
    renderer_->render(*scene_, *minimapCamera_);
}
