
#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "TileOptions.h"
#include "Map.h"
#include <vector>
#include <string>
#include <iostream>
#include "MenuManager.h"
#include "MapGrid.h"
#include <map> // Cần cho map trong Game

using namespace std;

class Game {
public:
    Game();
    ~Game();

    enum GameMode {
        Play,
        LevelEditor
    };

    // Scroll wheel values
    enum ScrollWheel {
        ScrollUp,
        ScrollDown,
        None
    };

    void run();

    // Menu functions
    void StartGame(int level); // Giữ nguyên signature, nhưng implementation sẽ thay đổi
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
    void UpdateLevelEditor();
    void UpdatePhysics();

    // Collision functions
    void ProcessCollision(Entity& entity1, Entity& entity2);
    bool isColiding(const Entity& entity1, const Entity& entity2);

public:
    // Draw functions
    void Draw();
    void DrawMenu(); // Dùng MenuManager
    void DrawPlay();
    void DrawLevelEditor();
	void UpdatePlayerText(); // Cập nhật text hiển thị thông tin người chơi

    // Input handling
    void HandleInput();
    void HandleGameInput(sf::Event& event);
    void HandleKeyboardInput();
    void HandlePlayInput();
    void HandleLevelEditorInput();

    // Game state management
    void ResetGameState();
    void LoadLevel(int level); // Hàm mới để tải dữ liệu level cụ thể

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
    vector <Entity> m_Towers;

    Entity m_enemyTemplate;
    vector<Entity> m_enemies;

    Entity m_axeTemplate;
    vector<Entity> m_axes;

    sf::Text m_GameModeText;
    sf::Font m_Font;
    sf::Text m_PlayerText;
    sf::Text m_GameOverText;

    // Level Editor Mode specific
    int m_optionIndex;
    ScrollWheel m_eScrollWheelInput;

    // Gameplay variables
    int m_iPlayerHealth;
    int m_iPlayerGold;
    int m_iGoldGainedThisUpdate;
    int m_iCurrentLevel; // Biến lưu level hiện tại đang chơi
    float m_fTimeInPlayMode;
    float m_fDifficulty;
    float m_fGoldPerSecond;
    float m_fGoldPerSecondTimer;

    // Game state
    bool m_bGameRunning;
    bool m_bGameOverSoundPlayed; // Đã phát âm thanh game over chưa

    // Map system
    Map m_Map;

    // Menu manager
    MenuManager m_MenuManager;


};

#endif // GAME_H

