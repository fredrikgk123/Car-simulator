// AI Assistance: GitHub Copilot was used for OBJ model loading
// and 3D transformation matrix calculations.

#pragma once

#include <threepp/threepp.hpp>

#include "graphics/game_object_renderer.hpp"
#include "core/interfaces/IVehicleState.hpp"
#include <string>

/**
 * Renders the vehicle with support for custom OBJ models.
 * Handles wheel rotation/steering animations and steering wheel rendering.
 */
class VehicleRenderer : public GameObjectRenderer {
  public:
    VehicleRenderer(threepp::Scene& scene, const IVehicleState& vehicleState);

    // Load 3D model from OBJ file
    bool loadModel(const std::string& modelPath);

    // Revert to fallback box geometry
    void unloadModel();

    // Apply runtime scale to the vehicle model
    void applyScale(float scale);

    // Update visual representation with wheel and steering animations
    void update(bool leftPressed = false, bool rightPressed = false);

    // Steering wheel position in vehicle-local coordinates (for camera placement)
    [[nodiscard]] std::array<float, 3> getSteeringWheelPosition() const noexcept;
    [[nodiscard]] bool hasSteeringWheel() const noexcept;

  protected:
    void createModel() override;

  private:
    const IVehicleState& vehicleState_;
    bool useCustomModel_;
    std::shared_ptr<threepp::Object3D> customModelGroup_;
    float modelScale_ = 1.0f;
    float actualAppliedScale_ = 1.0f;

    // Steering wheel
    std::shared_ptr<threepp::Object3D> steeringWheel_;
    std::shared_ptr<threepp::Group> steeringWheelPivot_;
    bool steeringWheelPivotOnCustom_ = false;

    // Wheel objects (front-left, front-right, rear-left, rear-right)
    std::shared_ptr<threepp::Object3D> wheelFL_;
    std::shared_ptr<threepp::Object3D> wheelFR_;
    std::shared_ptr<threepp::Object3D> wheelRL_;
    std::shared_ptr<threepp::Object3D> wheelRR_;

    // Pivot groups for independent steering and rotation
    std::shared_ptr<threepp::Group> wheelFLPivot_;
    std::shared_ptr<threepp::Group> wheelFRPivot_;
    std::shared_ptr<threepp::Group> wheelRLPivot_;
    std::shared_ptr<threepp::Group> wheelRRPivot_;

    // Helper methods
    bool loadWheelModels(const std::string& wheelsDir);
    void unloadWheelModels();
    void applyWheelScaleAndPosition(float appliedScale);

    bool loadSteeringWheel(const std::string& steeringWheelPath);
    void unloadSteeringWheel();
    void applySteeringWheelScaleAndPosition(float appliedScale);

    // Wheel animation state
    std::array<float,3> prevPosition_ = {0.0f, 0.0f, 0.0f};
    float wheelSpinFL_ = 0.0f;
    float wheelSpinFR_ = 0.0f;
    float wheelSpinRL_ = 0.0f;
    float wheelSpinRR_ = 0.0f;

    // Steering wheel smoothing
    float currentSteeringWheelRotation_ = 0.0f;

    // Spin axis detection
    enum class WheelSpinAxis { X, Y, Z };

    // Steering wheel geometry and rotation axis
    std::array<float,3> steeringWheelCenter_ = {0.0f, 0.0f, 0.0f};
    WheelSpinAxis steeringWheelRotationAxis_ = WheelSpinAxis::X;

    // Per-wheel geometry centers
    std::array<float,3> wheelCenterFL_ = {0.0f, 0.0f, 0.0f};
    std::array<float,3> wheelCenterFR_ = {0.0f, 0.0f, 0.0f};
    std::array<float,3> wheelCenterRL_ = {0.0f, 0.0f, 0.0f};
    std::array<float,3> wheelCenterRR_ = {0.0f, 0.0f, 0.0f};

    // Per-wheel spin axes
    WheelSpinAxis wheelSpinAxisFL_ = WheelSpinAxis::X;
    WheelSpinAxis wheelSpinAxisFR_ = WheelSpinAxis::X;
    WheelSpinAxis wheelSpinAxisRL_ = WheelSpinAxis::X;
    WheelSpinAxis wheelSpinAxisRR_ = WheelSpinAxis::X;

    // Per-wheel invert multipliers
    float wheelInvertFL_ = 1.0f;
    float wheelInvertFR_ = 1.0f;
    float wheelInvertRL_ = 1.0f;
    float wheelInvertRR_ = 1.0f;

    bool wheelFLPivotOnCustom_ = false;
    bool wheelFRPivotOnCustom_ = false;
    bool wheelRLPivotOnCustom_ = false;
    bool wheelRRPivotOnCustom_ = false;
};
