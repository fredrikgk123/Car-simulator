#pragma once

#include <string>
#include <memory>

// Forward declare miniaudio types
struct ma_engine;
struct ma_sound;

class Vehicle;

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Delete copy constructor and assignment operator (audio resources shouldn't be copied)
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Initialize audio engine and load sound files
    bool initialize(const std::string& engineSoundPath);

    // Update audio based on vehicle state
    void update(const Vehicle& vehicle);

private:
    float calculateEnginePitch(float velocity, float maxSpeed) const;

    struct AudioDeleter {
        void operator()(ma_engine* engine) const;
        void operator()(ma_sound* sound) const;
    };

    std::unique_ptr<ma_engine, AudioDeleter> engine_;
    std::unique_ptr<ma_sound, AudioDeleter> engineSound_;
    std::unique_ptr<ma_sound, AudioDeleter> driftSound_;
    bool initialized_;
    bool soundLoaded_;
    bool driftSoundLoaded_;
};
