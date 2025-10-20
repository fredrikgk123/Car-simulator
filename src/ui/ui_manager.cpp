#include "ui_manager.hpp"
#include "../core/vehicle.hpp"
#include <cmath>
#include <array>

using namespace threepp;

// Seven-segment display implementation inspired by standard digital display logic
// Pattern encoding based on common 7-segment layout conventions
// Reference: https://en.wikipedia.org/wiki/Seven-segment_display
//
// Segment layout:
//     [0]
//   [5] [1]
//     [6]
//   [4] [2]
//     [3]

// Anonymous namespace - these constants are LOCAL to this file only (NOT global!)
namespace {
    // Seven-segment display patterns for digits 0-9
    // Segments: top, top-right, bottom-right, bottom, bottom-left, top-left, middle
    const std::array<std::array<bool, 7>, 10> SEGMENT_PATTERNS = {{
        {true, true, true, true, true, true, false},      // 0
        {false, true, true, false, false, false, false},  // 1
        {true, true, false, true, true, false, true},     // 2
        {true, true, true, true, false, false, true},     // 3
        {false, true, true, false, false, true, true},    // 4
        {true, false, true, true, false, true, true},     // 5
        {true, false, true, true, true, true, true},      // 6
        {true, true, true, false, false, false, false},   // 7
        {true, true, true, true, true, true, true},       // 8
        {true, true, true, true, false, true, true}       // 9
    }};

    // Speedometer dimensions and positioning
    constexpr float SPEEDOMETER_BG_WIDTH = 0.4f;
    constexpr float SPEEDOMETER_BG_HEIGHT = 0.25f;
    constexpr float SPEEDOMETER_BAR_WIDTH = 0.32f;
    constexpr float SPEEDOMETER_BAR_HEIGHT = 0.04f;
    constexpr float SPEEDOMETER_X_POS = 0.6f;          // Right side of screen
    constexpr float SPEEDOMETER_BG_Y_POS = -0.725f;
    constexpr float SPEEDOMETER_BAR_Y_POS = -0.68f;
    constexpr float SPEEDOMETER_DIGIT_Y_POS = -0.775f;
    constexpr float SPEEDOMETER_DIGIT_START_X = 0.52f;
    constexpr float SPEEDOMETER_DIGIT_SPACING = 0.055f;

    // Seven-segment display dimensions
    constexpr float SEGMENT_HEIGHT = 0.006f;
    constexpr float SEGMENT_LENGTH = 0.025f;
    constexpr float SEGMENT_VERTICAL_OFFSET = 0.028f;
    constexpr float SEGMENT_HORIZONTAL_OFFSET = 0.0135f;
    constexpr float SEGMENT_HALF_OFFSET = 0.014f;
    constexpr int NUM_DIGITS = 3;
    constexpr int NUM_SEGMENTS_PER_DIGIT = 7;

    // Nitrous indicator positioning
    constexpr float NITROUS_INDICATOR_SIZE = 0.08f;
    constexpr float NITROUS_BAR_WIDTH = 0.32f;
    constexpr float NITROUS_BAR_HEIGHT = 0.03f;
    constexpr float NITROUS_X_POS = 0.6f;
    constexpr float NITROUS_Y_POS = -0.85f;

    // Gear display positioning and dimensions
    constexpr float GEAR_BG_WIDTH = 0.15f;
    constexpr float GEAR_BG_HEIGHT = 0.15f;
    constexpr float GEAR_X_POS = -0.7f;           // Left side of screen
    constexpr float GEAR_Y_POS = -0.75f;
    constexpr float GEAR_DIGIT_SCALE = 2.0f;      // Make gear digit larger than speedometer digits

    // RPM display positioning and dimensions
    constexpr float RPM_BG_WIDTH = 0.25f; // Narrower background for left side
    constexpr float RPM_BG_HEIGHT = 0.15f;
    constexpr float RPM_X_POS = -0.45f;   // Just right of gear window
    constexpr float RPM_Y_POS = -0.75f;   // Align vertically with gear window
    constexpr float RPM_DIGIT_START_X = -0.55f; // Start digits just right of gear window
    constexpr float RPM_DIGIT_SPACING = 0.045f; // Slightly reduced spacing for compactness
    constexpr int RPM_NUM_DIGITS = 4;

    // Material properties
    constexpr float BACKGROUND_OPACITY = 0.7f;
    constexpr float OUTLINE_OPACITY = 0.8f;
    constexpr float FILL_OPACITY = 0.9f;
    constexpr float SEGMENT_OPACITY = 0.9f;

    // Colors
    constexpr unsigned int BLACK_COLOR = 0x000000;
    constexpr unsigned int DARK_GRAY_COLOR = 0x333333;
    constexpr unsigned int GREEN_COLOR = 0x00ff00;
    constexpr unsigned int NITROUS_BLUE_COLOR = 0x00aaff;

    // Speed constants
    constexpr float MAX_DISPLAY_SPEED_KMH = 150.0f;  // 150 km/h - matches vehicle's real max speed
    constexpr float MS_TO_KMH = 3.6f;                // Conversion: 1 m/s = 3.6 km/h
    constexpr float NITROUS_DURATION = 5.0f;         // 5 seconds - must match vehicle.cpp

    // Z-depth layers
    constexpr float Z_BACKGROUND = 0.0f;
    constexpr float Z_OUTLINE = 0.01f;
    constexpr float Z_FILL = 0.02f;
    constexpr float Z_SEGMENTS = 0.03f;
}

UIManager::UIManager(GLRenderer& renderer)
    : renderer_(renderer) {
    hudScene_ = std::make_shared<Scene>();

    // Orthographic camera for 2D overlay
    hudCamera_ = std::make_shared<OrthographicCamera>(-1, 1, 1, -1, 0.1f, 10.0f);
    hudCamera_->position.z = 1;

    createSpeedometerGeometry();

    // Create nitrous indicator
    auto nitrousGeometry = PlaneGeometry::create(NITROUS_INDICATOR_SIZE, NITROUS_INDICATOR_SIZE);
    auto nitrousMaterial = MeshBasicMaterial::create();
    nitrousMaterial->color = Color(NITROUS_BLUE_COLOR);
    nitrousMaterial->transparent = true;
    nitrousMaterial->opacity = FILL_OPACITY;
    nitrousMaterial->depthTest = false;

    nitrousIndicator_ = Mesh::create(nitrousGeometry, nitrousMaterial);
    nitrousIndicator_->position.set(NITROUS_X_POS, NITROUS_Y_POS, Z_SEGMENTS);
    nitrousIndicator_->visible = false;
    hudScene_->add(nitrousIndicator_);

    // Create nitrous bar outline
    auto nitrousOutlineGeometry = PlaneGeometry::create(NITROUS_BAR_WIDTH, NITROUS_BAR_HEIGHT);
    auto nitrousOutlineMaterial = MeshBasicMaterial::create();
    nitrousOutlineMaterial->color = Color(DARK_GRAY_COLOR);
    nitrousOutlineMaterial->transparent = true;
    nitrousOutlineMaterial->opacity = OUTLINE_OPACITY;
    nitrousOutlineMaterial->depthTest = false;

    nitrousBarOutline_ = Mesh::create(nitrousOutlineGeometry, nitrousOutlineMaterial);
    nitrousBarOutline_->position.set(NITROUS_X_POS, NITROUS_Y_POS, Z_OUTLINE);
    nitrousBarOutline_->visible = false;
    hudScene_->add(nitrousBarOutline_);

    // Create nitrous bar fill
    auto nitrousBarGeometry = PlaneGeometry::create(NITROUS_BAR_WIDTH, NITROUS_BAR_HEIGHT);
    auto nitrousBarMaterial = MeshBasicMaterial::create();
    nitrousBarMaterial->color = Color(NITROUS_BLUE_COLOR);
    nitrousBarMaterial->transparent = true;
    nitrousBarMaterial->opacity = FILL_OPACITY;
    nitrousBarMaterial->depthTest = false;

    nitrousBar_ = Mesh::create(nitrousBarGeometry, nitrousBarMaterial);
    nitrousBar_->position.set(NITROUS_X_POS, NITROUS_Y_POS, Z_FILL);
    nitrousBar_->visible = false;
    hudScene_->add(nitrousBar_);

    // Create gear display
    createGearDisplay();

    // Create RPM display
    createRPMDisplay();

    // Note: ImGui context and backend initialization is performed by the application (main.cpp).
    // UIManager will only build ImGui widgets when a context is available.
}

void UIManager::createSpeedometerGeometry() {
    // Create background panel
    auto bgGeometry = PlaneGeometry::create(SPEEDOMETER_BG_WIDTH, SPEEDOMETER_BG_HEIGHT);
    auto bgMaterial = MeshBasicMaterial::create();
    bgMaterial->color = Color(BLACK_COLOR);
    bgMaterial->transparent = true;
    bgMaterial->opacity = BACKGROUND_OPACITY;
    bgMaterial->depthTest = false;

    speedometerBackground_ = Mesh::create(bgGeometry, bgMaterial);
    speedometerBackground_->position.set(SPEEDOMETER_X_POS, SPEEDOMETER_BG_Y_POS, Z_BACKGROUND);
    hudScene_->add(speedometerBackground_);

    // Create bar outline
    auto outlineGeometry = PlaneGeometry::create(SPEEDOMETER_BAR_WIDTH, SPEEDOMETER_BAR_HEIGHT);
    auto outlineMaterial = MeshBasicMaterial::create();
    outlineMaterial->color = Color(DARK_GRAY_COLOR);
    outlineMaterial->transparent = true;
    outlineMaterial->opacity = OUTLINE_OPACITY;
    outlineMaterial->depthTest = false;

    speedometerOutline_ = Mesh::create(outlineGeometry, outlineMaterial);
    speedometerOutline_->position.set(SPEEDOMETER_X_POS, SPEEDOMETER_BAR_Y_POS, Z_OUTLINE);
    hudScene_->add(speedometerOutline_);

    // Create fill bar
    auto fillGeometry = PlaneGeometry::create(SPEEDOMETER_BAR_WIDTH, SPEEDOMETER_BAR_HEIGHT);
    auto fillMaterial = MeshBasicMaterial::create();
    fillMaterial->color = Color(GREEN_COLOR);
    fillMaterial->transparent = true;
    fillMaterial->opacity = FILL_OPACITY;
    fillMaterial->depthTest = false;

    speedometerFill_ = Mesh::create(fillGeometry, fillMaterial);
    speedometerFill_->position.set(SPEEDOMETER_X_POS, SPEEDOMETER_BAR_Y_POS, Z_FILL);
    speedometerFill_->scale.x = 0.0f;  // Start empty (0 speed)
    hudScene_->add(speedometerFill_);

    // Create three digits for speed display
    digitSegments_.resize(NUM_DIGITS);

    for (int i = 0; i < NUM_DIGITS; ++i) {
        float xPosition = SPEEDOMETER_DIGIT_START_X + (i * SPEEDOMETER_DIGIT_SPACING);
        createSevenSegmentDigit(xPosition, SPEEDOMETER_DIGIT_Y_POS, i);
    }
}

void UIManager::createSevenSegmentDigit(float xPos, float yPos, int digitIndex) {
    auto segmentMaterial = MeshBasicMaterial::create();
    segmentMaterial->color = Color(GREEN_COLOR);
    segmentMaterial->transparent = true;
    segmentMaterial->opacity = SEGMENT_OPACITY;
    segmentMaterial->depthTest = false;

    std::vector<std::shared_ptr<Mesh>> segments;

    auto hSegmentGeometry = PlaneGeometry::create(SEGMENT_LENGTH, SEGMENT_HEIGHT);  // Horizontal segments
    auto vSegmentGeometry = PlaneGeometry::create(SEGMENT_HEIGHT, SEGMENT_LENGTH);  // Vertical segments

    // Create 7 segments for this digit
    // Segment 0: Top horizontal
    auto seg0 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg0->position.set(xPos, yPos + SEGMENT_VERTICAL_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg0);
    segments.push_back(seg0);

    // Segment 1: Top right vertical
    auto seg1 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg1->position.set(xPos + SEGMENT_HORIZONTAL_OFFSET, yPos + SEGMENT_HALF_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg1);
    segments.push_back(seg1);

    // Segment 2: Bottom right vertical
    auto seg2 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg2->position.set(xPos + SEGMENT_HORIZONTAL_OFFSET, yPos - SEGMENT_HALF_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg2);
    segments.push_back(seg2);

    // Segment 3: Bottom horizontal
    auto seg3 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg3->position.set(xPos, yPos - SEGMENT_VERTICAL_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg3);
    segments.push_back(seg3);

    // Segment 4: Bottom left vertical
    auto seg4 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg4->position.set(xPos - SEGMENT_HORIZONTAL_OFFSET, yPos - SEGMENT_HALF_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg4);
    segments.push_back(seg4);

    // Segment 5: Top left vertical
    auto seg5 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg5->position.set(xPos - SEGMENT_HORIZONTAL_OFFSET, yPos + SEGMENT_HALF_OFFSET, Z_SEGMENTS);
    hudScene_->add(seg5);
    segments.push_back(seg5);

    // Segment 6: Middle horizontal
    auto seg6 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg6->position.set(xPos, yPos, Z_SEGMENTS);
    hudScene_->add(seg6);
    segments.push_back(seg6);

    digitSegments_[digitIndex] = segments;

    // Initially hide all segments
    for (auto& segment : segments) {
        segment->visible = false;
    }
}

void UIManager::updateSpeedometer(float speed) {
    // Calculate speed display
    float displaySpeed = std::abs(speed) * MS_TO_KMH;
    float speedRatio = std::clamp(displaySpeed / MAX_DISPLAY_SPEED_KMH, 0.0f, 1.0f);

    // Update fill bar scale
    speedometerFill_->scale.x = speedRatio;

    // Position bar to grow from left to right
    speedometerFill_->position.x = SPEEDOMETER_X_POS - ((SPEEDOMETER_BAR_WIDTH / 2.0f) * (1.0f - speedRatio));

    // Update seven-segment display
    int speedInt = static_cast<int>(displaySpeed);
    std::array<int, 3> digits = {
        (speedInt / 100) % 10,  // hundreds
        (speedInt / 10) % 10,   // tens
        speedInt % 10           // ones
    };

    for (int d = 0; d < NUM_DIGITS; ++d) {
        int digit = digits[static_cast<size_t>(d)];

        // Hide leading zeros
        bool hideDigit = (d == 0 && speedInt < 100) || (d == 1 && speedInt < 10);

        if (hideDigit) {
            for (int s = 0; s < NUM_SEGMENTS_PER_DIGIT; ++s) {
                digitSegments_[static_cast<size_t>(d)][static_cast<size_t>(s)]->visible = false;
            }
        } else {
            // Show segments based on digit pattern
            for (int s = 0; s < NUM_SEGMENTS_PER_DIGIT; ++s) {
                digitSegments_[static_cast<size_t>(d)][static_cast<size_t>(s)]->visible = SEGMENT_PATTERNS[static_cast<size_t>(digit)][static_cast<size_t>(s)];
            }
        }
    }
}

void UIManager::render(const Vehicle& vehicle, const WindowSize& size) {
    float speed = vehicle.getVelocity();
    updateSpeedometer(speed);

    // Update gear display
    int currentGear = vehicle.getCurrentGear();
    updateGearDisplay(currentGear);

    // Update RPM display
    updateRPMDisplay(vehicle.getRPM());

    // Update nitrous indicator
    if (vehicle.hasNitrous()) {
        nitrousIndicator_->visible = true;
        nitrousBarOutline_->visible = false;
        nitrousBar_->visible = false;
    } else if (vehicle.isNitrousActive()) {
        nitrousIndicator_->visible = false;
        nitrousBarOutline_->visible = true;
        nitrousBar_->visible = true;

        // Update nitrous bar based on time remaining
        float timeRemaining = vehicle.getNitrousTimeRemaining();
        float nitrousRatio = timeRemaining / NITROUS_DURATION;

        nitrousBar_->scale.x = nitrousRatio;
        nitrousBar_->position.x = NITROUS_X_POS - ((NITROUS_BAR_WIDTH / 2.0f) * (1.0f - nitrousRatio));
    } else {
        nitrousIndicator_->visible = false;
        nitrousBarOutline_->visible = false;
        nitrousBar_->visible = false;
    }

    // Render HUD overlay on top of main scene
    renderer_.autoClear = false;
    renderer_.render(*hudScene_, *hudCamera_);
    renderer_.autoClear = true;
}

void UIManager::createGearDisplay() {
    // Create background for gear display
    auto gearBgGeometry = PlaneGeometry::create(GEAR_BG_WIDTH, GEAR_BG_HEIGHT);
    auto gearBgMaterial = MeshBasicMaterial::create();
    gearBgMaterial->color = Color(BLACK_COLOR);
    gearBgMaterial->transparent = true;
    gearBgMaterial->opacity = BACKGROUND_OPACITY;
    gearBgMaterial->depthTest = false;

    gearBackground_ = Mesh::create(gearBgGeometry, gearBgMaterial);
    gearBackground_->position.set(GEAR_X_POS, GEAR_Y_POS, Z_BACKGROUND);
    hudScene_->add(gearBackground_);

    // Create 7-segment display for gear (larger than speed digits)
    auto segmentMaterial = MeshBasicMaterial::create();
    segmentMaterial->color = Color(GREEN_COLOR);
    segmentMaterial->transparent = true;
    segmentMaterial->opacity = SEGMENT_OPACITY;
    segmentMaterial->depthTest = false;

    // Scale up the segment dimensions for the gear display
    float gearSegmentLength = SEGMENT_LENGTH * GEAR_DIGIT_SCALE;
    float gearSegmentHeight = SEGMENT_HEIGHT * GEAR_DIGIT_SCALE;
    float gearVerticalOffset = SEGMENT_VERTICAL_OFFSET * GEAR_DIGIT_SCALE;
    float gearHorizontalOffset = SEGMENT_HORIZONTAL_OFFSET * GEAR_DIGIT_SCALE;
    float gearHalfOffset = SEGMENT_HALF_OFFSET * GEAR_DIGIT_SCALE;

    auto hSegmentGeometry = PlaneGeometry::create(gearSegmentLength, gearSegmentHeight);
    auto vSegmentGeometry = PlaneGeometry::create(gearSegmentHeight, gearSegmentLength);

    // Create 7 segments for gear digit
    // Segment 0: Top horizontal
    auto seg0 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg0->position.set(GEAR_X_POS, GEAR_Y_POS + gearVerticalOffset, Z_SEGMENTS);
    hudScene_->add(seg0);
    gearDigitSegments_.push_back(seg0);

    // Segment 1: Top right vertical
    auto seg1 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg1->position.set(GEAR_X_POS + gearHorizontalOffset, GEAR_Y_POS + gearHalfOffset, Z_SEGMENTS);
    hudScene_->add(seg1);
    gearDigitSegments_.push_back(seg1);

    // Segment 2: Bottom right vertical
    auto seg2 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg2->position.set(GEAR_X_POS + gearHorizontalOffset, GEAR_Y_POS - gearHalfOffset, Z_SEGMENTS);
    hudScene_->add(seg2);
    gearDigitSegments_.push_back(seg2);

    // Segment 3: Bottom horizontal
    auto seg3 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg3->position.set(GEAR_X_POS, GEAR_Y_POS - gearVerticalOffset, Z_SEGMENTS);
    hudScene_->add(seg3);
    gearDigitSegments_.push_back(seg3);

    // Segment 4: Bottom left vertical
    auto seg4 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg4->position.set(GEAR_X_POS - gearHorizontalOffset, GEAR_Y_POS - gearHalfOffset, Z_SEGMENTS);
    hudScene_->add(seg4);
    gearDigitSegments_.push_back(seg4);

    // Segment 5: Top left vertical
    auto seg5 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg5->position.set(GEAR_X_POS - gearHorizontalOffset, GEAR_Y_POS + gearHalfOffset, Z_SEGMENTS);
    hudScene_->add(seg5);
    gearDigitSegments_.push_back(seg5);

    // Segment 6: Middle horizontal
    auto seg6 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg6->position.set(GEAR_X_POS, GEAR_Y_POS, Z_SEGMENTS);
    hudScene_->add(seg6);
    gearDigitSegments_.push_back(seg6);

    // Initially hide all segments
    for (auto& segment : gearDigitSegments_) {
        segment->visible = false;
    }
}

void UIManager::updateGearDisplay(int gear) {
    // Display gear 0-5 (0 = R for reverse, 1-5 = forward gears)
    // For gear 0 (reverse), we'll display "r" which is a custom pattern

    if (gear == 0) {
        // Display "r" for reverse (segments 4 and 6 only - simplified)
        gearDigitSegments_[0]->visible = false;  // top
        gearDigitSegments_[1]->visible = false;  // top-right
        gearDigitSegments_[2]->visible = false;  // bottom-right
        gearDigitSegments_[3]->visible = false;  // bottom
        gearDigitSegments_[4]->visible = true;   // bottom-left
        gearDigitSegments_[5]->visible = false;  // top-left
        gearDigitSegments_[6]->visible = true;   // middle
    } else if (gear >= 1 && gear <= 5) {
        // Display gear 1-5 using standard 7-segment patterns
        const auto& pattern = SEGMENT_PATTERNS[static_cast<size_t>(gear)];
        for (int s = 0; s < NUM_SEGMENTS_PER_DIGIT; ++s) {
            gearDigitSegments_[static_cast<size_t>(s)]->visible = pattern[static_cast<size_t>(s)];
        }
    } else {
        // Invalid gear - hide all segments
        for (auto& segment : gearDigitSegments_) {
            segment->visible = false;
        }
    }
}

void UIManager::createRPMDisplay() {
    // Create background for RPM display
    auto rpmBgGeometry = PlaneGeometry::create(RPM_BG_WIDTH, RPM_BG_HEIGHT);
    auto rpmBgMaterial = MeshBasicMaterial::create();
    rpmBgMaterial->color = Color(BLACK_COLOR);
    rpmBgMaterial->transparent = true;
    rpmBgMaterial->opacity = BACKGROUND_OPACITY;
    rpmBgMaterial->depthTest = false;

    rpmBackground_ = Mesh::create(rpmBgGeometry, rpmBgMaterial);
    rpmBackground_->position.set(RPM_X_POS, RPM_Y_POS, Z_BACKGROUND);
    hudScene_->add(rpmBackground_);

    // Create 4 digits for RPM display (0000-7000)
    rpmDigitSegments_.resize(RPM_NUM_DIGITS);

    for (int i = 0; i < RPM_NUM_DIGITS; ++i) {
        float xPosition = RPM_DIGIT_START_X + (i * RPM_DIGIT_SPACING);

        // Create segments for this digit
        auto segmentMaterial = MeshBasicMaterial::create();
        segmentMaterial->color = Color(GREEN_COLOR);
        segmentMaterial->transparent = true;
        segmentMaterial->opacity = SEGMENT_OPACITY;
        segmentMaterial->depthTest = false;

        std::vector<std::shared_ptr<Mesh>> segments;
        auto hSegmentGeometry = PlaneGeometry::create(SEGMENT_LENGTH * 0.7f, SEGMENT_HEIGHT * 0.7f);
        auto vSegmentGeometry = PlaneGeometry::create(SEGMENT_HEIGHT * 0.7f, SEGMENT_LENGTH * 0.7f);

        float vOffset = SEGMENT_VERTICAL_OFFSET * 0.7f;
        float hOffset = SEGMENT_HORIZONTAL_OFFSET * 0.7f;
        float halfOffset = SEGMENT_HALF_OFFSET * 0.7f;

        // Create 7 segments
        auto seg0 = Mesh::create(hSegmentGeometry, segmentMaterial);
        seg0->position.set(xPosition, RPM_Y_POS + vOffset, Z_SEGMENTS);
        hudScene_->add(seg0);
        segments.push_back(seg0);

        auto seg1 = Mesh::create(vSegmentGeometry, segmentMaterial);
        seg1->position.set(xPosition + hOffset, RPM_Y_POS + halfOffset, Z_SEGMENTS);
        hudScene_->add(seg1);
        segments.push_back(seg1);

        auto seg2 = Mesh::create(vSegmentGeometry, segmentMaterial);
        seg2->position.set(xPosition + hOffset, RPM_Y_POS - halfOffset, Z_SEGMENTS);
        hudScene_->add(seg2);
        segments.push_back(seg2);

        auto seg3 = Mesh::create(hSegmentGeometry, segmentMaterial);
        seg3->position.set(xPosition, RPM_Y_POS - vOffset, Z_SEGMENTS);
        hudScene_->add(seg3);
        segments.push_back(seg3);

        auto seg4 = Mesh::create(vSegmentGeometry, segmentMaterial);
        seg4->position.set(xPosition - hOffset, RPM_Y_POS - halfOffset, Z_SEGMENTS);
        hudScene_->add(seg4);
        segments.push_back(seg4);

        auto seg5 = Mesh::create(vSegmentGeometry, segmentMaterial);
        seg5->position.set(xPosition - hOffset, RPM_Y_POS + halfOffset, Z_SEGMENTS);
        hudScene_->add(seg5);
        segments.push_back(seg5);

        auto seg6 = Mesh::create(hSegmentGeometry, segmentMaterial);
        seg6->position.set(xPosition, RPM_Y_POS, Z_SEGMENTS);
        hudScene_->add(seg6);
        segments.push_back(seg6);

        rpmDigitSegments_[i] = segments;

        // Initially hide all segments
        for (auto& segment : segments) {
            segment->visible = false;
        }
    }
}

void UIManager::updateRPMDisplay(float rpm) {
    // Display RPM as 4-digit number (e.g., 6000)
    int rpmInt = static_cast<int>(rpm);
    rpmInt = std::clamp(rpmInt, 0, 9999); // Max 9999 RPM for display

    std::array<int, RPM_NUM_DIGITS> digits = {
        (rpmInt / 1000) % 10,  // thousands
        (rpmInt / 100) % 10,   // hundreds
        (rpmInt / 10) % 10,    // tens
        rpmInt % 10            // ones
    };

    for (int d = 0; d < RPM_NUM_DIGITS; ++d) {
        int digit = digits[static_cast<size_t>(d)];

        // Hide leading zeros except for the last digit
        bool hideDigit = (d == 0 && rpmInt < 1000) || (d == 1 && rpmInt < 100) || (d == 2 && rpmInt < 10);

        if (hideDigit) {
            for (int s = 0; s < NUM_SEGMENTS_PER_DIGIT; ++s) {
                rpmDigitSegments_[static_cast<size_t>(d)][static_cast<size_t>(s)]->visible = false;
            }
        } else {
            // Show segments based on digit pattern
            for (int s = 0; s < NUM_SEGMENTS_PER_DIGIT; ++s) {
                rpmDigitSegments_[static_cast<size_t>(d)][static_cast<size_t>(s)]->visible =
                    SEGMENT_PATTERNS[static_cast<size_t>(digit)][static_cast<size_t>(s)];
            }
        }
    }
}
