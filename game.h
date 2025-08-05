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

    enum Difficulty {
        Easy,
        Medium,
        Hard,
        VeryHard
    };

    struct EnemyConfig {
        int count;
        int health;
        int goldReward;
        float speed;
        int damageToBase; // Added damage to base attribute
        sf::Texture texture;
    };

    struct TowerConfig {
        int maxCount;
        int cost;
        int damage;
        float range;
        sf::Texture texture;
    };

    void run();

    // Menu functions
    void StartGame(int level, Difficulty difficulty);
    void ReturnToMenu();
    void ExitGame();

    // Volume control
    void SetMusicVolume(float volume);
    void SetSoundVolume(float volume);

private:
    // Update functions
    void UpdatePlay();
    void UpdateTower();
    void UpdateProjectiles();
    void CheckForDeletionRequest();
    void UpdatePhysics();

    // Collision functions
    void ProcessCollision(Entity& entity1, Entity& entity2);
    bool isColiding(const Entity& entity1, const Entity& entity2);

    // Draw functions
    void Draw();
    void DrawPlay();
    void UpdatePlayerText();
    void DrawTowerSelectionMenu();

    // Input handling
    void HandleInput();
    void HandleGameInput(sf::Event& event);
    void HandleKeyboardInput();
    void HandleTowerSelectionInput();

    // Game state management
    void ResetGameState();
    void LoadLevel(int level);
    void InitializeDifficulty(Difficulty difficulty);

    // Tower/Game logic
    bool CreateTowerAtPosition(const sf::Vector2f& pos, int towerType);
    bool CanPlaceTowerAtPosition(const sf::Vector2f& pos);
    void AddGold(int gold);
    void ShowTowerSelectionMenu(const sf::Vector2f& pos);

private:
    sf::RenderWindow m_Window;
    sf::Time m_deltaTime;
    GameMode m_eGameMode;
    Difficulty m_eDifficulty;

    // Play mode specific
    std::vector<TowerConfig> m_TowerConfigs;
    std::vector<EnemyConfig> m_EnemyConfigs;
    Entity m_TowerTemplates[4];
    Entity m_EnemyTemplates[4];
    Entity m_BulletTemplates[4];

    std::vector<int> m_spawnedEnemies;
	std::vector<int> m_killedEnemies;


    std::vector<Entity> m_Towers;
    std::vector<Entity> m_enemies;
    std::vector<Entity> m_axes;
    
	// Textures for towers, enemies, and bullets
	sf::Texture m_towerTextures[4];
	sf::Texture m_enemyTextures[4];
	sf::Texture m_bulletTextures[4];

	// UI elements
    sf::Text m_GameModeText;
    sf::Font m_Font;
    sf::Text m_PlayerText;
    sf::Text m_GameOverText;

    // Tower selection menu
    bool m_bShowTowerSelection;
    sf::Vector2f m_TowerSelectionPos;
    std::vector<sf::RectangleShape> m_TowerButtons;
    std::vector<sf::Text> m_TowerButtonTexts;

    // Gameplay variables
    int m_iPlayerHealth;
    int m_iPlayerGold;
    int m_iStartingGold;
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

    // Tower counts
    int m_TowerCounts[4];
};

#endif // GAME_H