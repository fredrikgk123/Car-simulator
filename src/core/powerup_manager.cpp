#include "core/powerup_manager.hpp"
#include "core/game_config.hpp"
#include "core/random_position_generator.hpp"


PowerupManager::PowerupManager(int count, float playAreaSize) {
    generatePowerups(count, playAreaSize);
}

void PowerupManager::generatePowerups(int count, float playAreaSize) {
    powerups_.clear();

    RandomPositionGenerator posGen(playAreaSize, GameConfig::Powerup::SPAWN_MARGIN);

    for (int i = 0; i < count; ++i) {
        auto pos = posGen.getRandomPosition();
        auto powerup = std::make_unique<Powerup>(pos[0], GameConfig::Powerup::HEIGHT, pos[1], PowerupType::NITROUS);
        powerups_.push_back(std::move(powerup));
    }
}

void PowerupManager::update(float deltaTime) {
    // Powerups are static objects - no updates needed
}

void PowerupManager::handleCollisions(Vehicle& vehicle) {
    for (auto& powerup : powerups_) {
        // Can only collect if: active, don't have nitrous, not using nitrous, and touching it
        if (powerup->isActive() &&
            !vehicle.hasNitrous() &&
            !vehicle.isNitrousActive() &&
            vehicle.intersects(*powerup)) {
            vehicle.pickupNitrous();
            powerup->setActive(false);
        }
    }
}

void PowerupManager::reset() noexcept {
    for (auto& powerup : powerups_) {
        powerup->setActive(true);
    }
}

const std::vector<std::unique_ptr<Powerup>>& PowerupManager::getPowerups() const noexcept {
    return powerups_;
}

size_t PowerupManager::getCount() const noexcept {
    return powerups_.size();
}
