#include "vehicleRenderer.hpp"
#include <cmath>

using namespace threepp;

VehicleRenderer::VehicleRenderer(SceneManager& sceneManager, const Vehicle& vehicle)
    : sceneManager_(sceneManager), vehicle_(vehicle) {

    // Create a group for the vehicle
    vehicleGroup_ = std::make_shared<Group>();
    sceneManager_.getScene().add(vehicleGroup_);

    createVehicleModel();
}

VehicleRenderer::~VehicleRenderer() {
    // Remove from scene
    if (vehicleGroup_) {
        sceneManager_.getScene().remove(*vehicleGroup_);
    }
}

void VehicleRenderer::createVehicleModel() {
    // Get vehicle dimensions
    auto size = vehicle_.getSize();

    // Create body geometry (a simple box)
    auto geometry = BoxGeometry::create(size[0], size[1], size[2]);
    auto material = MeshPhongMaterial::create(); // Changed to MeshPhongMaterial like in example
    material->color = Color::red;

    bodyMesh_ = Mesh::create(geometry, material);
    bodyMesh_->position.y = size[1] / 2;  // Adjust position so bottom is at y=0
    bodyMesh_->castShadow = true;

    vehicleGroup_->add(bodyMesh_);
}

void VehicleRenderer::update() {
    // Update position and rotation from vehicle model
    auto position = vehicle_.getPosition();
    vehicleGroup_->position.set(position[0], position[1], position[2]);
    vehicleGroup_->rotation.y = vehicle_.getRotation();
}

void VehicleRenderer::setColor(const Color& color) {
    if (bodyMesh_ && vehicleGroup_) {
        // Get vehicle dimensions
        auto size = vehicle_.getSize();

        // Remove existing mesh from group
        vehicleGroup_->remove(*bodyMesh_);

        // Create new geometry and material
        auto geometry = BoxGeometry::create(size[0], size[1], size[2]);
        auto material = MeshPhongMaterial::create();
        material->color = color;

        // Create new mesh
        bodyMesh_ = Mesh::create(geometry, material);
        bodyMesh_->position.y = size[1] / 2;
        bodyMesh_->castShadow = true;

        // Add new mesh to group
        vehicleGroup_->add(bodyMesh_);
    }
}
