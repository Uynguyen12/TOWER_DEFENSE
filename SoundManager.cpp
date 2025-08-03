#include "SoundManager.h"
#include <iostream>

SoundManager SoundManager::m_Instance;

SoundManager::SoundManager()
    : m_fMusicVolume(50.0f)
    , m_fSoundVolume(70.0f)
{
}

SoundManager::~SoundManager() {
    Cleanup();
}

void SoundManager::Initialize() {
    // Load background music
    if (!m_BackgroundMusic.openFromFile("sound/background.wav")) {
        std::cout << "Warning: Could not load background music from 'sound/background.wav'" << std::endl;
    }

    // Configure background music
    m_BackgroundMusic.setLoop(true);
    m_BackgroundMusic.setVolume(m_fMusicVolume);

    // Load sound effect buffers
    if (!m_ThrowingSoundBuffer.loadFromFile("sound/axe_throw.wav")) {
        std::cout << "Warning: Could not load throwing sound from 'sound/axe_throw.wav'" << std::endl;
    }
    if (!m_HitSoundBuffer.loadFromFile("sound/axe_hit.wav")) {
        std::cout << "Warning: Could not load hit sound from 'sound/axe_hit.wav'" << std::endl;
    }
    if (!m_EnemyDeathSoundBuffer.loadFromFile("sound/enemy_death.wav")) {
        std::cout << "Warning: Could not load enemy death sound from 'sound/enemy_death.wav'" << std::endl;
    }
    if (!m_TowerPlaceSoundBuffer.loadFromFile("sound/tower_place.wav")) {
        std::cout << "Warning: Could not load tower place sound from 'sound/tower_place.wav'" << std::endl;
    }
    if (!m_GameOverSoundBuffer.loadFromFile("sound/gameover.wav")) {
        std::cout << "Warning: Could not load game over sound from 'sound/gameover.wav'" << std::endl;
    }
    if (!m_ambientWindBuffer.loadFromFile("sound/ambient_wind.wav")) {
        std::cout << "Warning: Could not load ambient wind sound from 'sound/ambient_wind.wav'" << std::endl;
    }
    if (!m_ambientFireBuffer.loadFromFile("sound/ambient_fire.wav")) {
        std::cout << "Warning: Could not load ambient fire sound from 'sound/ambient_fire.wav'" << std::endl;
    }
    if (!m_dragonRoarBuffer.loadFromFile("sound/dragon_roar.wav")) {
        std::cout << "Warning: Could not load dragon roar sound from 'sound/dragon_roar.wav'" << std::endl;
    }

    // Create sound pools and configure them
    CreateSoundPool();

    // Configure individual sounds
    m_TowerPlaceSound.setBuffer(m_TowerPlaceSoundBuffer);
    m_TowerPlaceSound.setVolume(m_fSoundVolume);

    m_GameOverSound.setBuffer(m_GameOverSoundBuffer);
    m_GameOverSound.setVolume(m_fSoundVolume);

    m_ambientWindSound.setBuffer(m_ambientWindBuffer);
    m_ambientWindSound.setVolume(m_fSoundVolume);
    m_ambientWindSound.setLoop(true); // Ambient sounds typically loop

    m_ambientFireSound.setBuffer(m_ambientFireBuffer);
    m_ambientFireSound.setVolume(m_fSoundVolume);
    m_ambientFireSound.setLoop(true); // Ambient sounds typically loop

    m_dragonRoarSound.setBuffer(m_dragonRoarBuffer);
    m_dragonRoarSound.setVolume(m_fSoundVolume);
}

void SoundManager::CreateSoundPool() {
    const int throwingSoundPoolSize = 8; // Allow 8 simultaneous throwing sounds
    const int hitSoundPoolSize = 5;  // Allow 5 simultaneous hit sounds
    const int deathSoundPoolSize = 3; // Allow 3 simultaneous death sounds
    const int ambientSoundPoolSize = 3; // Allow 3 simultaneous ambient sounds

    // Create throwing sound pool
    m_ThrowingSounds.clear();
    for (int i = 0; i < throwingSoundPoolSize; ++i) {
        sf::Sound sound;
        sound.setBuffer(m_ThrowingSoundBuffer);
        sound.setVolume(m_fSoundVolume);
        m_ThrowingSounds.push_back(sound);
    }

    // Create hit sound pool
    m_HitSounds.clear();
    for (int i = 0; i < hitSoundPoolSize; ++i) {
        sf::Sound sound;
        sound.setBuffer(m_HitSoundBuffer);
        sound.setVolume(m_fSoundVolume);
        m_HitSounds.push_back(sound);
    }

    // Create death sound pool
    m_EnemyDeathSounds.clear();
    for (int i = 0; i < deathSoundPoolSize; ++i) {
        sf::Sound sound;
        sound.setBuffer(m_EnemyDeathSoundBuffer);
        sound.setVolume(m_fSoundVolume);
        m_EnemyDeathSounds.push_back(sound);
    }
}

sf::Sound* SoundManager::GetAvailableSound(std::vector<sf::Sound>& soundPool) {
    // Find a sound that's not currently playing
    for (auto& sound : soundPool) {
        if (sound.getStatus() != sf::Sound::Playing) {
            return &sound;
        }
    }

    // If all sounds are playing, return the first one (it will be interrupted)
    if (!soundPool.empty()) {
        return &soundPool[0];
    }

    return nullptr;
}

void SoundManager::PlayBackgroundMusic() {
    if (m_BackgroundMusic.getStatus() != sf::Music::Playing) {
        m_BackgroundMusic.play();
    }
}

void SoundManager::StopBackgroundMusic() {
    m_BackgroundMusic.stop();
}

void SoundManager::PauseBackgroundMusic() {
    if (m_BackgroundMusic.getStatus() == sf::Music::Playing) {
        m_BackgroundMusic.pause();
    }
}

void SoundManager::ResumeBackgroundMusic() {
    if (m_BackgroundMusic.getStatus() == sf::Music::Paused) {
        m_BackgroundMusic.play();
    }
    else if (m_BackgroundMusic.getStatus() == sf::Music::Stopped) {
        m_BackgroundMusic.play();
    }
}

void SoundManager::PlayThrowingSound() {
    sf::Sound* sound = GetAvailableSound(m_ThrowingSounds);
    if (sound) {
        sound->play();
    }
}

void SoundManager::PlayHitSound() {
    sf::Sound* sound = GetAvailableSound(m_HitSounds);
    if (sound) {
        sound->play();
    }
}

void SoundManager::PlayEnemyDeathSound() {
    sf::Sound* sound = GetAvailableSound(m_EnemyDeathSounds);
    if (sound) {
        sound->play();
    }
}

void SoundManager::PlayTowerPlaceSound() {
    if (m_TowerPlaceSound.getStatus() != sf::Sound::Playing) {
        m_TowerPlaceSound.play();
    }
}

void SoundManager::PlayGameOverSound() {
    m_GameOverSound.play();
}

void SoundManager::PlayWindSound() {
    if (m_ambientWindSound.getStatus() != sf::Sound::Playing) {
        m_ambientWindSound.play();
    }
}

void SoundManager::PlayFireSound() {
    if (m_ambientFireSound.getStatus() != sf::Sound::Playing) {
        m_ambientFireSound.play();
    }
}

void SoundManager::PlayDragonRoarSound() {
    if (m_dragonRoarSound.getStatus() != sf::Sound::Playing) {
        m_dragonRoarSound.play();
    }
}

void SoundManager::StartAmbientSoundCycle() {
    m_nextAmbientInterval = 5.f + static_cast<float>(rand() % 6); // 5–10s ngẫu nhiên
    m_ambientTimer.restart();
    PlayRandomAmbientSound(); // Phát lần đầu
}

void SoundManager::PlayRandomAmbientSound() {
    StopAmbientSounds();
    int random = rand() % 3;
    switch (random) {
    case 0:
        m_ambientFireSound.play();
        break;
    case 1:
        m_ambientWindSound.play();
        break;
    case 2:
        m_dragonRoarSound.play();
        break;
    }
}
void SoundManager::UpdateAmbientSound() {
    if (m_ambientTimer.getElapsedTime().asSeconds() >= m_nextAmbientInterval) {
        PlayRandomAmbientSound(); // Dừng âm cũ, phát âm mới
        m_nextAmbientInterval = 5.f + static_cast<float>(rand() % 6); // Reset khoảng thời gian
        m_ambientTimer.restart();
    }
}

void SoundManager::StopAmbientSounds() {
    m_ambientFireSound.stop();
    m_ambientWindSound.stop();
    m_dragonRoarSound.stop();
}
void SoundManager::SetMusicVolume(float volume) {
    m_fMusicVolume = std::max(0.0f, std::min(100.0f, volume));
    m_BackgroundMusic.setVolume(m_fMusicVolume);
}

void SoundManager::SetSoundVolume(float volume) {
    m_fSoundVolume = std::max(0.0f, std::min(100.0f, volume));

    // Update all sound effects
    for (auto& sound : m_ThrowingSounds) {
        sound.setVolume(m_fSoundVolume);
    }

    for (auto& sound : m_HitSounds) {
        sound.setVolume(m_fSoundVolume);
    }

    for (auto& sound : m_EnemyDeathSounds) {
        sound.setVolume(m_fSoundVolume);
    }

    m_TowerPlaceSound.setVolume(m_fSoundVolume);
    m_GameOverSound.setVolume(m_fSoundVolume);
    m_ambientWindSound.setVolume(m_fSoundVolume);
    m_ambientFireSound.setVolume(m_fSoundVolume);
    m_dragonRoarSound.setVolume(m_fSoundVolume);
}

void SoundManager::Cleanup() {
    StopBackgroundMusic();

    // Stop all sound effects
    for (auto& sound : m_ThrowingSounds) {
        sound.stop();
    }

    for (auto& sound : m_HitSounds) {
        sound.stop();
    }

    for (auto& sound : m_EnemyDeathSounds) {
        sound.stop();
    }

    m_TowerPlaceSound.stop();
    m_GameOverSound.stop();
    m_ambientWindSound.stop();
    m_ambientFireSound.stop();
    m_dragonRoarSound.stop();
}