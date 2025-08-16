#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Map.h"
#include "TitleScreen.h" 
#include "MapGrid.h"
#include <vector>
#include <string>
#include <iostream>
#include "PlayerTextManager.h"
#include "MenuManager.h"
#include "UIManager.h"
#include "TowerSelectionPanel.h"
#include "SpeedControlPanel.h"

using namespace std;

class Game {
public:
    Game();
    ~Game();

    enum GameMode {
        TitleScreenMode,
        Play
    };

    enum Difficulty {
        Easy,
        Medium,
        Hard,
        Extremely,
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
    void StartGame(int level, MenuManager::Difficulty difficulty);
    void ReturnToMenu();
    void ExitGame();
    void HandleGameExit();

    // Volume control
    void SetMusicVolume(float volume);
    void SetSoundVolume(float volume);

private:
    //UI game
    UIManager m_UIManager;
    TowerSelectionPanel m_TowerSelectionPanel;
    SpeedControlPanel m_SpeedControlPanel;

    // Update functions
    void UpdatePlay();
    void UpdateTower(float adjustedDeltaTime);
    void UpdateProjectiles(float adjustedDeltaTime);
    void CheckForDeletionRequest();
    void UpdatePhysics(float adjustedDeltaTime);
    void UpdateEnemySpawning(float adjustedDeltaTime);
    void UpdateEnemyMovement(float adjustedDeltaTime);
    void UpdateGoldCalculation(float adjustedDeltaTime);
    void CheckGameOver();
    void UpdateExperienceBarVisibility();
    void OnProfileChanged();


    // Add title screen update function
    void UpdateTitleScreen();

    // Collision functions
    void ProcessCollision(Entity& entity1, Entity& entity2);
    bool isColiding(const Entity& entity1, const Entity& entity2);

public:

    //Initialise
    void InitializeTowerSystem();
    void InitializeEnemySystem();
    void InitializeBulletSystem();

    // Draw functions
    void Draw();
    void DrawPlay();
    void UpdatePlayerText();
    void DrawTowerSelectionMenu();
    // Input handling - only for game controls, no tile placement
    void HandleInput();
    void HandleGameInput(sf::Event& event);
    void HandleKeyboardInput();
    void HandlePlayInput();
    void HandleTowerSelectionInput();

    // Add title screen input  float m_fEnemySpawnRate;
    void HandleTitleScreenInput(sf::Event& event);

    // Game state management
    void ResetGameState();
    void LoadMap(int map);
    void InitializeDifficulty(Difficulty difficulty);

    // Tower/Game logic
    bool CreateTowerAtPosition(const sf::Vector2f& pos, int towerType);
    bool CanPlaceTowerAtPosition(const sf::Vector2f& pos);
    void AddGold(int gold);
    void ShowTowerSelectionMenu(const sf::Vector2f& pos);

    // Range visualization methods
    void UpdateRangeVisualization(const sf::Vector2f& mousePos);
    void DrawRangeIndicator();
    void DrawGhostTower();
    int FindHoveredTower(const sf::Vector2f& mousePos);
    bool IsMouseOverExistingTower(const sf::Vector2f& mousePos, int& towerIndex);
    void ShowTowerRange(const sf::Vector2f& center, float radius, const sf::Color& color);
    void ShowGhostTower(const sf::Vector2f& position, int towerType, const sf::Color& tintColor);
    void UpdateTowerStats();
    void DrawTowerStats();
    void ShowTowerStatsPanel(int towerIndex);
    void HideTowerStatsPanel();
    void HideRange();
    void HideGhostTower();

    // Tower deletion
    bool DeleteTowerAtIndex(int towerIndex);
    void DrawDeleteButton();
    void UpdateDeleteButton(const sf::Vector2f& mousePos);
    bool IsMouseOverDeleteButton(const sf::Vector2f& mousePos);

    //Save game
    void SaveCurrentGameState();
    void LoadGameState();
    bool HasSavedGameData();
    void ClearSavedGameData();

    // Victory handling methods
    bool CheckVictoryConditions();
    void HandleVictory();
    int CalculateVictoryBonus();
    void ShowVictoryScreen();
    void OnGameCompleted();

    void HandleExperienceGain();
    void ShowLevelUpMessage(int newLevel);
    int CalculateExpGain() const;

    void HandleCoinsReward();

private:
    sf::RenderWindow m_Window;
    sf::Time m_deltaTime;
    GameMode m_eGameMode;
    Difficulty m_eDifficulty;
    
    //Player's text
     PlayerTextManager m_PlayerTextManager; 
    
    //Title screen
     TitleScreen m_TitleScreen;

    // Play mode specific
    std::vector<TowerConfig> m_TowerConfigs;
    std::vector<EnemyConfig> m_EnemyConfigs;
    Entity m_TowerTemplates[4];
    Entity m_EnemyTemplates[4];
    Entity m_BulletTemplates[4];
    Entity m_enemyTemplate;
    Entity m_bulletTemplate;
    std::vector<int> m_spawnedEnemies;
    std::vector<int> m_killedEnemies;

    std::vector<Entity> m_Towers;
    std::vector<Entity> m_enemies;
    std::vector<Entity> m_projectiles;

    // Textures for towers, enemies, and bullets
    sf::Texture m_towerTextures[4];
    sf::Texture m_enemyTextures[4];
    sf::Texture m_bulletTextures[4];

    // Speed control
    float m_GameSpeedMultiplier;

    Entity* FindClosestEnemyInRange(const Entity& tower);
    const Map::PathTile* FindClosestPathTile(const Entity& enemy, const Map::Path& path);

    sf::Text m_GameModeText;
    sf::Font m_Font;
    sf::Text m_PlayerText;
    sf::Text m_GameOverText;

    //Base
    sf::Vector2i m_BaseGridCoords;

    // Gameplay variables
    int m_iPlayerHealth;
    int m_iPlayerGold;
    int m_iStartingGold;
    int m_iGoldGainedThisUpdate;
    int m_iCurrentMap;
    int m_selectedTowerIndex;
    float m_fTimeInPlayMode;
    float m_fDifficulty;
    float m_fGoldPerSecond;
    float m_fGoldPerSecondTimer;


    float m_fEnemySpawnTimer;
    float m_fEnemySpawnRate;
    float fEnemySpeed;
    int m_iMaxEnemies;
    std::vector<int> m_TowerCounts;
    std::vector<int> m_TowerCosts;
    std::vector<int> m_TowerCount;
    int m_towerCost;
    float m_fTowerRange;

    // Range visualization
    sf::CircleShape m_RangeIndicator;
    bool m_bShowRange;
    sf::Vector2f m_RangeCenter;
    float m_CurrentRangeRadius;
    sf::Color m_RangeColor;
    int m_HoveredTowerIndex; // Index của tower đang được hover (-1 nếu không có)

    // Ghost tower preview
    sf::Sprite m_GhostTowerSprite;  
    bool m_bShowGhostTower;
    sf::Vector2f m_GhostTowerPosition;
    sf::Color m_GhostTowerColor;

    // Tower selection and stats
    int m_ClickedTowerIndex;           // Index của tower được click (-1 nếu không có)
    bool m_ShowTowerStats;             // Hiển thị bảng thống kê tower
    sf::RectangleShape m_StatsPanel;   // Background của bảng thống kê
    sf::Text m_StatsTitleText;         // Title của bảng thống kê
    sf::Text m_StatsContentText;       // Nội dung thống kê

    // Delete button for towers
    sf::RectangleShape m_DeleteButton;
    sf::Text m_DeleteButtonText;
    bool m_ShowDeleteButton;
    bool m_IsDeleteButtonHovered;

    // Mouse tracking
    sf::Vector2f m_LastMousePosition;

    // Game state
    bool m_bGameRunning;
    bool m_bGameOverSoundPlayed;
    bool m_bGameWonSoundPlayed;
    bool m_bGameOverTriggered;
    bool m_bVictoryTriggered;

    // Map system - only for display, no editing
    Map m_Map;
    MapGrid m_MapGrid;

    // Menu manager
    MenuManager m_MenuManager;

};

#endif // GAME_H