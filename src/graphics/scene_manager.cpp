#include "scene_manager.hpp"
#include <cmath>

using namespace threepp;

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Camera constants
    constexpr float DEFAULT_CAMERA_DISTANCE = 8.0f;   // 8 units behind - default follow distance
    constexpr float DEFAULT_CAMERA_HEIGHT = 4.0f;     // 4 units above - default follow height
    constexpr float DEFAULT_CAMERA_LERP_SPEED = 0.2f; // 0.2 = smooth but responsive
    constexpr float MINIMAP_VIEW_SIZE = 15.0f;        // 15 units - covers 30x30 area
    constexpr float MINIMAP_HEIGHT = 50.0f;           // 50 units high - bird's eye view

    // Hood cam constants
    constexpr float HOOD_CAM_FORWARD_OFFSET = 2.5f;  // 2.5 units in front of vehicle center (moved more forward)
    constexpr float HOOD_CAM_HEIGHT = 1.3f;          // 1.3 units above ground (0.5 car lift + 0.8 hood height)
    constexpr float HOOD_CAM_LOOK_DISTANCE = 10.0f;  // Look 10 units ahead

    // Side cam constants
    constexpr float SIDE_CAM_DISTANCE = 6.0f;       // 6 units to the side of the vehicle
    constexpr float SIDE_CAM_HEIGHT = 2.0f;         // 2 units above ground for side view

    // Inside/cockpit cam constants
    constexpr float INSIDE_CAM_FORWARD_OFFSET = -0.15f; // Move camera slightly back inside cabin
    constexpr float INSIDE_CAM_HEIGHT = 1.4f;           // Slightly higher eye position to clear interior
    constexpr float INSIDE_CAM_SIDE_OFFSET = 0.45f;     // Lateral offset to the right (behind wheel)
    // Reduced look distance to match shortened wheelbase
    constexpr float INSIDE_CAM_LOOK_DISTANCE = 8.0f; // Look slightly ahead from inside
    // Match the visual lowering applied to the car model so the cockpit aligns visually
    constexpr float CAR_LOWER_AMOUNT = 0.30f; // matches vehicle_renderer CAR_LOWER_AMOUNT

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

    // Cockpit/inside camera FOV tuning (reduce dramatic changes)
    constexpr float COCKPIT_FOV_FACTOR = 0.5f;        // Reduce speed-based FOV change to 50% in cockpit
    constexpr float COCKPIT_NITROUS_FACTOR = 0.5f;    // Reduce nitrous FOV boost by half in cockpit
    constexpr float COCKPIT_FOV_LERP_MULT = 0.6f;     // Slow FOV interpolation in cockpit for smoother change
    // Smoothing specific to nitrous application (when nitrous toggles, make FOV change smoother)
    constexpr float NITROUS_FOV_LERP_MULT = 0.25f;   // Reduce interpolation speed to 25% when nitrous is active

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
      cameraSideDistance_(SIDE_CAM_DISTANCE),
      cameraSideHeight_(SIDE_CAM_HEIGHT),
      cameraInsideForwardOffset_(INSIDE_CAM_FORWARD_OFFSET),
      cameraInsideHeight_(INSIDE_CAM_HEIGHT),
      cameraInsideSideOffset_(INSIDE_CAM_SIDE_OFFSET),
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
    } else if (cameraMode_ == CameraMode::SIDE) {
        // Side cam - position camera to the side of the vehicle and look at it
        float sideAngle = targetRotation + (math::PI / 2.0f);  // 90 degrees to the right

        desiredCameraX = targetX + (std::sin(sideAngle) * cameraSideDistance_);
        desiredCameraY = targetY + cameraSideHeight_;
        desiredCameraZ = targetZ + (std::cos(sideAngle) * cameraSideDistance_);

        // Look at the vehicle center
        desiredLookAtX = targetX;
        desiredLookAtY = targetY;
        desiredLookAtZ = targetZ;

        // No drift offset applied in side view
        driftCameraOffset_ = 0.0f;
    } else if (cameraMode_ == CameraMode::INSIDE) {
        // Inside/cockpit cam - approximate driver's eye position inside the vehicle
        desiredCameraX = targetX + (std::sin(targetRotation) * cameraInsideForwardOffset_);
        // Subtract the visual lowering amount so camera aligns with lowered body
        desiredCameraY = targetY + cameraInsideHeight_ - CAR_LOWER_AMOUNT;
        desiredCameraZ = targetZ + (std::cos(targetRotation) * cameraInsideForwardOffset_);

        // Apply lateral offset to position the camera behind the (right-side) wheel
        {
            float sideAngle = targetRotation + (math::PI / 2.0f); // right vector
            desiredCameraX += std::sin(sideAngle) * cameraInsideSideOffset_;
            desiredCameraZ += std::cos(sideAngle) * cameraInsideSideOffset_;
        }

        // Look ahead from inside the cabin
        desiredLookAtX = targetX + (std::sin(targetRotation) * INSIDE_CAM_LOOK_DISTANCE);
        // Align look-at with visual lowering
        desiredLookAtY = targetY + cameraInsideHeight_ - CAR_LOWER_AMOUNT;
        desiredLookAtZ = targetZ + (std::cos(targetRotation) * INSIDE_CAM_LOOK_DISTANCE);

        // Reset drift offset in inside cam
        driftCameraOffset_ = 0.0f;

        // Immediate, non-interpolated cockpit camera: set position and lookAt directly
        camera_->position.set(desiredCameraX, desiredCameraY, desiredCameraZ);
        camera_->lookAt(desiredLookAtX, desiredLookAtY, desiredLookAtZ);

        // Keep internal smoothed state in sync so other modes don't jump
        currentCameraX_ = desiredCameraX;
        currentCameraY_ = desiredCameraY;
        currentCameraZ_ = desiredCameraZ;
        currentLookAtX_ = desiredLookAtX;
        currentLookAtY_ = desiredLookAtY;
        currentLookAtZ_ = desiredLookAtZ;

        // Done — skip interpolation below
        return;
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
    // Calculate base speed ratio (0..1)
    float speedRatio = std::min(vehicleVelocity, SPEED_FOR_MAX_FOV) / SPEED_FOR_MAX_FOV;

    // Base FOV calculation (speed-based portion)
    float fovRange = CAMERA_FOV_MAX - CAMERA_FOV_MIN;

    float targetFOV = CAMERA_FOV_MIN + (speedRatio * fovRange);

    // Apply nitrous boost normally
    if (nitrousActive) {
        targetFOV += NITROUS_FOV_BOOST;
    }

    // Adjust the target FOV for cockpit mode (reduced magnitude)
    if (cameraMode_ == CameraMode::INSIDE) {
        targetFOV = CAMERA_FOV_MIN + (speedRatio * fovRange * COCKPIT_FOV_FACTOR);
        if (nitrousActive) targetFOV += (NITROUS_FOV_BOOST * COCKPIT_NITROUS_FACTOR);
    }

    // Compute lerp speed and reduce it when nitrous is active to make the transition smoother
    float lerpSpeed = fovLerpSpeed_;
    if (cameraMode_ == CameraMode::INSIDE) lerpSpeed *= COCKPIT_FOV_LERP_MULT;
    if (nitrousActive) lerpSpeed *= NITROUS_FOV_LERP_MULT;

    // Interpolate current FOV toward target using the computed lerpSpeed (applies also to cockpit now)
    currentFOV_ += (targetFOV - currentFOV_) * lerpSpeed;

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
    if (cameraMode_ == CameraMode::FOLLOW) {
        cameraMode_ = CameraMode::HOOD;
    } else if (cameraMode_ == CameraMode::HOOD) {
        cameraMode_ = CameraMode::SIDE;
    } else if (cameraMode_ == CameraMode::SIDE) {
        cameraMode_ = CameraMode::INSIDE;
    } else {
        cameraMode_ = CameraMode::FOLLOW;
    }
}

void SceneManager::renderMinimap() {
    renderer_->render(*scene_, *minimapCamera_);
}
