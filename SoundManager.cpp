#include "SoundManager.h"
#include <stdexcept>
#include <algorithm> 

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() : m_musicVolume(50.f), m_sfxVolume(80.f), m_isMusicEnabled(true), m_isSfxEnabled(true) {
    if (!m_sfxBuffers[SfxType::ButtonClick].loadFromFile("assets/audio/sfx/button_click.wav")) {
        // throw std::runtime_error("Failed to load button_click.wav");
    }
    if (!m_sfxBuffers[SfxType::EnemyHit].loadFromFile("assets/audio/sfx/enemy_hit.wav")) {
        // throw std::runtime_error("Failed to load enemy_hit.wav");
    }
    if (!m_sfxBuffers[SfxType::TowerShoot].loadFromFile("assets/audio/sfx/tower_shoot.wav")) {
        // throw std::runtime_error("Failed to load tower_shoot.wav");
    }
    if (!m_sfxBuffers[SfxType::Victory].loadFromFile("assets/audio/sfx/victory.wav")) {
        // throw std::runtime_error("Failed to load victory.wav");
    }
    if (!m_sfxBuffers[SfxType::Defeat].loadFromFile("assets/audio/sfx/defeat.wav")) {
        // throw std::runtime_error("Failed to load defeat.wav");
    }
}

void SoundManager::playSfx(SfxType type) {
    if (!m_isSfxEnabled) return;

    // Find an available sound player in the pool
    sf::Sound* soundToPlay = nullptr;
    for (auto& sound : m_sfxSounds) {
        if (sound.getStatus() == sf::Sound::Stopped) {
            soundToPlay = &sound;
            break;
        }
    }

    // If no available sound player, create a new one
    if (!soundToPlay) {
        m_sfxSounds.emplace_back();
        soundToPlay = &m_sfxSounds.back();
    }

    soundToPlay->setBuffer(m_sfxBuffers.at(type));
    soundToPlay->setVolume(m_sfxVolume);
    soundToPlay->play();
}

void SoundManager::playMusic(const std::string& filepath) {
    if (!m_isMusicEnabled) return;
    if (!m_music.openFromFile(filepath)) {
        throw std::runtime_error("Failed to load music: " + filepath);
    }
    m_music.setVolume(m_musicVolume);
    m_music.setLoop(true);
    m_music.play();
}

void SoundManager::stopMusic() {
    m_music.stop();
}

void SoundManager::setMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.f, 100.f);
    if (m_isMusicEnabled) m_music.setVolume(m_musicVolume);
}

void SoundManager::setSfxVolume(float volume) {
    m_sfxVolume = std::clamp(volume, 0.f, 100.f);
}

float SoundManager::getMusicVolume() const { return m_musicVolume; }
float SoundManager::getSfxVolume() const { return m_sfxVolume; }

void SoundManager::toggleMusic(bool enabled) {
    m_isMusicEnabled = enabled;
    if (m_isMusicEnabled) {
        m_music.setVolume(m_musicVolume);
    }
    else {
        m_music.setVolume(0);
    }
}

void SoundManager::toggleSfx(bool enabled) {
    m_isSfxEnabled = enabled;
}