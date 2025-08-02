#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Map.h"
#include "MapGrid.h"
#include <vector>
#include <string>
#include "MenuManager.h"

class Game {
public:
    Game();
    ~Game();

    enum GameMode {
        Play
    };

    void run();

    // Menu functions
    void StartGame(int level);
    void ReturnToMenu();
    void ExitGame();

    // Volume control
    void SetMusicVolume(float volume);
    void SetSoundVolume(float volume);

private:
    // Update functions
    void UpdatePlay();
    void UpdateTower();
    void UpdateAxe();
    void CheckForDeletionRequest();
    void UpdatePhysics();

    // Collision functions
    void ProcessCollision(Entity& entity1, Entity& entity2);
    bool isColiding(const Entity& entity1, const Entity& entity2);

    // Draw functions
    void Draw();
    void DrawPlay();
    void UpdatePlayerText();

    // Input handling
    void HandleInput();
    void HandleGameInput(sf::Event& event);
    void HandleKeyboardInput();

    // Game state management
    void ResetGameState();
    void LoadLevel(int level);

    // Tower/Game logic
    bool CreateTowerAtPosition(const sf::Vector2f& pos);
    bool CanPlaceTowerAtPosition(const sf::Vector2f& pos);
    void AddGold(int gold);

private:
    sf::RenderWindow m_Window;
    sf::Time m_deltaTime;
    GameMode m_eGameMode;

    // Play mode specific
    sf::Texture towerTexture;
    sf::Texture enemyTexture;
    sf::Texture axeTexture;

    Entity m_TowerTemplate;
    std::vector<Entity> m_Towers;

    Entity m_enemyTemplate;
    std::vector<Entity> m_enemies;

    Entity m_axeTemplate;
    std::vector<Entity> m_axes;

    sf::Text m_GameModeText;
    sf::Font m_Font;
    sf::Text m_PlayerText;
    sf::Text m_GameOverText;

    // Gameplay variables
    int m_iPlayerHealth;
    int m_iPlayerGold;
    int m_iGoldGainedThisUpdate;
    int m_iCurrentLevel;
    float m_fTimeInPlayMode;
    float m_fDifficulty;
    float m_fGoldPerSecond;
    float m_fGoldPerSecondTimer;

    // Game state
    bool m_bGameRunning;
    bool m_bGameOverSoundPlayed;

    // Map system
    Map m_Map;
    MapGrid m_MapGrid;

    // Menu manager
    MenuManager m_MenuManager;
};

#endif // GAME_H