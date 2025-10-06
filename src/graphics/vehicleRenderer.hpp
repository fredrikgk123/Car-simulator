#pragma once

#include <threepp/threepp.hpp>

#include "gameObjectRenderer.hpp"
#include "vehicle.hpp"

class VehicleRenderer : public GameObjectRenderer {
  public:
    VehicleRenderer(threepp::Scene& scene, const Vehicle& vehicle);

  protected:
    // Override to create vehicle-specific model (red box)
    void createModel() override;

  private:
    const Vehicle& vehicle_;
};
