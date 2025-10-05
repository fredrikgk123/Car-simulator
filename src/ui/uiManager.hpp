#pragma once

#include <threepp/threepp.hpp>
#include "vehicle.hpp"
#include <memory>

class UIManager {
  public:
    UIManager(threepp::GLRenderer& renderer);

    // Render the UI overlay
    void render(const Vehicle& vehicle, const threepp::WindowSize& size);

  private:
    void createSpeedometerGeometry();
    void updateSpeedometer(float speed);
    void createSevenSegmentDigit(float xPos, float yPos, int digitIndex);

    threepp::GLRenderer& renderer_;

    // HUD scene and camera for 2D overlay
    std::shared_ptr<threepp::Scene> hudScene_;
    std::shared_ptr<threepp::OrthographicCamera> hudCamera_;

    // Speedometer visual elements
    std::shared_ptr<threepp::Mesh> speedometerBackground_;
    std::shared_ptr<threepp::Mesh> speedometerFill_;  // Filled portion
    std::shared_ptr<threepp::Mesh> speedometerOutline_;  // Empty portion

    // Seven-segment display - each digit has 7 segments
    std::vector<std::vector<std::shared_ptr<threepp::Mesh>>> digitSegments_;  // [digit][segment]
};
