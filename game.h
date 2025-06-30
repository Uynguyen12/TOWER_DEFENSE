#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "TileOptions.h"
#include <vector>
#include <string>
#include <iostream>
#include "UIElements.h"
using namespace std;

// ADDED: Define your game's internal, logical resolution
const unsigned int LOGICAL_WIDTH = 2560;
const unsigned int LOGICAL_HEIGHT = 1600;

class Game {
public:
	Game();
	~Game();

	enum GameMode {
		MainMenu,
		ProfileSelect,
		Settings,
		GetReady,
		Play,
		Paused,
		GameOver,
		LevelEditor
	};

	//Scroll wheel values
	enum ScrollWheel {
		ScrollUp,
		ScrollDown,
		None
	};

	struct PathTile {
		const Entity* pCurrentTile;
		const Entity* pNextTile;
	};

	void run();
private:
	/*void UpdatePlay();
	void UpdateTower();
	void UpdateAxe();
	void CheckForDeletionRequest();
	void UpdateLevelEditor();

	void UpdatePhysics();*/

	void UpdateMainMenu();
	void UpdateGetReady();
	void UpdatePlay();
	void UpdatePaused();
	void UpdateGameOver();
	void UpdateTower();
	void UpdateAxe();
	void CheckForDeletionRequest();
	void UpdateLevelEditor();
	void UpdatePhysics();
private:
	void ProcessCollision(Entity &entity1, Entity &entity2);
	bool isColiding(const Entity& entity1, const Entity& entity2);
public:
	/*void Draw();
	void DrawPlay();
	void DrawLevelEditor();*/
	void Draw();
	void DrawMainMenu();
	void DrawGetReady();
	void DrawPlay();
	void DrawPaused();
	void DrawGameOver();
	void DrawLevelEditor();

	/*void HandlePlayInput();
	void HandleLevelEditorInput();
	void HandleInput();*/
	void HandleInput();
	void HandleMainMenuInput();
	void HandleGetReadyInput();
	void HandlePlayInput();
	void HandlePausedInput();
	void HandleGameOverInput();
	void HandleLevelEditorInput();

	//Level Editor functions
	void CreateTileAtPosition(const sf::Vector2f& pos) ;
	void DeleteTileAtPosition(const sf::Vector2f& pos);
	void ConstructionPath();
	vector<Entity>& GetListOfTiles(TileOptions::TileType eTileType);

	// Play functions
	bool CreateTowerAtPosition(const sf::Vector2f& pos);
	bool CanPlaceTowerAtPosition(const sf::Vector2f& pos);

	void AddGold(int gold);

	// ADDED: A helper function to get mouse position in world coordinates
	sf::Vector2f getMouseWorldPosition() const;

	// ADDED: A function to handle window resizing
	void resizeView(unsigned int width, unsigned int height);

	// ADDED: Level and Save/Load Management
	void LoadLevel(int levelNumber);
	void SaveGame(const std::string& profileName);
	void LoadGame(const std::string& profileName);
	void StartNewGame();
	void WinGame();
private:
	sf::RenderWindow m_Window;
	sf::Time m_deltaTime;
	GameMode m_eGameMode;

	// ADDED: The main view for our game world
	sf::View m_gameView;

	//Play mode
	sf::Texture towerTexture;
	sf::Texture enemyTexture;
	sf::Texture axeTexture;

	// ADD THESE TWO LINES for the visual background map
	sf::Texture m_backgroundTexture;
	sf::Sprite m_backgroundSprite;


	Entity m_TowerTemplate;
	vector <Entity> m_Towers;

	Entity m_enemyTemplate;
	vector<Entity> m_enemies;

	Entity m_axeTemplate;
	vector<Entity> m_axes;

	//vector <Entity*> m_AllEntities;

	sf::Text m_GameModeText;
	sf::Font m_Font;
	//sf::Text m_PlayerText;
	sf::Text m_healthText;
	sf::Text m_goldText;
	sf::Text m_difficultyText;
	sf::Text m_levelText;
	sf::Text m_enemiesRemainingText;
	sf::Text m_GameOverText;
	sf::Text m_playingTimeText;

	//Level Editor Mode
	int m_optionIndex;
	ScrollWheel m_eScrollWheelInput;

	sf::Texture m_TileMapTexture;
	// TODO: these need to be entities, not sprites
	vector <TileOptions> m_TileOptions;
	vector <Entity> m_AestheticTiles;
	vector <Entity> m_SpawnTiles;
	vector <Entity> m_EndTiles;
	vector <Entity> m_PathTiles;

	bool m_bDrawPath;

	//GamePlay variables
	int m_iPlayerHealth;
	int m_iPlayerGold;
	int m_iGoldGainedThisUpdate;
	float m_fTimeInPlayMode;
	float m_fDifficulty;
	float m_fGoldPerSecond;
	float m_fGoldPerSecondTimer;
	float m_fLevelTimeGoal;

	// ADDED: State variables
	int m_iCurrentLevel;
	float m_fGetReadyTimer;

	// ADDED: UI Elements
	sf::Text m_TitleText;
	std::unique_ptr<Button> m_NewGameButton;
	std::unique_ptr<Button> m_LoadGameButton;
	std::unique_ptr<Button> m_SettingsButton;
	std::unique_ptr<Button> m_ExitButton;
	std::unique_ptr<Button> m_ResumeButton;
	std::unique_ptr<Button> m_BackToMenuButton;
private:
	//PathFinding
	typedef vector<PathTile> Path;

	void VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords);
	bool DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates);

	vector<Path> m_Paths;
};