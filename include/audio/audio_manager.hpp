// AI Assistance: GitHub Copilot was used for miniaudio API integration
// and custom deleters for RAII resource management.

#pragma once

#include <string>
#include <string_view>
#include <memory>

// Forward declarations to avoid including miniaudio header
struct ma_engine;
struct ma_sound;

class IVehicleState;

/**
 * Handles audio playback for engine and drift sounds.
 * Uses miniaudio with RAII wrappers for automatic cleanup.
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Can't copy (audio resources are unique)
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    [[nodiscard]] bool initialize(std::string_view engineSoundPath);

    // Updates audio based on RPM, speed, drift state, etc.
    void update(const IVehicleState& vehicleState);

private:
    [[nodiscard]] static float calculateEnginePitch(float velocity, float maxSpeed) noexcept;

    struct AudioDeleter {
        void operator()(ma_engine* engine) const noexcept;
        void operator()(ma_sound* sound) const noexcept;
    };

    std::unique_ptr<ma_engine, AudioDeleter> engine_;
    std::unique_ptr<ma_sound, AudioDeleter> engineSound_;
    std::unique_ptr<ma_sound, AudioDeleter> driftSound_;
    bool initialized_;
    bool soundLoaded_;
    bool driftSoundLoaded_;
};
