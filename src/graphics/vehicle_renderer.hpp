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

    // Unload any custom model and fall back to the box model
    void unloadModel();

    // Apply a runtime scale to the vehicle model (either custom or fallback)
    void applyScale(float scale);

  protected:
    // Override to create vehicle-specific model
    void createModel() override;

  private:
    const Vehicle& vehicle_;
    bool useCustomModel_;
    std::shared_ptr<threepp::Object3D> customModelGroup_;
    float modelScale_ = 1.0f; // runtime scale applied to loaded model
};
