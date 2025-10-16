#pragma once

#include <threepp/threepp.hpp>

#include "game_object_renderer.hpp"
#include "../core/vehicle.hpp"
#include <string>

class VehicleRenderer : public GameObjectRenderer {
  public:
    VehicleRenderer(threepp::Scene& scene, const Vehicle& vehicle);

    // Load a 3D model from file (OBJ format)
    bool loadModel(const std::string& modelPath);

  protected:
    // Override to create vehicle-specific model
    void createModel() override;

  private:
    const Vehicle& vehicle_;
    bool useCustomModel_;
};
