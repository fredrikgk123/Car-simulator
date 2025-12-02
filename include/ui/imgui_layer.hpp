#pragma once

#include <threepp/threepp.hpp>
#include "core/interfaces/IVehicleState.hpp"

/**
 * Renders ImGui dashboard overlay with speedometer, RPM gauge, and gear indicator.
 * Uses smoothed values for realistic needle movement.
 */
class ImGuiLayer {
public:
    ImGuiLayer();

    // Render UI elements (call between ImGui::NewFrame() and ImGui::Render())
    void render(const IVehicleState& vehicle, const threepp::WindowSize& size);

private:
    // Smoothed display state for realistic gauge needles
    float displayedSpeedRatio_ = 0.0f;
    float displayedRpmRatio_ = 0.0f;
    float smoothingAlpha_ = 0.18f;
};
