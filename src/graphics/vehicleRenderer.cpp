#include "vehicleRenderer.hpp"

using namespace threepp;

VehicleRenderer::VehicleRenderer(Scene& scene, const Vehicle& vehicle)
    : GameObjectRenderer(scene, vehicle),
      vehicle_(vehicle) {
    // Now call createModel() after VehicleRenderer is fully constructed
    createModel();
}

void VehicleRenderer::createModel() {
    std::array<float, 3> size = vehicle_.getSize();

    // Create simple box geometry for vehicle
    auto geometry = BoxGeometry::create(size[0], size[1], size[2]);
    auto material = MeshPhongMaterial::create();
    material->color = Color::red;  // Red for player vehicle

    bodyMesh_ = Mesh::create(geometry, material);
    bodyMesh_->position.y = size[1] / 2.0f;  // Half height - positions box so bottom sits at y=0 (on ground)
    bodyMesh_->castShadow = true;

    objectGroup_->add(bodyMesh_);
}
