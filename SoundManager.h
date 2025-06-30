#pragma once
#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SFML/Audio.hpp>
#include <map>
#include <string>

class SoundManager {
public:
    enum class SfxType {
        ButtonClick,
        EnemyHit,
        TowerShoot,
        Victory,
        Defeat
    };

    static SoundManager& getInstance();

    void playSfx(SfxType type);
    void playMusic(const std::string& filepath);
    void stopMusic();

    void setMusicVolume(float volume);
    void setSfxVolume(float volume);
    float getMusicVolume() const;
    float getSfxVolume() const;

    void toggleMusic(bool enabled);
    void toggleSfx(bool enabled);

private:
    SoundManager();
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    sf::Music m_music;
    std::map<SfxType, sf::SoundBuffer> m_sfxBuffers;
    std::vector<sf::Sound> m_sfxSounds; // Pool of sounds to play multiple at once

    float m_musicVolume;
    float m_sfxVolume;
    bool m_isMusicEnabled;
    bool m_isSfxEnabled;
};

#endif // SOUNDMANAGER_H