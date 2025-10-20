#include "vehicle_renderer.hpp"
#include <threepp/loaders/OBJLoader.hpp>
#include <iostream>
#include <algorithm>

using namespace threepp;

namespace {
    constexpr float MODEL_SCALE = 0.5f;          // Scale factor for loaded 3D models
    constexpr float MODEL_VERTICAL_OFFSET = 0.5f; // Lift car above ground
}

VehicleRenderer::VehicleRenderer(Scene& scene, const Vehicle& vehicle)
    : GameObjectRenderer(scene, vehicle),
      vehicle_(vehicle),
      useCustomModel_(false),
      customModelGroup_(nullptr),
      modelScale_(1.0f) {
    // Now call createModel() after VehicleRenderer is fully constructed
    createModel();
}

bool VehicleRenderer::loadModel(const std::string& modelPath) {
    try {
        OBJLoader loader;
        auto loadedGroup = loader.load(modelPath);

        if (!loadedGroup) {
            std::cerr << "Failed to load model: " << modelPath << std::endl;
            return false;
        }

        // Clear existing model
        if (bodyMesh_) {
            objectGroup_->remove(*bodyMesh_);
            bodyMesh_.reset();
        }

        // Scale and position the loaded model
        float appliedScale = MODEL_SCALE * modelScale_;
        loadedGroup->scale.setScalar(appliedScale);
        loadedGroup->position.y = MODEL_VERTICAL_OFFSET * appliedScale;

        // Enable shadows for all meshes
        loadedGroup->traverse([](Object3D& obj) {
            if (auto mesh = obj.as<Mesh>()) {
                mesh->castShadow = true;
                mesh->receiveShadow = false;
            }
        });

        objectGroup_->add(loadedGroup);
        customModelGroup_ = loadedGroup;
        useCustomModel_ = true;

        std::cout << "Successfully loaded car model: " << modelPath << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return false;
    }
}

void VehicleRenderer::unloadModel() {
    // Remove any custom loaded model and fall back to the box mesh
    if (customModelGroup_) {
        objectGroup_->remove(*customModelGroup_);
        customModelGroup_.reset();
    }
    useCustomModel_ = false;

    // Remove any existing body mesh so createModel can recreate it with current vehicle size
    if (bodyMesh_) {
        objectGroup_->remove(*bodyMesh_);
        bodyMesh_.reset();
    }

    createModel();
}

void VehicleRenderer::applyScale(float scale) {
    modelScale_ = scale;

    if (useCustomModel_ && customModelGroup_) {
        // Apply scale to loaded group
        float appliedScale = MODEL_SCALE * modelScale_;
        customModelGroup_->scale.setScalar(appliedScale);
        customModelGroup_->position.y = MODEL_VERTICAL_OFFSET * appliedScale;
    } else {
        // Recreate fallback mesh with new size from vehicle
        if (bodyMesh_) {
            objectGroup_->remove(*bodyMesh_);
            bodyMesh_.reset();
        }
        createModel();
    }
}

void VehicleRenderer::createModel() {
    std::array<float, 3> size = vehicle_.getSize();

    // Create simple box geometry for vehicle (fallback)
    auto geometry = BoxGeometry::create(size[0], size[1], size[2]);
    auto material = MeshPhongMaterial::create();
    material->color = Color::red;  // Red for player vehicle

    bodyMesh_ = Mesh::create(geometry, material);
    bodyMesh_->position.y = size[1] / 2.0f;  // Half height - positions box so bottom sits at y=0 (on ground)
    bodyMesh_->castShadow = true;

    objectGroup_->add(bodyMesh_);
}
