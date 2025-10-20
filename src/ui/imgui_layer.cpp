#include "imgui_layer.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <string>
#include <cfloat>

ImGuiLayer::ImGuiLayer() = default;

static inline ImU32 toU32(const ImVec4 &c){ return IM_COL32((int)(c.x*255),(int)(c.y*255),(int)(c.z*255),(int)(c.w*255)); }

void ImGuiLayer::render(const Vehicle& vehicle, const threepp::WindowSize& size) {
    if (!ImGui::GetCurrentContext()) return;

    // HUD constants
    constexpr float MAX_DISPLAY_SPEED_KMH = 150.0f;
    constexpr float MAX_RPM = 7000.0f;
    constexpr float PI = 3.14159265358979323846f;

    const float speed_kmh = std::abs(vehicle.getVelocity()) * 3.6f;
    const float speed_ratio = std::clamp(speed_kmh / MAX_DISPLAY_SPEED_KMH, 0.0f, 1.0f);
    const float rpm = vehicle.getRPM();
    const float rpm_ratio = std::clamp(rpm / MAX_RPM, 0.0f, 1.0f);
    const int gear = vehicle.getCurrentGear();

    ImDrawList* dl = ImGui::GetForegroundDrawList();

    // Gauge drawing helper (arc gauge with ticks and needle)
    auto drawGauge = [&](ImVec2 center, float radius, float ratio, const char* label, const char* valueText, ImU32 color, float scaleMax){
        // detect speed gauge so we can draw a digital speed box inside it (styled per reference)
        const bool isSpeed = (std::string(label) == "km/h" || std::string(label) == "mph");
        // detect RPM gauge — skip numeric tick labels for RPM to avoid clutter/overlap
        const bool isRPM = (std::string(label) == "RPM");
         // scale many of the drawing constants with radius so the gauge is fully scalable
         const float thickness = std::max(4.0f, radius * 0.04f);
         // Angles (radians): gauge spans -120deg to +120deg
         const float a1 = -120.0f * (PI/180.0f);
         const float a2 =  120.0f * (PI/180.0f);

        // Background circle (subtle)
        dl->AddCircleFilled(center, radius, toU32(ImVec4(0.f,0.f,0.f,0.45f)));

        // Draw outer arc (empty)
        dl->PathClear();
        dl->PathArcTo(center, radius - std::max(6.0f, radius*0.06f), a1, a2, 64);
        dl->PathStroke(toU32(ImVec4(0.25f,0.25f,0.25f,0.9f)), false, thickness);

        // Draw filled arc based on ratio
        if (ratio > 0.0001f) {
            dl->PathClear();
            dl->PathArcTo(center, radius - std::max(6.0f, radius*0.06f), a1, a1 + (a2 - a1) * ratio, 64);
            dl->PathStroke(color, false, thickness + std::max(1.0f, radius*0.02f));
        }

        // Draw ticks and numeric labels at major ticks
        const int ticks = 12;
        for (int i = 0; i <= ticks; ++i){
            float t = (float)i / (float)ticks;
            float a = a1 + (a2 - a1) * t;
            bool major = (i % 3 == 0);
            float p1r = radius - std::max(8.0f, radius*0.07f);
            float p2r = radius - (major ? std::max( (float) (radius*0.14f),  (float)28.0f) : std::max((float)(radius*0.09f),(float)18.0f));
            ImVec2 p1 = ImVec2(center.x + cosf(a) * p1r, center.y + sinf(a) * p1r);
            ImVec2 p2 = ImVec2(center.x + cosf(a) * p2r, center.y + sinf(a) * p2r);
            // reduce and simplify ticks for the speed gauge: draw ticks but skip numeric labels
            dl->AddLine(p1, p2, toU32(ImVec4(0.95f,0.95f,0.95f,0.9f)), major? std::max(2.0f, radius*0.03f) : std::max(1.0f, radius*0.015f));
            // Only draw numeric labels for non-speed, non-RPM gauges (avoid clutter for RPM)
            if (!isSpeed && !isRPM && major) {
                // numeric label (not shown on speed gauge — speed will use a digital readout)
                char lbl[16];
                int val = static_cast<int>(std::round(scaleMax * t));
                snprintf(lbl, sizeof(lbl), "%d", val);
                ImVec2 lpos = ImVec2(center.x + cosf(a) * (radius - std::max( (float)44.0f, radius*0.18f)) - 8.0f, center.y + sinf(a) * (radius - std::max((float)44.0f, radius*0.18f)) - 6.0f);
                float tickFont = std::max(9.0f, radius * 0.10f);
                dl->AddText(ImGui::GetFont(), tickFont, lpos, toU32(ImVec4(0.85f,0.85f,0.85f,0.95f)), lbl);
            }
        }

        // Needle (triangular/filled for a car-dash feel)
        float needleA = a1 + (a2 - a1) * ratio;
        ImVec2 tip = ImVec2(center.x + cosf(needleA) * (radius - std::max(20.0f, radius*0.12f)), center.y + sinf(needleA) * (radius - std::max(20.0f, radius*0.12f)));
        float bw = std::max(4.0f, radius * 0.06f); // base width
        // base offset points perpendicular to needle
        ImVec2 dir = ImVec2(cosf(needleA), sinf(needleA));
        ImVec2 perp = ImVec2(-dir.y, dir.x);
        ImVec2 b1 = ImVec2(center.x + perp.x * bw, center.y + perp.y * bw);
        ImVec2 b2 = ImVec2(center.x - perp.x * bw, center.y - perp.y * bw);
        dl->AddTriangleFilled(b1, b2, tip, color);
        // add highlight line
        dl->AddLine(center, tip, toU32(ImVec4(1.0f,0.85f,0.85f,0.95f)), std::max(1.0f, radius*0.008f));
        dl->AddCircleFilled(center, std::max(6.0f, radius*0.08f), toU32(ImVec4(0.12f,0.12f,0.12f,1.0f)));
        dl->AddCircle(center, std::max(6.0f, radius*0.08f), toU32(ImVec4(0.95f,0.95f,0.95f,0.06f)), 12, std::max(1.0f, radius*0.02f));

        // Label and value text
        ImFont* font = ImGui::GetFont();
        if (isSpeed) {
            // Draw a digital speed box inside the gauge (right side) similar to the provided reference
            float boxW = radius * 0.56f;
            float boxH = radius * 0.38f;
            // Shift the digital speed box even further to the left to create space to its right
            // (now moved to -0.36 of radius) and nudge it down a bit
            // Increase vertical shift so the speed box moves further down
            float vShift = radius * 0.24f; // move both elements further down
            ImVec2 boxCenter = ImVec2(center.x - radius * 0.36f, center.y + radius * 0.02f + vShift);
            ImVec2 boxTL = ImVec2(boxCenter.x - boxW*0.5f, boxCenter.y - boxH*0.5f);
            ImVec2 boxBR = ImVec2(boxCenter.x + boxW*0.5f, boxCenter.y + boxH*0.5f);
            float boxR = std::max(4.0f, radius * 0.06f);
            dl->AddRectFilled(boxTL, boxBR, toU32(ImVec4(0.08f,0.08f,0.08f,0.95f)), boxR);
            dl->AddRect(boxTL, boxBR, toU32(ImVec4(1.0f,1.0f,1.0f,0.06f)), boxR, 0, std::max(1.0f, radius*0.02f));
            // speed number
            float speedFont = std::max(22.0f, radius * 0.38f);
            // small unit label
            const char* unit = (std::string(label) == "mph")? "mph" : "km/h";
            float unitFont = std::max(10.0f, radius * 0.12f);
            ImVec2 unitSize = font->CalcTextSizeA(unitFont, FLT_MAX, 0.0f, unit);
            // padding and layout
            float padding = std::max(4.0f, radius * 0.04f);
            // available width for the speed text (reserve space for unit on the right)
            float availableWidth = boxW - padding*2.0f - unitSize.x - padding*0.5f;
            ImVec2 speedTxt = font->CalcTextSizeA(speedFont, FLT_MAX, 0.0f, valueText);
            if (speedTxt.x > availableWidth) {
                // reduce font to fit the available width
                float scale = availableWidth / speedTxt.x;
                speedFont = std::max(12.0f, speedFont * scale);
                speedTxt = font->CalcTextSizeA(speedFont, FLT_MAX, 0.0f, valueText);
            }
            // left-align speed with padding inside the box
            float speedX = boxCenter.x - boxW*0.5f + padding;
            // vertically center speed a bit above center
            float speedY = boxCenter.y - speedTxt.y*0.6f;
            dl->AddText(font, speedFont, ImVec2(speedX, speedY), toU32(ImVec4(1,1,1,0.98f)), valueText);
            // place unit anchored to the bottom-right corner of the box with padding
            float unitX = boxCenter.x + boxW*0.5f - padding - unitSize.x;
            float unitY = boxCenter.y + boxH*0.5f - padding - unitSize.y;
            dl->AddText(font, unitFont, ImVec2(unitX, unitY), toU32(ImVec4(0.7f,0.7f,0.7f,0.95f)), unit);
            // small descriptive label below gauge center (e.g., "km/h") optional — avoid duplication
        } else {
            // For RPM gauge the gear overlay sits in the center — avoid drawing overlapping text
            if (!isRPM) {
                float valueFont = std::max(14.0f, radius * 0.28f);
                ImVec2 txtSize = font->CalcTextSizeA(valueFont, FLT_MAX, 0.0f, valueText);
                dl->AddText(ImGui::GetFont(), valueFont, ImVec2(center.x - txtSize.x*0.5f, center.y + radius*0.18f - txtSize.y*0.5f), toU32(ImVec4(1,1,1,0.98f)), valueText);
            }
            // small label (skip for RPM to avoid overlap with gear)
            if (!isRPM) {
                float labelFont = std::max(10.0f, radius * 0.12f);
                ImVec2 lblSize = font->CalcTextSizeA(labelFont, FLT_MAX, 0.0f, label);
                dl->AddText(ImGui::GetFont(), labelFont, ImVec2(center.x - lblSize.x*0.5f, center.y + radius*0.33f - lblSize.y*0.5f), toU32(ImVec4(0.8f,0.8f,0.8f,0.9f)), label);
            }
        }
     };

    // Pixel positions
    const int w = size.width();
    const int h = size.height();

    // Smooth needles (interpolate towards target ratios)
    displayed_speed_ratio_ += (speed_ratio - displayed_speed_ratio_) * smoothing_alpha_;
    displayed_rpm_ratio_ += (rpm_ratio - displayed_rpm_ratio_) * smoothing_alpha_;

    // Adaptive gauge sizing based on the window's minimum dimension so size stays consistent
    // across different aspect ratios and small windows.
    const float minDim = std::min((float)w, (float)h);
    // radius is a fraction of the minimum window dimension; keep it clamped to a relative range
    const float gaugeRadius = std::clamp(minDim * 0.12f, minDim * 0.06f, minDim * 0.18f);
    const float gap = gaugeRadius * 0.32f;

    // Instrument cluster background (rounded rect placed at lower-right)
    // compute half-width of the cluster so we can anchor it with a margin from the right edge
    const float clusterHalfW = (gaugeRadius * 1.9f + gap);
    // margins scaled with window size so small windows keep relative spacing
    const float rightMargin = std::max(6.0f, minDim * 0.02f);
    const float bottomMargin = std::max(6.0f, minDim * 0.03f);
    ImVec2 clusterCenter = ImVec2((float)w - clusterHalfW - rightMargin, (float)h - (gaugeRadius + bottomMargin));
    ImVec2 clusterTL = ImVec2(clusterCenter.x - clusterHalfW, clusterCenter.y - (gaugeRadius * 0.9f));
    ImVec2 clusterBR = ImVec2(clusterCenter.x + clusterHalfW, clusterCenter.y + (gaugeRadius * 0.45f));
    const float clusterRounding = std::max(6.0f, gaugeRadius * 0.1f);
    const float clusterBorder = std::max(1.0f, gaugeRadius * 0.02f);
    dl->AddRectFilled(clusterTL, clusterBR, toU32(ImVec4(0.02f,0.02f,0.02f,0.55f)), clusterRounding);
    dl->AddRect(clusterTL, clusterBR, toU32(ImVec4(1.0f,1.0f,1.0f,0.03f)), clusterRounding, 0, clusterBorder);

    // Positions for left (RPM) and right (speed) gauges within cluster
    ImVec2 leftCenter = ImVec2(clusterCenter.x - (gaugeRadius + gap*0.45f), clusterCenter.y);
    ImVec2 rightCenter = ImVec2(clusterCenter.x + (gaugeRadius + gap*0.45f), clusterCenter.y);

    // Draw RPM (left) and Speed (right) using smoothed ratio
    {
        // Speed (right)
        char speedbuf[32];
        snprintf(speedbuf, sizeof(speedbuf), "%d", static_cast<int>(std::round(speed_kmh)));
        drawGauge(rightCenter, gaugeRadius, displayed_speed_ratio_, "km/h", speedbuf, toU32(ImVec4(0.2f,0.9f,0.2f,1.0f)), MAX_DISPLAY_SPEED_KMH);

        // Rev counter / RPM (left) - same style as speedometer
        char rpmbuf[32];
        // Display RPM as a whole number (e.g., 3500)
        snprintf(rpmbuf, sizeof(rpmbuf), "%d", static_cast<int>(std::round(rpm)));
        // Orange/red color for RPM gauge
        drawGauge(leftCenter, gaugeRadius, displayed_rpm_ratio_, "RPM", rpmbuf, toU32(ImVec4(1.0f,0.6f,0.0f,1.0f)), MAX_RPM);
    }

    // Gear display: moved inside the RPM (left) gauge for a typical car HUD
    {
        const float gearRadius = gaugeRadius * 0.42f;
        ImVec2 center = ImVec2(leftCenter.x, leftCenter.y + gaugeRadius * 0.06f);
        dl->AddCircleFilled(center, gearRadius, toU32(ImVec4(0.04f,0.04f,0.04f,0.95f)));
        dl->AddCircle(center, gearRadius, toU32(ImVec4(0.95f,0.95f,0.95f,0.06f)), 32, std::max(1.5f, gaugeRadius*0.04f));
        std::string gearStr = (gear==0)?"R":(gear>0?std::to_string(gear):std::string("-"));
        ImFont* font = ImGui::GetFont();
        float gearFont = std::max(18.0f, gaugeRadius * 0.5f);
        ImVec2 txtSize = font->CalcTextSizeA(gearFont, FLT_MAX, 0.0f, gearStr.c_str());
        dl->AddText(ImGui::GetFont(), gearFont, ImVec2(center.x - txtSize.x*0.5f, center.y - txtSize.y*0.5f), toU32(ImVec4(0.98f,0.98f,0.98f,1.0f)), gearStr.c_str());
    }

    // Nitrous indicator: small circular radial above the gear
    {
        const float nitroRadius = gaugeRadius * 0.22f;
        // Align nitrous indicator above the speed digital box (right gauge)
        // Compute the same boxCenter used in the speed gauge: rightCenter + offset
        // Use the same offset as the speed box (moved left): rightCenter - gaugeRadius * 0.36
        // Add the same vertical shift used for the speed box so nitro moves down together
        ImVec2 speedBoxCenter = ImVec2(rightCenter.x - gaugeRadius * 0.36f, rightCenter.y + gaugeRadius * 0.02f + gaugeRadius * 0.24f);
         // Place nitrous slightly above the box with a small padding. Recompute box height from gaugeRadius.
         float boxH = gaugeRadius * 0.38f;
         ImVec2 center = ImVec2(speedBoxCenter.x, speedBoxCenter.y - (boxH * 0.5f) - nitroRadius - 6.0f);
         dl->AddCircleFilled(center, nitroRadius, toU32(ImVec4(0.06f,0.06f,0.08f,0.75f)));
        dl->AddCircle(center, nitroRadius, toU32(ImVec4(0.9f,0.9f,0.9f,0.06f)), 32, 1.5f);
        if (vehicle.hasNitrous()){
            dl->AddText(ImGui::GetFont(), 14.0f, ImVec2(center.x - 22.0f, center.y - 6.0f), toU32(ImVec4(0.4f,0.8f,1.0f,1.0f)), "NITRO");
        } else if (vehicle.isNitrousActive()){
            float remaining = vehicle.getNitrousTimeRemaining();
            float ratio = std::clamp(remaining / 5.0f, 0.0f, 1.0f);
            const float a1n = -90.0f * (PI/180.0f);
            const float a2deg = -90.0f + 360.0f * ratio;
            const float a2n = a2deg * (PI/180.0f);
            dl->PathClear();
            dl->PathArcTo(center, nitroRadius-6.0f, a1n, a2n, 64);
            dl->PathStroke(toU32(ImVec4(0.2f,0.6f,1.0f,1.0f)), false, 5.0f);
            dl->AddText(ImGui::GetFont(), 12.0f, ImVec2(center.x - 18.0f, center.y - 6.0f), toU32(ImVec4(1,1,1,1)), "BOOST");
        } else {
            dl->AddText(ImGui::GetFont(), 12.0f, ImVec2(center.x - 15.0f, center.y - 6.0f), toU32(ImVec4(0.6f,0.6f,0.6f,1.0f)), "---");
        }
     }

    // Small developer/test window (toggleable)
    if (show_test_window_) {
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
        ImGui::Begin("Vehicle customization", &show_test_window_);

        // New developer controls: car scale and spawn/scrap buttons
        ImGui::Separator();
        ImGui::SliderFloat("Car scale", &car_scale_, 0.5f, 3.0f, "%.2f");
        // Acceleration multiplier slider: controls how aggressively the vehicle accelerates
        ImGui::SliderFloat("Acceleration", &accel_multiplier_, 0.1f, 3.0f, "%.2f");
        ImGui::TextDisabled("Changes to acceleration are staged and applied only when you click 'Spawn/Respawn Car'.");
        if (ImGui::Button("Scrap mesh")) {
            scrap_mesh_requested_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn/Respawn Car")) {
            respawn_requested_ = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload model")) {
            reload_model_requested_ = true;
        }

        ImGui::End();
    }
}

// Consume a respawn request if set by the UI
bool ImGuiLayer::consumeRespawnRequest(float& out_scale) noexcept {
    if (!respawn_requested_) return false;
    out_scale = car_scale_;
    respawn_requested_ = false;
    return true;
}

bool ImGuiLayer::consumeScrapMeshRequest() noexcept {
    if (!scrap_mesh_requested_) return false;
    scrap_mesh_requested_ = false;
    return true;
}

bool ImGuiLayer::consumeReloadModelRequest() noexcept {
    if (!reload_model_requested_) return false;
    reload_model_requested_ = false;
    return true;
}
