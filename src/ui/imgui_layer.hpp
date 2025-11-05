#pragma once

#include <threepp/threepp.hpp>
#include "../core/vehicle.hpp"

class ImGuiLayer {
public:
    ImGuiLayer();

    // Render ImGui widgets. Called between ImGui::NewFrame() and ImGui::Render().
    void render(const Vehicle& vehicle, const threepp::WindowSize& size);

private:
    // Smoothed display state for needles to avoid jitter
    float displayed_speed_ratio_ = 0.0f;
    float displayed_rpm_ratio_ = 0.0f;
    float smoothing_alpha_ = 0.18f; // 0..1 (higher = snappier)
};
