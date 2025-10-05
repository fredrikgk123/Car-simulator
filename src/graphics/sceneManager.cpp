#include "sceneManager.hpp"

using namespace threepp;

SceneManager::SceneManager() {
    scene_ = std::make_shared<Scene>();
    renderer_ = std::make_unique<GLRenderer>();
}

Scene& SceneManager::getScene() {
    return *scene_;
}

Camera& SceneManager::getCamera() {
    return *camera_;
}

GLRenderer& SceneManager::getRenderer() {
    return *renderer_;
}

void SceneManager::setupLighting() {
    auto ambientLight = AmbientLight::create(0x404040);
    scene_->add(ambientLight);
    
    auto directionalLight = DirectionalLight::create(0xffffff, 0.8f);
    directionalLight->position.set(5, 10, 7);
    directionalLight->castShadow = true;
    scene_->add(directionalLight);
}

void SceneManager::setupGround() {
    auto groundGeometry = PlaneGeometry::create(50, 50);
    auto groundMaterial = MeshPhongMaterial::create();
    groundMaterial->color = Color::gray;
    
    groundMesh_ = Mesh::create(groundGeometry, groundMaterial);
    groundMesh_->rotation.x = -math::PI / 2;
    groundMesh_->receiveShadow = true;
    scene_->add(groundMesh_);
}

void SceneManager::setupCamera(float aspectRatio) {
    camera_ = std::make_shared<PerspectiveCamera>(75, aspectRatio, 0.1f, 1000.0f);
    camera_->position.set(0, 10, 15);
    camera_->lookAt(Vector3(0, 0, 0));
}

void SceneManager::render() {
    renderer_->render(*scene_, *camera_);
}

void SceneManager::resize(const WindowSize& size) {
    camera_->aspect = size.aspect();
    camera_->updateProjectionMatrix();
    renderer_->setSize(size);
}
