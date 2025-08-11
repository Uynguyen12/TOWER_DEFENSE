#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SFML/Audio.hpp>
#include <vector>
#include <string>

class SoundManager {
private:
    SoundManager();
    ~SoundManager();

public:
    static SoundManager& getInstance() {
        return m_Instance;
    }

    // Load audio files
    void Initialize();

    // Play different types of sounds
    void PlayBackgroundMusic();
    void PauseBackgroundMusic();
    void ResumeBackgroundMusic();
    void StopBackgroundMusic();
    void PlayThrowingSound();
    void PlayHitSound();
    void PlayEnemyDeathSound();
    void PlayTowerPlaceSound();
    void PlayGameOverSound();
    void PlayRandomGameWonSound();
    void PlayGameWonSound();
    void StopGameOverSound();
    void StopGameWonSound();
    void PlayWindSound();
    void PlayFireSound();
    void PlayDragonRoarSound();
    void StopAmbientSounds();
    void UpdateAmbientSound(); // gọi trong vòng lặp chính
    void PlayRandomAmbientSound();
    void StartAmbientSoundCycle(); // khi vào menu

    // Volume control
    void SetMusicVolume(float volume); // 0.0f to 100.0f
    void SetSoundVolume(float volume); // 0.0f to 100.0f

    // Cleanup
    void Cleanup();

private:
    static SoundManager m_Instance;

    // Music
    sf::Music m_BackgroundMusic;
    sf::Clock m_ambientTimer;

    // Sound effects
    sf::SoundBuffer m_ThrowingSoundBuffer;
    sf::SoundBuffer m_HitSoundBuffer;
    sf::SoundBuffer m_EnemyDeathSoundBuffer;
    sf::SoundBuffer m_TowerPlaceSoundBuffer;
    sf::SoundBuffer m_GameOverSoundBuffer;
    sf::SoundBuffer m_GameWonSoundBuffer1;
    sf::SoundBuffer m_GameWonSoundBuffer2;
    sf::SoundBuffer m_ambientWindBuffer; // Added for ambient wind sound
    sf::SoundBuffer m_ambientFireBuffer; // Added for ambient fire sound
    sf::SoundBuffer m_dragonRoarBuffer; // Added for dragon roar sound
    sf::Sound m_ambientWindSound;
    sf::Sound m_ambientFireSound;
    sf::Sound m_dragonRoarSound;

    // Sound objects (we need multiple for overlapping sounds)
    std::vector<sf::Sound> m_ThrowingSounds;
    std::vector<sf::Sound> m_HitSounds;
    std::vector<sf::Sound> m_EnemyDeathSounds;
    sf::Sound m_TowerPlaceSound;
    sf::Sound m_GameOverSound;
    sf::Sound m_GameWonSound1;
    sf::Sound m_GameWonSound2;

    // Settings
    float m_fMusicVolume;
    float m_fSoundVolume;
    float m_nextAmbientInterval = 0.f;

    // Helper methods
    void CreateSoundPool();
    sf::Sound* GetAvailableSound(std::vector<sf::Sound>& soundPool);
};

#endif