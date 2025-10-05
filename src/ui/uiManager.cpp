#include "uiManager.hpp"
#include <cmath>
#include <array>

using namespace threepp;

// Seven-segment display mapping for digits 0-9
// Segments: top, top-right, bottom-right, bottom, bottom-left, top-left, middle
static const std::array<std::array<bool, 7>, 10> SEGMENT_PATTERNS = {{
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

UIManager::UIManager(GLRenderer& renderer)
    : renderer_(renderer) {

    // Create HUD scene and orthographic camera for 2D overlay
    hudScene_ = std::make_shared<Scene>();

    // Orthographic camera covering screen space (-1 to 1)
    hudCamera_ = std::make_shared<OrthographicCamera>(-1, 1, 1, -1, 0.1f, 10.0f);
    hudCamera_->position.z = 1;

    createSpeedometerGeometry();
}

void UIManager::createSpeedometerGeometry() {
    // Create speedometer background panel
    auto bgGeometry = PlaneGeometry::create(0.4f, 0.25f);
    auto bgMaterial = MeshBasicMaterial::create();
    bgMaterial->color = Color(0x000000);
    bgMaterial->transparent = true;
    bgMaterial->opacity = 0.7f;
    bgMaterial->depthTest = false;

    speedometerBackground_ = Mesh::create(bgGeometry, bgMaterial);
    speedometerBackground_->position.set(0.6f, -0.725f, 0);
    hudScene_->add(speedometerBackground_);

    // Create speedometer bar outline (full bar)
    auto outlineGeometry = PlaneGeometry::create(0.32f, 0.04f);
    auto outlineMaterial = MeshBasicMaterial::create();
    outlineMaterial->color = Color(0x333333);  // Dark gray outline
    outlineMaterial->transparent = true;
    outlineMaterial->opacity = 0.8f;
    outlineMaterial->depthTest = false;

    speedometerOutline_ = Mesh::create(outlineGeometry, outlineMaterial);
    speedometerOutline_->position.set(0.6f, -0.68f, 0.01f);
    hudScene_->add(speedometerOutline_);

    // Create speedometer fill bar (starts at 0 width)
    auto fillGeometry = PlaneGeometry::create(0.32f, 0.04f);
    auto fillMaterial = MeshBasicMaterial::create();
    fillMaterial->color = Color(0x00ff00);  // Green fill
    fillMaterial->transparent = true;
    fillMaterial->opacity = 0.9f;
    fillMaterial->depthTest = false;

    speedometerFill_ = Mesh::create(fillGeometry, fillMaterial);
    speedometerFill_->position.set(0.6f, -0.68f, 0.02f);
    speedometerFill_->scale.x = 0.0f;  // Start with no fill
    hudScene_->add(speedometerFill_);

    // Create three seven-segment digits for speed display
    digitSegments_.resize(3);  // Three digits (hundreds, tens, ones)

    float digitStartX = 0.52f;
    float digitSpacing = 0.055f;
    float digitY = -0.775f;

    for (int i = 0; i < 3; i++) {
        createSevenSegmentDigit(digitStartX + i * digitSpacing, digitY, i);
    }

    // Add "km/h" label
    auto labelGeometry = PlaneGeometry::create(0.08f, 0.02f);
    auto labelMaterial = MeshBasicMaterial::create();
    labelMaterial->color = Color(0xaaaaaa);
    labelMaterial->transparent = true;
    labelMaterial->opacity = 0.7f;
    labelMaterial->depthTest = false;

    auto label = Mesh::create(labelGeometry, labelMaterial);
    label->position.set(0.7f, -0.775f, 0.01f);
    hudScene_->add(label);
}

void UIManager::createSevenSegmentDigit(float xPos, float yPos, int digitIndex) {
    // Create 7 segments for a single digit
    // Segment layout:
    //     0
    //   5   1
    //     6
    //   4   2
    //     3

    auto segmentMaterial = MeshBasicMaterial::create();
    segmentMaterial->color = Color(0x00ff00);  // Green segments
    segmentMaterial->transparent = true;
    segmentMaterial->opacity = 0.9f;
    segmentMaterial->depthTest = false;

    std::vector<std::shared_ptr<Mesh>> segments;

    float segmentWidth = 0.03f;
    float segmentHeight = 0.006f;
    float segmentLength = 0.025f;

    // Horizontal segments (top, middle, bottom)
    auto hSegmentGeometry = PlaneGeometry::create(segmentLength, segmentHeight);

    // Vertical segments (sides)
    auto vSegmentGeometry = PlaneGeometry::create(segmentHeight, segmentLength);

    // Segment 0: Top horizontal
    auto seg0 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg0->position.set(xPos, yPos + 0.028f, 0.03f);
    hudScene_->add(seg0);
    segments.push_back(seg0);

    // Segment 1: Top right vertical
    auto seg1 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg1->position.set(xPos + 0.0135f, yPos + 0.014f, 0.03f);
    hudScene_->add(seg1);
    segments.push_back(seg1);

    // Segment 2: Bottom right vertical
    auto seg2 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg2->position.set(xPos + 0.0135f, yPos - 0.014f, 0.03f);
    hudScene_->add(seg2);
    segments.push_back(seg2);

    // Segment 3: Bottom horizontal
    auto seg3 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg3->position.set(xPos, yPos - 0.028f, 0.03f);
    hudScene_->add(seg3);
    segments.push_back(seg3);

    // Segment 4: Bottom left vertical
    auto seg4 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg4->position.set(xPos - 0.0135f, yPos - 0.014f, 0.03f);
    hudScene_->add(seg4);
    segments.push_back(seg4);

    // Segment 5: Top left vertical
    auto seg5 = Mesh::create(vSegmentGeometry, segmentMaterial);
    seg5->position.set(xPos - 0.0135f, yPos + 0.014f, 0.03f);
    hudScene_->add(seg5);
    segments.push_back(seg5);

    // Segment 6: Middle horizontal
    auto seg6 = Mesh::create(hSegmentGeometry, segmentMaterial);
    seg6->position.set(xPos, yPos, 0.03f);
    hudScene_->add(seg6);
    segments.push_back(seg6);

    digitSegments_[digitIndex] = segments;

    // Initially hide all segments
    for (auto& seg : segments) {
        seg->visible = false;
    }
}

void UIManager::updateSpeedometer(float speed) {
    // Calculate speed ratio (0.0 to 1.0)
    float maxSpeed = 250.0f;  // Max display speed in km/h
    float displaySpeed = std::abs(speed) * 10.0f;  // Convert to km/h
    float speedRatio = std::min(displaySpeed / maxSpeed, 1.0f);

    // Update bar fill
    speedometerFill_->scale.x = speedRatio;

    // Adjust position so it grows from left to right
    float barWidth = 0.32f;
    speedometerFill_->position.x = 0.6f - (barWidth / 2.0f) * (1.0f - speedRatio);

    // Update numeric display
    int speedInt = static_cast<int>(displaySpeed);
    int hundreds = (speedInt / 100) % 10;
    int tens = (speedInt / 10) % 10;
    int ones = speedInt % 10;

    std::array<int, 3> digits = {hundreds, tens, ones};

    // Update each digit's segments
    for (int d = 0; d < 3; d++) {
        int digit = digits[d];

        // Hide leading zeros except for the ones place
        bool hideDigit = (d == 0 && speedInt < 100) || (d == 1 && speedInt < 10);

        if (hideDigit) {
            // Hide all segments
            for (int s = 0; s < 7; s++) {
                digitSegments_[d][s]->visible = false;
            }
        } else {
            // Show/hide segments based on digit pattern
            for (int s = 0; s < 7; s++) {
                digitSegments_[d][s]->visible = SEGMENT_PATTERNS[digit][s];
            }
        }
    }
}

void UIManager::render(const Vehicle& vehicle, const WindowSize& size) {
    float speed = vehicle.getVelocity();
    updateSpeedometer(speed);

    // Render HUD overlay on top of main scene
    renderer_.autoClear = false;
    renderer_.render(*hudScene_, *hudCamera_);
    renderer_.autoClear = true;
}
