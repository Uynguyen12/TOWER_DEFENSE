#include "game.h"
#include <SFML/Graphics.hpp>
#include "MathHelpers.h"
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include "DamageTextManager.h"
#include "SoundManager.h"
#include "MenuManager.h"
Game::Game()
    : m_Window()
    , m_eGameMode(TitleScreenMode)
    , m_enemyTemplate(Entity::PhysicsData::Type::Dynamic, 0)
    , m_bulletTemplate(Entity::PhysicsData::Type::Dynamic, 0)
    , m_iPlayerHealth(10)
    , m_iPlayerGold(10)
    , m_fTimeInPlayMode(0.0f)
    , m_fDifficulty(1.0f)
    , m_iGoldGainedThisUpdate(0)
    , m_fGoldPerSecond(0.1f)
    , m_fGoldPerSecondTimer(0.0f)
    , m_bGameRunning(true)
    , m_bGameOverSoundPlayed(false)
    , m_bGameOverTriggered(false)
    , m_bGameWonSoundPlayed(false)
    , m_bVictoryTriggered(false)
    , m_iCurrentMap(0)
    , m_fEnemySpawnTimer(0.0f)
    , m_fEnemySpawnRate(2.5f)
    , m_iMaxEnemies(30)
    , m_fTowerRange(300.0f)
    , fEnemySpeed(100.0f)
    , m_selectedTowerIndex(-1)
    , m_bShowRange(false)
    , m_RangeCenter(0.0f, 0.0f)
    , m_CurrentRangeRadius(0.0f)
    , m_RangeColor(sf::Color::Blue)
    , m_HoveredTowerIndex(-1)
    , m_LastMousePosition(0.0f, 0.0f)
    , m_bShowGhostTower(false)
    , m_GhostTowerPosition(0.0f, 0.0f)
    , m_GhostTowerColor(sf::Color::Blue)
    , m_ClickedTowerIndex(-1) 
    , m_ShowTowerStats(false)
{
    m_MenuManager.LoadSettingsFromFile();

    auto settings = m_MenuManager.GetSettings();
    m_Window.create(sf::VideoMode(settings.resolution.x, settings.resolution.y), "SFML window");

    if (!m_TitleScreen.Initialize(m_Window)) {
        throw std::runtime_error("Failed to initialize Title Screen");
    }

    // Gọi thêm hàm resize để đảm bảo vị trí các phần tử UI đúng
    m_TitleScreen.HandleResize(m_Window.getSize());

    // Initialize MenuManager
    m_MenuManager.Initialize(m_Window);

    // Set up resolution change callback to update UI text positions
    m_MenuManager.SetResolutionChangeCallback([this](sf::Vector2u newResolution) {
        std::cout << "Game received resolution change: " << newResolution.x << "x" << newResolution.y << std::endl;
        // Update UI text positions
        m_GameModeText.setPosition(sf::Vector2f(newResolution.x * 0.75f, newResolution.y * 0.1f));
        m_PlayerText.setPosition(sf::Vector2f(newResolution.x * 0.75f, newResolution.y * 0.05f));
        m_GameOverText.setPosition(sf::Vector2f(newResolution.x * 0.5f - m_GameOverText.getLocalBounds().width / 2, newResolution.y * 0.5f));
        // Update map scaling (assumes Map::UpdateScale exists)
        //m_Map.UpdateScale(sf::Vector2f(newResolution));
        });

    // Set up volume callbacks
    m_MenuManager.SetMusicVolumeCallback([this](float volume) {
        SetMusicVolume(volume);
        });
    m_MenuManager.SetSFXVolumeCallback([this](float volume) {
        SetSoundVolume(volume);
        });

    // Initialize SoundManager
    SoundManager::getInstance().Initialize();
    SoundManager::getInstance().PlayBackgroundMusic();


    // Initialize Font
    if (!m_Font.loadFromFile("Fonts/MedievalSharp-Regular.ttf")) {
        throw std::runtime_error("Failed to load font from 'Fonts/MedievalSharp-Regular.ttf'");
    }

    // Initialize UI Text
    sf::Vector2u windowSize = m_Window.getSize();
    m_GameModeText.setFont(m_Font);
    m_GameModeText.setPosition(sf::Vector2f(windowSize.x * 0.4f, windowSize.y * 0.1f));
    m_GameModeText.setString("Play Mode");

    m_PlayerText.setPosition(sf::Vector2f(1500, 100));
    m_PlayerText.setFont(m_Font);

    m_GameOverText.setCharacterSize(100);
    m_GameOverText.setPosition(sf::Vector2f(windowSize.x * 0.5f - m_GameOverText.getLocalBounds().width / 2, windowSize.y * 0.5f));
    m_GameOverText.setString("GAME OVERRR");


    // Set up MenuManager callbacks
    m_MenuManager.SetExitCallback([this](sf::RenderWindow& window) {
        this->ExitGame();
        });

    m_MenuManager.SetStartGameCallback([this](int map, MenuManager::Difficulty difficulty) {
        this->StartGame(map, difficulty);
        });

    m_MenuManager.SetSaveGameCallback([this]() {
        this->SaveCurrentGameState();
        });

    m_MenuManager.SetClearGameDataCallback([this]() {
        this->ClearSavedGameData();
        });

    // Khởi tạo kích thước cho các vector
    m_TowerConfigs.resize(4);
    m_EnemyConfigs.resize(4);
    m_spawnedEnemies.resize(4, 0);
    m_killedEnemies.resize(4, 0);
    m_TowerCounts.resize(4, 0);
    m_TowerCosts.resize(4);
    m_TowerCount.resize(4);

    // Initialize range indicator
    m_RangeIndicator.setFillColor(sf::Color::Transparent);
    m_RangeIndicator.setOutlineThickness(3.0f);

    // Initialize ghost tower sprite
    m_GhostTowerSprite.setScale(sf::Vector2f(1.0f, 1.0f));
    m_GhostTowerSprite.setOrigin(sf::Vector2f(32, 32));

    // Initialize stats panel
    m_StatsPanel.setSize(sf::Vector2f(250.0f, 180.0f));
    m_StatsPanel.setFillColor(sf::Color(40, 30, 20, 230));
    m_StatsPanel.setOutlineColor(sf::Color(180, 140, 100));
    m_StatsPanel.setOutlineThickness(3.0f);
    m_StatsPanel.setPosition(20.0f, 200.0f); // Vị trí bên trái màn hình

    // Initialize stats text
    m_StatsTitleText.setFont(m_Font);
    m_StatsTitleText.setCharacterSize(20);
    m_StatsTitleText.setFillColor(sf::Color(255, 215, 0));
    m_StatsTitleText.setStyle(sf::Text::Bold);

    m_StatsContentText.setFont(m_Font);
    m_StatsContentText.setCharacterSize(16);
    m_StatsContentText.setFillColor(sf::Color(255, 240, 200));

     // Load tower textures and initialize templates
    InitializeTowerSystem();

    // Load enemy textures and initialize templates
    InitializeEnemySystem();

    // Load bullet textures and initialize templates
    InitializeBulletSystem();


    // Initialize UIManager
    if (!m_UIManager.Initialize()) {
        throw std::runtime_error("Failed to initialize UI Manager");
    }

    // Initialize TowerSelectionPanel
    if (!m_TowerSelectionPanel.Initialize(m_Window)) {
        throw std::runtime_error("Failed to initialize Tower Selection Panel");
    }

    m_TowerSelectionPanel.SetWindowSize(m_Window.getSize());

    // Set initial UI positions
    m_UIManager.SetHealthBarPosition(sf::Vector2f(50.0f, 50.0f));
    m_UIManager.SetHealthBarSize(sf::Vector2f(200.0f, 20.0f));
    m_UIManager.UpdateHealthBar(m_iPlayerHealth, 100);
}

Game::~Game() {
    SoundManager::getInstance().Cleanup();
}

void Game::run() {
    sf::Clock clock;
    while (m_Window.isOpen()) {
        m_deltaTime = clock.restart();
        HandleInput();
        if (!m_MenuManager.IsInGamePlay() || m_MenuManager.IsGamePaused() || m_MenuManager.IsGameOver() || m_MenuManager.IsGameWon()) {
            m_MenuManager.Update(m_Window, m_deltaTime.asSeconds());
        }
        else {
            UpdatePlay();
        }
        Draw();
    }
}

void Game::InitializeTowerSystem() {
    // Load tower textures
    std::vector<std::string> towerTexturePaths = {
        "image/sprite/Tower1.png",
        "image/sprite/Tower2.png",
        "image/sprite/Tower3.png",
        "image/sprite/Tower4.png"
    };

    for (size_t i = 0; i < 4; ++i) {
        if (!m_towerTextures[i].loadFromFile(towerTexturePaths[i])) {
            throw std::runtime_error("Failed to load tower texture from '" + towerTexturePaths[i] + "'");
        }
        m_TowerConfigs[i].texture = m_towerTextures[i];
    }

    // Initialize tower templates
    for (int i = 0; i < 4; ++i) {
        m_TowerTemplates[i] = Entity(Entity::PhysicsData::Type::Static, i + 1);
        m_TowerTemplates[i].SetTexture(m_TowerConfigs[i].texture);
        m_TowerTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_TowerTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_TowerTemplates[i].setCirclePhysics(32.f);
        m_TowerTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Tower);
        m_TowerTemplates[i].SetType(i + 1); // Set tower type (1-4)
        m_TowerConfigs[i].maxCount = 0; // Sẽ được cập nhật trong InitializeDifficulty
        m_TowerConfigs[i].cost = 0;
        m_TowerConfigs[i].damage = 0;
        m_TowerConfigs[i].range = 0.0f;
    }
}

void Game::InitializeEnemySystem() {
    // Load enemy textures
    std::vector<std::string> enemyTexturePaths = {
        "image/sprite/Enemy1.png",
        "image/sprite/Enemy2.png",
        "image/sprite/Enemy3.png",
        "image/sprite/Enemy4.png"
    };

    for (size_t i = 0; i < 4; ++i) {
        if (!m_enemyTextures[i].loadFromFile(enemyTexturePaths[i])) {
            throw std::runtime_error("Failed to load enemy texture from '" + enemyTexturePaths[i] + "'");
        }
        m_EnemyConfigs[i].texture = m_enemyTextures[i];
    }

    // Initialize enemy templates
    for (int i = 0; i < 4; ++i) {
        m_EnemyTemplates[i] = Entity(Entity::PhysicsData::Type::Dynamic, i + 1);
        m_EnemyTemplates[i].SetTexture(m_EnemyConfigs[i].texture);
        m_EnemyTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_EnemyTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_EnemyTemplates[i].setCirclePhysics(32.f);
        m_EnemyTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Enemy);
        m_EnemyTemplates[i].SetType(i + 1); // Set enemy type (1-4)

        // Health bar setup will be done in InitializeDifficulty
        m_EnemyTemplates[i].ShowHealthBar(true);
        m_EnemyTemplates[i].SetHealthBarSize(50.0f, 6.0f);
        m_EnemyTemplates[i].SetHealthBarOffset(sf::Vector2f(0.0f, -45.0f));
    }
}

void Game::InitializeBulletSystem() {
    // Load bullet textures
    std::vector<std::string> bulletTexturePaths = {
        "image/sprite/Bullet1.png",
        "image/sprite/Bullet2.png",
        "image/sprite/Bullet3.png",
        "image/sprite/Bullet4.png"
    };

    for (size_t i = 0; i < 4; ++i) {
        if (!m_bulletTextures[i].loadFromFile(bulletTexturePaths[i])) {
            throw std::runtime_error("Failed to load bullet texture from '" + bulletTexturePaths[i] + "'");
        }
    }

    // Initialize bullet templates
    for (int i = 0; i < 4; ++i) {
        m_BulletTemplates[i] = Entity(Entity::PhysicsData::Type::Dynamic, i + 1);
        m_BulletTemplates[i].SetTexture(m_bulletTextures[i]);
        m_BulletTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_BulletTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_BulletTemplates[i].setCirclePhysics(32.f);
        m_BulletTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Projectile);
        m_BulletTemplates[i].GetPhysicsDataNonConst().setLayersToIgnore(
            Entity::PhysicsData::Layer::Projectile | Entity::PhysicsData::Layer::Tower);
        m_BulletTemplates[i].SetType(i + 1); // Set bullet type (1-4)
        m_BulletTemplates[i].m_fBulletTimer = 3.0f; // 3 second lifespan
    }
}

void Game::InitializeDifficulty(Difficulty difficulty) {
    switch (difficulty) {
    case Easy:
        m_EnemyConfigs[0] = { 30, 10, 1, 1.8f, 1, m_EnemyConfigs[0].texture }; // Enemy 1: 1 damage to base
        m_EnemyConfigs[1] = { 25, 20, 2, 1.6f, 2, m_EnemyConfigs[1].texture }; // Enemy 2: 2 damage to base
        m_EnemyConfigs[2] = { 15, 40, 3, 1.3f, 3, m_EnemyConfigs[2].texture }; // Enemy 3: 3 damage to base
        m_EnemyConfigs[3] = { 10, 60, 4, 1.0f, 5, m_EnemyConfigs[3].texture }; // Enemy 4: 5 damage to base
        m_TowerConfigs[0] = { 5, 3, 5, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 5, 7, 10, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 4, 11, 15, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 3, 15, 20, 192.0f, m_TowerConfigs[3].texture };
        for (int i = 0; i < 4; i++) {
            m_TowerCosts[i] = m_TowerConfigs[i].cost;
        }

        m_iStartingGold = 32;
        m_fGoldPerSecond = 1.1f; // 1.25 gold per second for Easy
        break;
    case Medium:
        m_EnemyConfigs[0] = { 45, 20, 2, 2.0f, 2, m_EnemyConfigs[0].texture }; // Enemy 1: 2 damage to base
        m_EnemyConfigs[1] = { 35, 40, 2, 1.8f, 3, m_EnemyConfigs[1].texture }; // Enemy 2: 3 damage to base
        m_EnemyConfigs[2] = { 25, 80, 4, 1.5f, 5, m_EnemyConfigs[2].texture }; // Enemy 3: 5 damage to base
        m_EnemyConfigs[3] = { 15, 100, 5, 1.2f, 8, m_EnemyConfigs[3].texture }; // Enemy 4: 8 damage to base
        m_TowerConfigs[0] = { 6, 4, 6, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 5, 9, 10, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 4, 14, 15, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 3, 19, 22, 192.0f, m_TowerConfigs[3].texture };
        for (int i = 0; i < 4; i++) {
            m_TowerCosts[i] = m_TowerConfigs[i].cost;
        }
        m_iStartingGold = 38;
        m_fGoldPerSecond = 1.2f; // 5 gold per second for Medium
        break;
    case Hard:
        m_EnemyConfigs[0] = { 60, 30, 2, 2.2f, 3, m_EnemyConfigs[0].texture }; // Enemy 1: 3 damage to base
        m_EnemyConfigs[1] = { 45, 60, 3, 2.0f, 5, m_EnemyConfigs[1].texture }; // Enemy 2: 5 damage to base
        m_EnemyConfigs[2] = { 35, 100, 5, 1.7f, 8, m_EnemyConfigs[2].texture }; // Enemy 3: 8 damage to base
        m_EnemyConfigs[3] = { 20, 130, 6, 1.4f, 12, m_EnemyConfigs[3].texture }; // Enemy 4: 12 damage to base
        m_TowerConfigs[0] = { 6, 5, 7, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 6, 11, 14, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 5, 17, 21, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 4, 23, 28, 192.0f, m_TowerConfigs[3].texture };
        for (int i = 0; i < 4; i++) {
            m_TowerCosts[i] = m_TowerConfigs[i].cost;
        }
        m_iStartingGold = 46;
        m_fGoldPerSecond = 1.3f; // 10 gold per second for Hard
        break;
    case Extremely:
        m_EnemyConfigs[0] = { 75, 40, 3, 2.5f, 5, m_EnemyConfigs[0].texture }; // Enemy 1: 5 damage to base
        m_EnemyConfigs[1] = { 55, 60, 3, 2.2f, 8, m_EnemyConfigs[1].texture }; // Enemy 2: 8 damage to base
        m_EnemyConfigs[2] = { 40, 130, 6, 1.9f, 12, m_EnemyConfigs[2].texture }; // Enemy 3: 12 damage to base
        m_EnemyConfigs[3] = { 30, 175, 7, 1.6f, 20, m_EnemyConfigs[3].texture }; // Enemy 4: 20 damage to base
        m_TowerConfigs[0] = { 6, 6, 8, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 6, 13, 16, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 5, 20, 24, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 5, 27, 40, 192.0f, m_TowerConfigs[3].texture };
        for (int i = 0; i < 4; i++) {
            m_TowerCosts[i] = m_TowerConfigs[i].cost;
        }
        m_iStartingGold = 54;
        m_fGoldPerSecond = 1.4f; // 20 gold per second for VeryHard
        break;
    }

    for (int i = 0; i < 4; ++i) {
        m_EnemyTemplates[i].SetHealth(m_EnemyConfigs[i].health);
        m_EnemyTemplates[i].SetGoldReward(m_EnemyConfigs[i].goldReward);
    }
}

void Game::LoadMap(int map) {
    std::string levelFileName = "maps/Map " + std::to_string(map) + ".txt";
    std::string mapImageFile = "image/maps/Map " + std::to_string(map) + ".png";


    try {
        m_MapGrid.loadMapDataFromFile(levelFileName);
        m_Map.Initialize(mapImageFile);
        m_Map.ConstructPath(m_MapGrid);

        const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
        for (int y = 0; y < m_MapGrid.getHeight(); ++y) {
            for (int x = 0; x < m_MapGrid.getWidth(); ++x) {
                if (nodeMatrix[y][x] == MapGrid::TileType::End) {
                    m_BaseGridCoords = sf::Vector2i(x, y);
                }
            }
        }

        sf::Vector2f basePos(m_BaseGridCoords.x * 64.f + 32.f, m_BaseGridCoords.y * 64.f + 32.f);

        switch (m_iCurrentMap) {
        case 1:
            m_UIManager.SetHealthBarPosition(sf::Vector2f(basePos.x - 100.f, basePos.y - 130.f));
            break;
        case 2:
            m_UIManager.SetHealthBarPosition(sf::Vector2f(basePos.x - 180.f, basePos.y - 130.f));
            break;
        case 3:
            m_UIManager.SetHealthBarPosition(sf::Vector2f(basePos.x - 100.f, basePos.y + 10.f));
            break;
        case 4:
            m_UIManager.SetHealthBarPosition(sf::Vector2f(basePos.x - 100.f, basePos.y - 130.f));
            break;
        }


        m_iPlayerHealth = 100;
        m_UIManager.UpdateHealthBar(m_iPlayerHealth, m_iPlayerHealth);


        std::cout << "Map " << map << " loaded successfully from " << levelFileName << std::endl;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error loading map " << map << ": " << e.what() << std::endl;
        ReturnToMenu();
    }
    catch (...) {
        std::cerr << "An unknown error occurred loading map " << map << "." << std::endl;
        ReturnToMenu();
    }
}


void Game::UpdateTitleScreen() {
    m_TitleScreen.Update(m_deltaTime.asSeconds());

    if (m_TitleScreen.ShouldExit()) {
        m_eGameMode = Play;
        m_GameModeText.setString("Menu Mode");

        // Thêm dòng này để chuyển đến menu profile
        m_MenuManager.SetMenuState(MenuManager::MenuState::ProfileMenu);
    }
}

void Game::HandleTitleScreenInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed ||
        (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)) {
        m_eGameMode = Play;
        m_GameModeText.setString("Play Mode");
        m_MenuManager.SetMenuState(MenuManager::MenuState::ProfileMenu);
    }
    m_TitleScreen.HandleInput(event);
}


void Game::UpdateEnemySpawning() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
    sf::Vector2i spawnCoords(-1, -1);

    // Find spawn coordinates
    for (int y = 0; y < m_MapGrid.getHeight(); ++y) {
        for (int x = 0; x < m_MapGrid.getWidth(); ++x) {
            if (nodeMatrix[y][x] == MapGrid::TileType::Spawn) {
                spawnCoords = sf::Vector2i(x, y);
                break;
            }
        }
        if (spawnCoords != sf::Vector2i(-1, -1)) break;
    }

    const std::vector<Map::Path>& paths = m_Map.GetPaths();
    if (spawnCoords != sf::Vector2i(-1, -1) && !paths.empty() && m_enemies.size() < m_iMaxEnemies) {
        static float fSpawnTimer = 0.0f;
        float fSpawnRate = m_fDifficulty;
        fSpawnTimer += m_deltaTime.asSeconds() * fSpawnRate;
        if (fSpawnTimer > 3.0f) {
            int totalEnemies = 0;
            for (const auto& config : m_EnemyConfigs) {
                totalEnemies += config.count;
            }
            if (m_enemies.size() < totalEnemies) {
                int type = rand() % 4;
                if (m_spawnedEnemies[type] < m_EnemyConfigs[type].count) {
                    Entity& newEnemy = m_enemies.emplace_back(m_EnemyTemplates[type]);
                    newEnemy.SetPosition(sf::Vector2f(spawnCoords.x * 64.0f + 32.0f, spawnCoords.y * 64.0f + 32.0f));
                    newEnemy.SetVelocity(sf::Vector2f(0, 0));
                    newEnemy.SetPathIndex(rand() % paths.size());
                    newEnemy.SetGoldReward(m_EnemyConfigs[type].goldReward);

                    // Scale enemy health with difficulty
                    int enemyHealth = m_EnemyConfigs[type].health;
                    newEnemy.SetMaxHealth(enemyHealth);
                    newEnemy.SetCurrentHealth(enemyHealth);
                    newEnemy.ShowHealthBar(true);
                    newEnemy.SetHealthBarSize(50.f, 6.f);
                    newEnemy.SetHealthBarOffset(sf::Vector2f(0.f, -45.f));
                    newEnemy.UpdateHealthBarPosition();

                    m_spawnedEnemies[type]++;
                }
                fSpawnTimer = 0.0f;
            }
        }
    }
    sf::Vector2i endCoords(-1, -1);
    for (int y = 0; y < m_MapGrid.getHeight(); ++y) {
        for (int x = 0; x < m_MapGrid.getWidth(); ++x) {
            if (nodeMatrix[y][x] == MapGrid::TileType::End) {
                endCoords = sf::Vector2i(x, y);
                break;
            }
        }
        if (endCoords != sf::Vector2i(-1, -1)) break;
    }

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Entity& rEnemy = m_enemies[i];
        const Map::Path& path = paths[rEnemy.GetPathIndex()];
        int type = rEnemy.GetType() - 1;

        const Map::PathTile* pClosestTile = nullptr;
        float fClosestDistance = std::numeric_limits<float>::max();

        for (const Map::PathTile& tile : path) {
            sf::Vector2f tilePos(tile.coords.x * 64.0f + 32.0f, tile.coords.y * 64.0f + 32.0f);
            sf::Vector2f vEnemyToTile = tilePos - rEnemy.GetPosition();
            float fDistance = MathHelpers::flength(vEnemyToTile);

            if (fDistance < fClosestDistance) {
                fClosestDistance = fDistance;
                pClosestTile = &tile;
            }
        }

        if (!pClosestTile) continue;
        sf::Vector2f nextTilePos(pClosestTile->nextCoords.x * 64.0f + 32.0f, pClosestTile->nextCoords.y * 64.0f + 32.0f);

        if (endCoords != sf::Vector2i(-1, -1) && pClosestTile->nextCoords == endCoords) {
            if (fClosestDistance < 32.0f) {
                int damageToBase = m_EnemyConfigs[type].damageToBase;
                m_iPlayerHealth -= damageToBase;
                DamageTextManager::getInstanceNonConst().AddDamageText(damageToBase, sf::Vector2f(endCoords.x * 64.0f + 32.0f, endCoords.y * 64.0f + 32.0f));
                m_enemies.erase(m_enemies.begin() + i);
                m_killedEnemies[type]++; // Tăng số quái đã tiêu diệt khi đến đích
                m_fDifficulty *= 0.9f;
                SoundManager::getInstance().PlayEnemyDeathSound();
                continue;
            }
        }

        float fEnemySpeed = m_EnemyConfigs[type].speed * 100.0f;
        sf::Vector2f vEnemyToNextTile = nextTilePos - rEnemy.GetPosition();
        vEnemyToNextTile = MathHelpers::normalize(vEnemyToNextTile);
        rEnemy.SetVelocity(vEnemyToNextTile * fEnemySpeed);

        // Rotate enemy sprite to face movement direction
        if (MathHelpers::flength(rEnemy.GetVelocity()) > 0.0f) {
            float fAngle = MathHelpers::Angle(rEnemy.GetVelocity()) + 90.0f;
            rEnemy.GetSpriteNonConst().setRotation(fAngle);
        }
    }
}

void Game::UpdateEnemyMovement() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
    sf::Vector2i endCoords(-1, -1);

    // Find end coordinates
    for (int y = 0; y < m_MapGrid.getHeight(); ++y) {
        for (int x = 0; x < m_MapGrid.getWidth(); ++x) {
            if (nodeMatrix[y][x] == MapGrid::TileType::End) {
                endCoords = sf::Vector2i(x, y);
                break;
            }
        }
        if (endCoords != sf::Vector2i(-1, -1)) break;
    }

    const std::vector<Map::Path>& paths = m_Map.GetPaths();

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Entity& rEnemy = m_enemies[i];
        rEnemy.UpdateHealthBarPosition();

        const Map::Path& path = paths[rEnemy.GetPathIndex()];
        const Map::PathTile* pClosestTile = FindClosestPathTile(rEnemy, path);

        if (!pClosestTile) continue;

        sf::Vector2f nextTilePos(pClosestTile->nextCoords.x * 64.0f + 32.0f,
            pClosestTile->nextCoords.y * 64.0f + 32.0f);

        // Check if enemy reached end
        if (endCoords != sf::Vector2i(-1, -1) && pClosestTile->nextCoords == endCoords) {
            sf::Vector2f vEnemyToEnd = sf::Vector2f(endCoords.x * 64.0f + 32.0f, endCoords.y * 64.0f + 32.0f) - rEnemy.GetPosition();
            float fDistanceToEnd = MathHelpers::flength(vEnemyToEnd);

            if (fDistanceToEnd < 90.0f) {
                int type = rEnemy.GetType() - 1;
                int damageToBase = m_EnemyConfigs[type].damageToBase;
                m_iPlayerHealth -= damageToBase;
                DamageTextManager::getInstanceNonConst().AddDamageText(damageToBase, sf::Vector2f(endCoords.x * 64.0f + 32.0f, endCoords.y * 64.0f + 32.0f));
                m_enemies.erase(m_enemies.begin() + i);
                m_killedEnemies[type]++;
                m_fDifficulty *= 0.9f;
                m_UIManager.UpdateHealthBar(m_iPlayerHealth, 100);
                SoundManager::getInstance().PlayEnemyDeathSound();
                continue;
            }
        }

        // Move enemy towards next tile
        float fEnemySpeed = m_EnemyConfigs[rEnemy.GetType() - 1].speed * 100.0f;
        sf::Vector2f vEnemyToNextTile = nextTilePos - rEnemy.GetPosition();
        vEnemyToNextTile = MathHelpers::normalize(vEnemyToNextTile);
        rEnemy.SetVelocity(vEnemyToNextTile * fEnemySpeed);

        // Rotate enemy sprite to face movement direction
        if (MathHelpers::flength(rEnemy.GetVelocity()) > 0.0f) {
            float fAngle = MathHelpers::Angle(rEnemy.GetVelocity()) + 90.0f;
            rEnemy.GetSpriteNonConst().setRotation(fAngle);
        }
    }
}

const Map::PathTile* Game::FindClosestPathTile(const Entity& enemy, const Map::Path& path) {
    const Map::PathTile* pClosestTile = nullptr;
    float fClosestDistance = std::numeric_limits<float>::max();

    for (const Map::PathTile& tile : path) {
        sf::Vector2f tilePos(tile.coords.x * 64.0f + 32.0f, tile.coords.y * 64.0f + 32.0f);
        sf::Vector2f vEnemyToTile = tilePos - enemy.GetPosition();
        float fDistance = MathHelpers::flength(vEnemyToTile);

        if (fDistance < fClosestDistance) {
            fClosestDistance = fDistance;
            pClosestTile = &tile;
        }
    }

    return pClosestTile;
}


void Game::UpdateGoldCalculation() {
    static float fAutoGoldTimer = 0.0f;
    fAutoGoldTimer += m_deltaTime.asSeconds();
    if (fAutoGoldTimer >= 1.0f) {
        int goldToAdd = static_cast<int>(m_fGoldPerSecond);
        AddGold(goldToAdd);
        fAutoGoldTimer -= 1.0f; // Reset timer, keeping remainder
    }
}

void Game::CheckGameOver() {
    if (m_iPlayerHealth <= 0 && !m_bGameOverTriggered) {
        m_bGameOverTriggered = true;
        m_MenuManager.ShowGameOverMenu();

        if (!m_bGameOverSoundPlayed) {
            SoundManager::getInstance().StopBackgroundMusic();
            SoundManager::getInstance().PlayGameOverSound();
            m_bGameOverSoundPlayed = true;
        }

        ClearSavedGameData();
        m_MenuManager.SaveProfilesToFile();
        return;
    }
}

void Game::UpdatePlay() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    m_fTimeInPlayMode += m_deltaTime.asSeconds();
    m_fDifficulty += m_deltaTime.asSeconds() / 10.0f;

    // Update UI
    if (m_iPlayerHealth <= 0) {
        m_UIManager.Update(m_deltaTime.asSeconds());
        m_UIManager.UpdateHealthBar(m_iPlayerHealth, 100);
        CheckGameOver();
        return;
    }

    if (CheckVictoryConditions() && !m_bVictoryTriggered) {
        m_bVictoryTriggered = true;
        m_MenuManager.ShowGameWonMenu();

        if (!m_bGameWonSoundPlayed) {
            SoundManager::getInstance().StopBackgroundMusic();
            SoundManager::getInstance().PlayRandomGameWonSound();
            m_bGameWonSoundPlayed = true;
        }
        HandleVictory();
        return;
    }

    m_UIManager.Update(m_deltaTime.asSeconds());
    m_UIManager.UpdateHealthBar(m_iPlayerHealth, 100);

    DamageTextManager::getInstanceNonConst().Update(m_deltaTime);

    m_TowerSelectionPanel.Update(
        m_deltaTime.asSeconds(),
        static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window)),
        m_iPlayerGold,
        m_TowerCosts
    );

    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window));
    UpdateRangeVisualization(mousePos);
    UpdateTowerStats();

    UpdateEnemySpawning();
    UpdateTower();
    UpdateProjectiles();
    UpdateEnemyMovement();

    // Update gold per second calculation
    UpdateGoldCalculation();

    UpdatePhysics();
    CheckForDeletionRequest();

    //Auto save game
    static float autoSaveTimer = 0.0f;
    autoSaveTimer += m_deltaTime.asSeconds();
    if (autoSaveTimer >= 30.0f) {
        SaveCurrentGameState();
        autoSaveTimer = 0.0f;
    }
}


Entity* Game::FindClosestEnemyInRange(const Entity& tower) {
    Entity* pClosestEnemy = nullptr;
    float fClosestDistance = m_fTowerRange; // Use tower range as max distance

    for (Entity& enemy : m_enemies) {
        sf::Vector2f vTowerToEnemy = enemy.GetPosition() - tower.GetPosition();
        float fDistance = MathHelpers::flength(vTowerToEnemy);

        if (fDistance < fClosestDistance) {
            fClosestDistance = fDistance;
            pClosestEnemy = &enemy;
        }
    }

    return pClosestEnemy;
}

void Game::UpdateTower() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    for (Entity& tower : m_Towers) {
        tower.m_fAttackTimer -= m_deltaTime.asSeconds();
        if (tower.m_fAttackTimer > 0.0f) continue;

        Entity* pClosestEnemy = nullptr;
        float fClosestDistance = std::numeric_limits<float>::max();
        int towerType = tower.GetType() - 1;
        float towerRange = m_TowerConfigs[towerType].range; // Get the tower's range

        for (Entity& enemy : m_enemies) {
            sf::Vector2f vTowerToEnemy = enemy.GetPosition() - tower.GetPosition();
            float fDistance = MathHelpers::flength(vTowerToEnemy);
            if (fDistance < fClosestDistance && fDistance <= towerRange) { // Check if enemy is within range
                fClosestDistance = fDistance;
                pClosestEnemy = &enemy;
            }
        }

        if (!pClosestEnemy) {
            continue;
        }

        sf::Vector2f vTowerToEnemy = pClosestEnemy->GetPosition() - tower.GetPosition();
        float fAngle = MathHelpers::Angle(vTowerToEnemy) + 180.0f;
        tower.GetSpriteNonConst().setRotation(fAngle);

        Entity& newProjectile = m_projectiles.emplace_back(m_BulletTemplates[towerType]);
        newProjectile.SetPosition(tower.GetPosition());
        
        float speedMultiplier = 1.0f + towerType * 0.5f;

        vTowerToEnemy = MathHelpers::normalize(vTowerToEnemy);
        float baseSpeed = 200.0f;
        float finalSpeed = baseSpeed * speedMultiplier;
        newProjectile.SetVelocity(vTowerToEnemy * finalSpeed);

        newProjectile.SetType(towerType + 1);
        newProjectile.SetDamage(m_TowerConfigs[towerType].damage);

        SoundManager::getInstance().PlayHitSound();
        tower.m_fAttackTimer = 1.0f / speedMultiplier;
    }
}


void Game::UpdateProjectiles() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    for (Entity& projectile : m_projectiles) { // Có thể đổi tên m_axes thành m_projectiles sau
        projectile.m_fBulletTimer -= m_deltaTime.asSeconds();

        int projectileType = projectile.GetType();

        // **PHẦN MỚI: AUTO-AIM - Tìm mục tiêu gần nhất**
        Entity* pClosestEnemy = nullptr;
        float fClosestDistance = std::numeric_limits<float>::max();

        for (Entity& enemy : m_enemies) {
            sf::Vector2f vProjectileToEnemy = enemy.GetPosition() - projectile.GetPosition();
            float fDistance = MathHelpers::flength(vProjectileToEnemy);
            if (fDistance < fClosestDistance) {
                fClosestDistance = fDistance;
                pClosestEnemy = &enemy;
            }
        }

        // **PHẦN MỚI: Điều chỉnh hướng bay theo mục tiêu gần nhất**
        if (pClosestEnemy) {
            sf::Vector2f vProjectileToEnemy = pClosestEnemy->GetPosition() - projectile.GetPosition();
            sf::Vector2f vCurrentVelocity = projectile.GetVelocity();
            float fCurrentSpeed = MathHelpers::flength(vCurrentVelocity);

            // Tính toán hướng mới
            sf::Vector2f vNewDirection = MathHelpers::normalize(vProjectileToEnemy);

            // Độ linh hoạt xoay (0.0f = không xoay, 1.0f = xoay ngay lập tức)
            float fTurnRate = 0.1f; // Có thể điều chỉnh theo từng loại đạn

            // Phân loại theo type của bullet để có độ linh hoạt khác nhau
            switch (projectileType) {
            case 1: // Axe/Dao ném - xoay vòng, ít linh hoạt
                fTurnRate = 0.05f;
                break;
            case 2: // Shuriken - xoay vòng, linh hoạt hơn
                fTurnRate = 0.08f;
                break;
            case 3: // Tên lửa nhỏ - rất linh hoạt
                fTurnRate = 0.15f;
                break;
            case 4: // Tên lửa lớn - linh hoạt vừa phải
                fTurnRate = 0.12f;
                break;
            default:
                fTurnRate = 0.1f;
                break;
            }

            // Lerp (Linear Interpolation) để làm mượt việc xoay
            sf::Vector2f vCurrentDirection = MathHelpers::normalize(vCurrentVelocity);
            sf::Vector2f vLerpedDirection = vCurrentDirection + (vNewDirection - vCurrentDirection) * fTurnRate;
            vLerpedDirection = MathHelpers::normalize(vLerpedDirection);

            // Cập nhật velocity với hướng mới nhưng giữ nguyên tốc độ
            projectile.SetVelocity(vLerpedDirection * fCurrentSpeed);
        }

        // Phần xoay sprite (giữ nguyên logic cũ)
        switch (projectileType) {
        case 1: // Axe/Dao ném - xoay vòng
        {
            const float fRotationSpeed = 360.0f; // degrees per second
            projectile.GetSpriteNonConst().rotate(fRotationSpeed * m_deltaTime.asSeconds());
            break;
        }
        case 2: // Shuriken - xoay vòng
        {
            const float fRotationSpeed = 540.0f; 
            projectile.GetSpriteNonConst().rotate(fRotationSpeed * m_deltaTime.asSeconds());
            break;;
        }

        case 3: // Tên lửa nhỏ - xoay theo hướng bay
        {
            if (MathHelpers::flength(projectile.GetVelocity()) > 0.0f) {
                float fAngle = MathHelpers::Angle(projectile.GetVelocity()) + 90.0f;
                projectile.GetSpriteNonConst().setRotation(fAngle);
            }
            break;
        }
        case 4: // Tên lửa lớn - xoay theo hướng bay
        {
            if (MathHelpers::flength(projectile.GetVelocity()) > 0.0f) {
                float fAngle = MathHelpers::Angle(projectile.GetVelocity()) + 90.0f;
                projectile.GetSpriteNonConst().setRotation(fAngle);
            }
            break;
        }

        default:
            // Không xoay cho các loại khác
            break;
        }

        // Kiểm tra thời gian sống của đạn
        if (projectile.m_fBulletTimer <= 0.0f) {
            projectile.RequestDeletion();
        }
    }
}

void Game::CheckForDeletionRequest() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    for (int i = m_projectiles.size() - 1; i >= 0; i--) {
        Entity& projectile = m_projectiles[i];
        if (projectile.IsDeletionRequested()) {
            m_projectiles.erase(m_projectiles.begin() + i);
        }
    }

    for (int i = m_enemies.size() - 1; i >= 0; i--) {
        Entity& enemy = m_enemies[i];
        if (enemy.IsDeletionRequested()) {
            int type = enemy.GetType() - 1;
            AddGold(enemy.GetGoldReward());
            m_killedEnemies[type]++;
            m_enemies.erase(m_enemies.begin() + i);
            SoundManager::getInstance().PlayEnemyDeathSound();
        }
    }
}

void Game::UpdatePhysics() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    const float fMaxDeltaTime = 0.1f;
    const float fDeltaTime = std::min(m_deltaTime.asSeconds(), fMaxDeltaTime);

    std::vector<Entity*> AllEntities;

    for (Entity& tower : m_Towers) {
        AllEntities.push_back(&tower);
    }

    for (Entity& enemy : m_enemies) {
        AllEntities.push_back(&enemy);
    }

    for (Entity& bullet : m_projectiles) {
        AllEntities.push_back(&bullet);
    }

    for (Entity* entity : AllEntities) {
        entity->GetPhysicsDataNonConst().ClearCollisions();
    }

    for (Entity* entity : AllEntities) {
        if (entity->GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic) {
            entity->move(entity->GetPhysicsData().m_vVelocity * fDeltaTime + entity->GetPhysicsData().m_vImpulse);
            entity->GetPhysicsDataNonConst().ClearImpulse();

            for (Entity* otherEntity : AllEntities) {
                if (entity == otherEntity) continue;
                if (entity->shouldIgnoreEntityForPhysics(otherEntity)) continue;

                if (!entity->GetPhysicsDataNonConst().HasCollidedThisUpdate(otherEntity) && isColiding(*entity, *otherEntity)) {
                    entity->OnCollision(*otherEntity);
                    otherEntity->OnCollision(*entity);

                    entity->GetPhysicsDataNonConst().AddEntityCollision(otherEntity);
                    otherEntity->GetPhysicsDataNonConst().AddEntityCollision(entity);
                }
                ProcessCollision(*entity, *otherEntity);
            }
        }
    }
}

void Game::UpdateRangeVisualization(const sf::Vector2f& mousePos) {
    m_LastMousePosition = mousePos;

    // Case 1: Đã chọn tower type và đang hover để đặt
    if (m_selectedTowerIndex != -1) {
        // Chỉ hiển thị ghost tower và range preview khi đang đặt tower mới
        bool canPlace = CanPlaceTowerAtPosition(mousePos);

        // Snap to grid cho preview
        int gridX = static_cast<int>(mousePos.x / 64.0f);
        int gridY = static_cast<int>(mousePos.y / 64.0f);
        sf::Vector2f gridCenterPos(gridX * 64.0f + 32.0f, gridY * 64.0f + 32.0f);

        // Màu sắc tùy theo khả năng đặt tower
        sf::Color previewColor = (canPlace && m_TowerCounts[m_selectedTowerIndex] < m_TowerConfigs[m_selectedTowerIndex].maxCount) 
            ? sf::Color(0, 150, 255, 100) : sf::Color(255, 0, 0, 120);

        // Hiện range và ghost tower cho tower mới
        ShowTowerRange(gridCenterPos, m_TowerConfigs[m_selectedTowerIndex].range, previewColor);
        ShowGhostTower(gridCenterPos, m_selectedTowerIndex, previewColor);
    }
    // Case 2: Không đang chọn tower type để đặt
    else {
        HideGhostTower(); // Luôn ẩn ghost tower

        // Chỉ hiển thị range cho tower đã được click (không hiển thị khi hover)
        if (m_ClickedTowerIndex != -1 && m_ClickedTowerIndex < static_cast<int>(m_Towers.size())) {
            // Hiện range của tower đã click
            const Entity& clickedTower = m_Towers[m_ClickedTowerIndex];
            int towerType = clickedTower.GetType() - 1;
            ShowTowerRange(clickedTower.GetPosition(), m_TowerConfigs[towerType].range, sf::Color(0, 200, 255, 120));
        }
        else {
            // Không có tower được click, ẩn range
            HideRange();
        }
    }
}

bool Game::IsMouseOverExistingTower(const sf::Vector2f& mousePos, int& towerIndex) {
    for (size_t i = 0; i < m_Towers.size(); ++i) {
        sf::Vector2f towerPos = m_Towers[i].GetPosition();
        sf::Vector2f diff = mousePos - towerPos;
        float distance = MathHelpers::flength(diff);

        // Kiểm tra trong radius của tower sprite (32 pixels)
        if (distance <= 40.0f) { // Hơi lớn hơn sprite để dễ click
            towerIndex = static_cast<int>(i);
            return true;
        }
    }
    towerIndex = -1;
    return false;
}

int Game::FindHoveredTower(const sf::Vector2f& mousePos) {
    for (size_t i = 0; i < m_Towers.size(); ++i) {
        sf::Vector2f towerPos = m_Towers[i].GetPosition();
        sf::Vector2f diff = mousePos - towerPos;
        float distance = MathHelpers::flength(diff);

        if (distance <= 40.0f) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Game::ShowTowerRange(const sf::Vector2f& center, float radius, const sf::Color& color) {
    m_bShowRange = true;
    m_RangeCenter = center;
    m_CurrentRangeRadius = radius;
    m_RangeColor = color;

    // Update circle shape
    m_RangeIndicator.setRadius(radius);
    m_RangeIndicator.setOrigin(radius, radius);
    m_RangeIndicator.setPosition(center);
    m_RangeIndicator.setOutlineColor(color);

    // Màu fill trong suốt với alpha thấp
    sf::Color fillColor = color;
    fillColor.a = 30; // Rất trong suốt
    m_RangeIndicator.setFillColor(fillColor);
}

void Game::ShowGhostTower(const sf::Vector2f& position, int towerType, const sf::Color& tintColor) {
    if (towerType < 0 || towerType >= 4) return;

    m_bShowGhostTower = true;
    m_GhostTowerPosition = position;
    m_GhostTowerColor = tintColor;

    // Set texture từ tower template
    m_GhostTowerSprite.setTexture(m_towerTextures[towerType]);
    m_GhostTowerSprite.setPosition(position);

    // Tint color với alpha để tạo hiệu ứng ghost
    sf::Color ghostColor = tintColor;
    ghostColor.a = 150; // Semi-transparent
    m_GhostTowerSprite.setColor(ghostColor);
}

void Game::DrawRangeIndicator() {
    if (m_bShowRange) {
        m_Window.draw(m_RangeIndicator);
    }
}

void Game::HideGhostTower() {
    m_bShowGhostTower = false;
}

void Game::DrawGhostTower() {
    if (m_bShowGhostTower) {
        m_Window.draw(m_GhostTowerSprite);
    }
}
void Game::HideRange() {
    m_bShowRange = false;
}

void Game::ShowTowerStatsPanel(int towerIndex) {
    if (towerIndex < 0 || towerIndex >= static_cast<int>(m_Towers.size())) return;

    m_ShowTowerStats = true;
    const Entity& tower = m_Towers[towerIndex];
    int towerType = tower.GetType() - 1;

    sf::Vector2f towerPosition = tower.GetPosition();
    sf::Vector2f panelPosition;
    panelPosition.x = towerPosition.x - 300.0f;
    panelPosition.y = towerPosition.y - 180.0f;

    sf::Vector2u windowSize = m_Window.getSize();

    // Nếu panel bị tràn ra khỏi bên trái màn hình
    if (panelPosition.x < 0) {
        panelPosition.x = towerPosition.x + 70.0f; // Hiển thị bên phải tower thay vì bên trái
    }

    // Nếu panel bị tràn ra khỏi phía trên màn hình
    if (panelPosition.y < 0) {
        panelPosition.y = 10.0f; // Đặt gần phía trên màn hình
    }

    // Nếu panel bị tràn ra khỏi phía dưới màn hình
    if (panelPosition.y + 180.0f > windowSize.y) { // 180 = panel height
        panelPosition.y = windowSize.y - 190.0f; // Điều chỉnh để vừa màn hình
    }

    // Cập nhật vị trí panel
    m_StatsPanel.setPosition(panelPosition);

    // Cập nhật title
    m_StatsTitleText.setString("Tower " + std::to_string(towerType + 1));
    m_StatsTitleText.setCharacterSize(23);

    // Cập nhật position cho title
    sf::Vector2f titlePos = m_StatsPanel.getPosition();
    titlePos.x += 10.0f;
    titlePos.y += 10.0f;
    m_StatsTitleText.setPosition(titlePos);

    // Tính cooldown từ attack timer (giả sử 1.0f / speedMultiplier)
    float speedMultiplier = 1.0f + towerType * 0.5f;
    float cooldown = 1.0f / speedMultiplier;

    // Cập nhật content
    std::string content = "Damage: " + std::to_string(m_TowerConfigs[towerType].damage) + "\n";
    content += "Range: " + std::to_string(static_cast<int>(m_TowerConfigs[towerType].range)) + "\n";
    content += "Cooldown: " + std::to_string(cooldown).substr(0, 4) + "s\n";
    content += "Cost: " + std::to_string(m_TowerConfigs[towerType].cost) + " gold\n";
    content += "Type: " + std::string(towerType == 0 ? "Common" : towerType == 1 ? "Epic" : towerType == 2 ? "Legendary" : "Mythic");

    m_StatsContentText.setString(content);
    m_StatsContentText.setCharacterSize(23);

    // Cập nhật position cho content
    sf::Vector2f contentPos = titlePos;
    contentPos.y += 30.0f;
    m_StatsContentText.setPosition(contentPos);
}

void Game::HideTowerStatsPanel() {
    m_ShowTowerStats = false;
}

void Game::UpdateTowerStats() {
    //// Update stats panel position nếu cần (có thể điều chỉnh theo map)
    //sf::Vector2f panelPos(20.0f, 200.0f);

    //// Điều chỉnh position theo map nếu cần
    //switch (m_iCurrentMap) {
    //case 1:
    //    panelPos = sf::Vector2f(20.0f, 200.0f);
    //    break;
    //case 2:
    //    panelPos = sf::Vector2f(20.0f, 250.0f);
    //    break;
    //case 3:
    //    panelPos = sf::Vector2f(20.0f, 50.0f);
    //    break;
    //case 4:
    //    panelPos = sf::Vector2f(20.0f, 200.0f);
    //    break;
    //default:
    //    panelPos = sf::Vector2f(20.0f, 200.0f);
    //    break;
    //}

    //m_StatsPanel.setPosition(panelPos);

    //// Update text positions
    //if (m_ShowTowerStats) {
    //    sf::Vector2f titlePos = panelPos;
    //    titlePos.x += 10.0f;
    //    titlePos.y += 10.0f;
    //    m_StatsTitleText.setPosition(titlePos);

    //    sf::Vector2f contentPos = titlePos;
    //    contentPos.y += 30.0f;
    //    m_StatsContentText.setPosition(contentPos);
    //}
}

void Game::DrawTowerStats() {
    if (m_ShowTowerStats) {
        m_Window.draw(m_StatsPanel);
        m_Window.draw(m_StatsTitleText);
        m_Window.draw(m_StatsContentText);
    }
}

void Game::ProcessCollision(Entity& entity1, Entity& entity2) {
    assert(entity1.GetPhysicsData().m_eType != Entity::PhysicsData::Type::Static);
    if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
            const float fDistanceBeeenEntities = MathHelpers::flength(vEntity1ToEntity2);
            float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;

            if (fDistanceBeeenEntities < fSumOfRadii) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    entity1.move(-MathHelpers::normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBeeenEntities));
                }
                else {
                    const sf::Vector2f vEntity1ToEntity2Normalized = MathHelpers::normalize(vEntity1ToEntity2);
                    const sf::Vector2f vEntity1Movement = vEntity1ToEntity2Normalized * (fSumOfRadii - fDistanceBeeenEntities) * 0.5f;
                    entity1.move(-vEntity1Movement);
                    entity2.move(vEntity1Movement);
                }
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth / 2, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight / 2, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity1.GetPhysicsData().m_fRadius) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    entity1.move(-MathHelpers::normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistanceToClosestPoint));
                }
                else {
                    const sf::Vector2f vEntity1ToEntity2Normalized = MathHelpers::normalize(vCircleToClosestPoint);
                    const sf::Vector2f vEntity1Movement = vEntity1ToEntity2Normalized * (entity1.GetPhysicsData().m_fRadius - fDistanceToClosestPoint) * 0.5f;
                    entity1.move(-vEntity1Movement);
                    entity2.move(vEntity1Movement);
                }
            }
        }
    }
    else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
            float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

            float fOverlapX = (entity1.GetPhysicsData().m_fWidth + entity2.GetPhysicsData().m_fWidth) / 2 - fDistanceX;
            float fOverlapY = (entity1.GetPhysicsData().m_fHeight + entity2.GetPhysicsData().m_fHeight) / 2 - fDistanceY;
            if (fOverlapX > 0 && fOverlapY > 0) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (fOverlapX < fOverlapY) {
                    if (entity1.GetPosition().x < entity2.GetPosition().x) {
                        if (isEntity2Dynamic) {
                            entity1.move(sf::Vector2f(-fOverlapX / 2, 0));
                            entity2.move(sf::Vector2f(fOverlapX / 2, 0));
                        }
                        else {
                            entity1.move(sf::Vector2f(-fOverlapX, 0));
                        }
                    }
                    else {
                        if (isEntity2Dynamic) {
                            entity1.move(sf::Vector2f(fOverlapX / 2, 0));
                            entity2.move(sf::Vector2f(-fOverlapX / 2, 0));
                        }
                        else {
                            entity1.move(sf::Vector2f(fOverlapX, 0));
                        }
                    }
                }
                else {
                    if (entity1.GetPosition().y < entity2.GetPosition().y) {
                        if (isEntity2Dynamic) {
                            entity1.move(sf::Vector2f(0, -fOverlapY / 2));
                            entity2.move(sf::Vector2f(0, fOverlapY / 2));
                        }
                        else {
                            entity1.move(sf::Vector2f(0, -fOverlapY));
                        }
                    }
                    else {
                        if (isEntity2Dynamic) {
                            entity1.move(sf::Vector2f(0, fOverlapY / 2));
                            entity2.move(sf::Vector2f(0, -fOverlapY / 2));
                        }
                        else {
                            entity1.move(sf::Vector2f(0, fOverlapY));
                        }
                    }
                }
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth / 2, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight / 2, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity2.GetPhysicsData().m_fRadius) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    entity1.move(MathHelpers::normalize(vCircleToClosestPoint) * (entity2.GetPhysicsData().m_fRadius - fDistanceToClosestPoint));
                }
                else {
                    const sf::Vector2f vEntity2ToEntity1Normalized = MathHelpers::normalize(vCircleToClosestPoint);
                    const sf::Vector2f vEntity2Movement = vEntity2ToEntity1Normalized * (entity2.GetPhysicsData().m_fRadius - fDistanceToClosestPoint) * 0.5f;
                    entity1.move(vEntity2Movement);
                    entity2.move(-vEntity2Movement);
                }
            }
        }
    }
}

bool Game::isColiding(const Entity& entity1, const Entity& entity2) {
    if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
            const float fDistanceBeeenEntities = MathHelpers::flength(vEntity1ToEntity2);
            float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;

            if (fDistanceBeeenEntities < fSumOfRadii) {
                return true;
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth / 2, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight / 2, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity1.GetPhysicsData().m_fRadius) {
                return true;
            }
        }
    }
    else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
            float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

            float fOverlapX = (entity1.GetPhysicsData().m_fWidth + entity2.GetPhysicsData().m_fWidth) / 2 - fDistanceX;
            float fOverlapY = (entity1.GetPhysicsData().m_fHeight + entity2.GetPhysicsData().m_fHeight) / 2 - fDistanceY;
            if (fOverlapX > 0 && fOverlapY > 0) {
                return true;
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth / 2, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight / 2, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity2.GetPhysicsData().m_fRadius) {
                return true;
            }
        }
    }
    return false;
}

void Game::DrawPlay() {
    if (m_iPlayerHealth <= 0) {
        m_Window.draw(m_GameOverText);
    }

    for (const Entity& tower : m_Towers) {
        m_Window.draw(tower);
    }
    for (const Entity& enemy : m_enemies) {
        m_Window.draw(enemy);
    }
    for (const Entity& bullet : m_projectiles) {
        m_Window.draw(bullet);
    }

    DrawRangeIndicator();
    DrawGhostTower();
    DrawTowerStats();

    UpdatePlayerText();
    m_Window.draw(m_PlayerText);
}

void Game::Draw() {
    m_Window.clear();


    switch (m_eGameMode) {
    case TitleScreenMode:
        m_TitleScreen.Draw(m_Window);
        break;

    case Play:
        // Đã sửa đổi: Cấu trúc vẽ hợp lý hơn
        if (!m_MenuManager.IsInGamePlay()) {
            m_MenuManager.Draw(m_Window);
        }
        else {
            m_Map.Draw(m_Window);
            //m_Window.draw(m_GoldCoinSprite);
            m_Window.draw(m_GameModeText);
            m_Window.draw(m_PlayerText);
            if (m_iPlayerHealth > 0 || !m_MenuManager.IsGameOver() || !m_MenuManager.IsGameWon()) {
                m_Window.draw(m_PlayerText);
            }

            DrawPlay();
            m_UIManager.Draw(m_Window);
            m_TowerSelectionPanel.Draw(m_Window);

            if (m_MenuManager.IsGamePaused() || m_MenuManager.IsGameOver() || m_MenuManager.IsGameWon()) {
                m_MenuManager.Draw(m_Window);
            }
        }
        break;
    }

    m_Window.display();
}


void Game::HandleInput() {
    sf::Event event;

    while (m_Window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            HandleGameExit();
            return;
        }

        // Handle different game modes
        switch (m_eGameMode) {

        case TitleScreenMode:
            HandleTitleScreenInput(event);
            break;

        case Play:
            if (!m_MenuManager.IsInGamePlay() || m_MenuManager.IsGamePaused() || m_MenuManager.IsGameOver() || m_MenuManager.IsGameWon()) {
                m_MenuManager.HandleInput(event, m_Window);

                if (m_MenuManager.IsInGamePlay()) {
                    m_eGameMode = Play;
                    m_GameModeText.setString("Play Mode");
                }
            }
            else {
                HandleGameInput(event);
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    HandleKeyboardInput();
                }
            }
        }
            break;
    }

    if (m_MenuManager.IsInGamePlay() && !m_MenuManager.IsGamePaused() && !m_MenuManager.IsGameOver() && !m_MenuManager.IsGameWon()) {
        HandleKeyboardInput();
    }
}

void Game::HandleGameInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F4 && event.key.alt) {
            HandleGameExit();
            return;
        }

        if (event.key.code == sf::Keyboard::Escape) {
            m_MenuManager.TogglePauseMenu();
        }
        // Thêm phím tắt 1,2,3,4 để chọn tower với toggle functionality
        else if (event.key.code == sf::Keyboard::Num1) {
            if (m_selectedTowerIndex == 0) {
                // Đang chọn Tower 1, nhấn lại thì deselect
                m_selectedTowerIndex = -1;
                m_UIManager.SetWarningMessage("Tower 1 deselected", m_Window.getSize());
            }
            else {
                // Chọn Tower 1
                m_selectedTowerIndex = 0;
                m_towerCost = m_TowerConfigs[0].cost;
                m_UIManager.SetWarningMessage("Selected Tower 1", m_Window.getSize());
            }
            // Ẩn stats panel và range khi thao tác với tower placement
            HideTowerStatsPanel();
            m_ClickedTowerIndex = -1;
        }
        else if (event.key.code == sf::Keyboard::Num2) {
            if (m_selectedTowerIndex == 1) {
                // Đang chọn Tower 2, nhấn lại thì deselect
                m_selectedTowerIndex = -1;
                m_UIManager.SetWarningMessage("Tower 2 deselected", m_Window.getSize());
            }
            else {
                // Chọn Tower 2
                m_selectedTowerIndex = 1;
                m_towerCost = m_TowerConfigs[1].cost;
                m_UIManager.SetWarningMessage("Selected Tower 2", m_Window.getSize());
            }
            HideTowerStatsPanel();
            m_ClickedTowerIndex = -1;
        }
        else if (event.key.code == sf::Keyboard::Num3) {
            if (m_selectedTowerIndex == 2) {
                // Đang chọn Tower 3, nhấn lại thì deselect
                m_selectedTowerIndex = -1;
                m_UIManager.SetWarningMessage("Tower 3 deselected", m_Window.getSize());
            }
            else {
                // Chọn Tower 3
                m_selectedTowerIndex = 2;
                m_towerCost = m_TowerConfigs[2].cost;
                m_UIManager.SetWarningMessage("Selected Tower 3", m_Window.getSize());
            }
            HideTowerStatsPanel();
            m_ClickedTowerIndex = -1;
        }
        else if (event.key.code == sf::Keyboard::Num4) {
            if (m_selectedTowerIndex == 3) {
                // Đang chọn Tower 4, nhấn lại thì deselect
                m_selectedTowerIndex = -1;
                m_UIManager.SetWarningMessage("Tower 4 deselected", m_Window.getSize());
            }
            else {
                // Chọn Tower 4
                m_selectedTowerIndex = 3;
                m_towerCost = m_TowerConfigs[3].cost;
                m_UIManager.SetWarningMessage("Selected Tower 4", m_Window.getSize());
            }
            HideTowerStatsPanel();
            m_ClickedTowerIndex = -1;
        }
    }
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos = m_Window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        // Kiểm tra xem có click vào tower selection panel không
        int towerIndex = m_TowerSelectionPanel.GetClickedTowerIndex(mousePos);
        if (towerIndex != -1) {
            m_selectedTowerIndex = towerIndex;
            m_towerCost = m_TowerConfigs[m_selectedTowerIndex].cost;
            m_UIManager.SetWarningMessage
            ("Selected " + std::string(towerIndex == 0 ? "Tower 1" : towerIndex == 1 ? "Tower 2" : towerIndex == 2 ? "Tower 3" : "Tower 4"), m_Window.getSize());

            // Ẩn stats panel khi chọn tower mới để đặt
            HideTowerStatsPanel();
            m_ClickedTowerIndex = -1;
        }
        else {
            // Kiểm tra xem có click vào tower đã đặt không
            int clickedTowerIndex = -1;
            bool clickedOnTower = IsMouseOverExistingTower(mousePos, clickedTowerIndex);

            if (clickedOnTower) {
                // Click vào tower đã đặt - hiện thông tin tower và range
                if (m_ClickedTowerIndex == clickedTowerIndex) {
                    // Click vào cùng tower -> deselect
                    m_ClickedTowerIndex = -1;
                    HideTowerStatsPanel();
                    HideRange();
                    m_UIManager.SetWarningMessage("Tower deselected", m_Window.getSize());
                }
                else {
                    // Click vào tower khác -> select
                    m_ClickedTowerIndex = clickedTowerIndex;
                    ShowTowerStatsPanel(clickedTowerIndex);

                    const Entity& clickedTower = m_Towers[clickedTowerIndex];
                    int towerType = clickedTower.GetType() - 1;
                    m_UIManager.SetWarningMessage(
                        "Selected Tower " + std::to_string(towerType + 1),
                        m_Window.getSize()
                    );
                }

                // Reset tower placement selection
                m_selectedTowerIndex = -1;
            }
            else {
                // Click vào vùng trống
                if (m_selectedTowerIndex != -1) {
                    // Đang trong mode đặt tower
                    if (m_TowerCounts[m_selectedTowerIndex] >= m_TowerConfigs[m_selectedTowerIndex].maxCount) {
                        m_UIManager.SetWarningMessage("Maximum towers of this type reached! (" +
                            std::to_string(m_TowerConfigs[m_selectedTowerIndex].maxCount) + "/" +
                            std::to_string(m_TowerConfigs[m_selectedTowerIndex].maxCount) + ")",
                            m_Window.getSize());
                    }
                    else {
                        m_towerCost = m_TowerConfigs[m_selectedTowerIndex].cost;
                        if (m_iPlayerGold >= m_towerCost) {
                            if (CreateTowerAtPosition(mousePos, m_selectedTowerIndex)) {
                                m_iPlayerGold -= m_towerCost;
                                m_TowerCounts[m_selectedTowerIndex]++;
                                m_UIManager.SetWarningMessage("Tower placed! Gold remaining: " +
                                    std::to_string(m_iPlayerGold) + " (" +
                                    std::to_string(m_TowerCounts[m_selectedTowerIndex]) + "/" +
                                    std::to_string(m_TowerConfigs[m_selectedTowerIndex].maxCount) + ")",
                                    m_Window.getSize());

                                HideGhostTower();
                                HideRange();
                            }
                            else {
                                m_UIManager.SetWarningMessage("Cannot place tower at this location!", m_Window.getSize());
                            }
                        }
                        else {
                            m_UIManager.SetWarningMessage("Not enough gold to build a tower! Need: " +
                                std::to_string(m_towerCost) + ", Have: " + std::to_string(m_iPlayerGold),
                                m_Window.getSize());
                        }
                    }
                }
                else {
                    // Không đang đặt tower, deselect tower đã chọn
                    if (m_ClickedTowerIndex != -1) {
                        m_ClickedTowerIndex = -1;
                        HideTowerStatsPanel();
                        HideRange();
                        m_UIManager.SetWarningMessage("Tower deselected", m_Window.getSize());
                    }
                }
            }
        }
    }
    // Right click để deselect mọi thứ
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        m_selectedTowerIndex = -1;
        m_ClickedTowerIndex = -1;
        HideGhostTower();
        HideRange();
        HideTowerStatsPanel();
        m_UIManager.SetWarningMessage("Selection cleared", m_Window.getSize());
    }
}

void Game::HandleKeyboardInput() {
   
}

bool Game::HasSavedGameData() {
    if (!m_MenuManager.GetCurrentProfile()) return false;

    const MenuManager::PlayerProfile* profile = m_MenuManager.GetCurrentProfile();

    // Check if there's saved data for current map and difficulty
    return profile->HasSavedData(m_iCurrentMap, static_cast<MenuManager::Difficulty>(m_eDifficulty));
}

void Game::SaveCurrentGameState() {
    if (m_MenuManager.GetCurrentProfile()) {
        MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());

        // Get game state for current map and difficulty
        auto& gameState = profile->GetGameState(m_iCurrentMap, static_cast<MenuManager::Difficulty>(m_eDifficulty));

        // Save current game state
        gameState.savedGoldForState = m_iPlayerGold;
        gameState.playerHealth = m_iPlayerHealth;
        gameState.timeInPlayMode = m_fTimeInPlayMode;

        // Save tower and enemy counts
        gameState.towerCounts = m_TowerCounts;
        gameState.spawnedEnemies = m_spawnedEnemies;
        gameState.killedEnemies = m_killedEnemies;

        // Save towers state
        gameState.savedTowers.clear();
        for (const Entity& tower : m_Towers) {
            MenuManager::TowerData towerData;
            sf::Vector2f pos = tower.GetPosition();
            towerData.x = static_cast<int>(pos.x / 64.0f);
            towerData.y = static_cast<int>(pos.y / 64.0f);
            towerData.type = tower.GetType();
            towerData.level = 1;
            gameState.savedTowers.push_back(towerData);
        }

        // Update current map and difficulty in profile
        profile->map = m_iCurrentMap;
        profile->difficulty = static_cast<MenuManager::Difficulty>(m_eDifficulty);

        m_MenuManager.SaveProfilesToFile();
        std::cout << "Game saved for Map " << m_iCurrentMap << " Difficulty " << static_cast<int>(m_eDifficulty) << "!" << std::endl;
    }
}

void Game::StartGame(int map, MenuManager::Difficulty difficulty) {
    if (map <= 0) {
        std::cerr << "Invalid level selected: " << map << std::endl;
        ReturnToMenu();
        return;
    }

    m_MenuManager.SetCurrentMapAndDifficulty(map, difficulty);
    m_TowerSelectionPanel.SetCurrentMap(map);
    m_TowerSelectionPanel.UpdateInterfaceForMap(map);

    m_iCurrentMap = map;
    m_eDifficulty = static_cast<Game::Difficulty>(difficulty);

    InitializeDifficulty(m_eDifficulty);
    LoadMap(map);

    // Kiểm tra xem có game đã lưu không
        if (m_MenuManager.GetCurrentProfile() && HasSavedGameData()) {
            // Load game state từ profile
            LoadGameState();
            std::cout << "Loaded saved game state." << std::endl;
        }
        else {
            // Bắt đầu game mới
            ResetGameState();
            m_iPlayerHealth = 100;
            m_iPlayerGold = m_iStartingGold;
        }

    m_selectedTowerIndex = -1;
    m_towerCost = m_TowerConfigs[0].cost;

    m_eGameMode = Play;
    m_GameModeText.setString("Play Mode");

    SoundManager::getInstance().PauseBackgroundMusic();
    SoundManager::getInstance().ResumeBackgroundMusic();

    m_MenuManager.SetMenuState(MenuManager::MenuState::GamePlay);
    UpdatePlayerText();
}

void Game::UpdatePlayerText() {
    std::string difficultyStr;
    switch (m_eDifficulty) {
    case Easy: difficultyStr = "Easy"; break;
    case Medium: difficultyStr = "Medium"; break;
    case Hard: difficultyStr = "Hard"; break;
    case Extremely: difficultyStr = "Extrememly"; break;
    }

    // Tính tổng số quái còn lại
    int totalEnemiesRemaining = 0;
    for (int i = 0; i < 4; ++i) {
        totalEnemiesRemaining += m_EnemyConfigs[i].count - m_killedEnemies[i];
    }

    sf::Vector2u windowSize = m_Window.getSize();

    switch (m_iCurrentMap) {
    case 1:
    {
        // Lấy kích thước của text để tính toán vị trí chính xác
        sf::FloatRect textBounds = m_PlayerText.getLocalBounds();

        float margin = 20.0f; // Khoảng cách từ edge phải
        float posX = windowSize.x - textBounds.width - margin;
        float posY = 0.0f; 

        m_PlayerText.setPosition(posX, posY);
        m_PlayerText.setCharacterSize(35);
        m_PlayerText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    }
    break;

    case 2:
    {

        float margin = 20.0f; // Khoảng cách từ edge phải
        float posX = windowSize.x * 0.2;
        float posY = 0.0f;

        m_PlayerText.setPosition(posX, posY);
        m_PlayerText.setCharacterSize(28);
        m_PlayerText.setFillColor(sf::Color(144, 238, 144));
        break;
    }
    case 3:
    {
        sf::FloatRect textBounds = m_PlayerText.getLocalBounds();

        float margin = 80.0f;
        float posY = windowSize.y - textBounds.height - margin;
        float posX = 0;

        m_PlayerText.setPosition(posX, posY);
        m_PlayerText.setCharacterSize(35);
        m_PlayerText.setFillColor(sf::Color(173, 216, 230));
        break;
    }
    case 4:
    {
        sf::FloatRect textBounds = m_PlayerText.getLocalBounds();

        float margin = 20.0f;
        float posX = 0;
        float posY = 0;

        m_PlayerText.setPosition(posX, posY);
        m_PlayerText.setCharacterSize(35);
        m_PlayerText.setFillColor(sf::Color(255, 182, 193));
        break;
    }
    default:
        m_PlayerText.setPosition(sf::Vector2f(50.0f, 50.0f));
        m_PlayerText.setCharacterSize(20);
        m_PlayerText.setFillColor(sf::Color::White);
        break;
    }

    m_PlayerText.setString("Difficulty: " + difficultyStr +
        "\nPlayer's Gold: " + std::to_string(m_iPlayerGold) +
        "\nPlayer's Health: " + std::to_string(m_iPlayerHealth) +
        "\nGold Per Second: " + std::to_string(static_cast<int>(m_fGoldPerSecond)) +
        "\nEnemies Remaining: " + std::to_string(totalEnemiesRemaining));
}

bool Game::CheckVictoryConditions() {
    // Kiểm tra xem tất cả enemies đã bị tiêu diệt chưa
    bool allEnemiesDefeated = true;
    for (int i = 0; i < 4; ++i) {
        if (m_killedEnemies[i] < m_EnemyConfigs[i].count) {
            allEnemiesDefeated = false;
            break;
        }
    }

    // Kiểm tra không còn enemy nào trên map
    bool noEnemiesOnMap = m_enemies.empty();

    // Kiểm tra player còn sống
    bool playerAlive = m_iPlayerHealth > 0;

    return allEnemiesDefeated && noEnemiesOnMap && playerAlive;
}
void Game::ClearSavedGameData() {
    if (!m_MenuManager.GetCurrentProfile()) return;

    MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());

    // Clear saved data for current map and difficulty
    auto& gameState = profile->GetGameState(m_iCurrentMap, static_cast<MenuManager::Difficulty>(m_eDifficulty));

    gameState.savedTowers.clear();
    gameState.timeInPlayMode = 0.0f;
    gameState.towerCounts = { 0, 0, 0, 0 };
    gameState.spawnedEnemies = { 0, 0, 0, 0 };
    gameState.killedEnemies = { 0, 0, 0, 0 };
    gameState.savedGoldForState = 10; // Reset to default
    gameState.playerHealth = 100;

    std::cout << "Cleared saved data for Map " << m_iCurrentMap << " Difficulty " << static_cast<int>(m_eDifficulty) << std::endl;
}

void Game::HandleVictory() {
    if (!m_MenuManager.GetCurrentProfile()) return;


    // Đánh dấu map và difficulty đã hoàn thành
    OnGameCompleted();

    // Tính điểm thưởng
    /*int bonusGold = CalculateVictoryBonus();
    AddGold(bonusGold);*/

    // Lưu trạng thái thắng game
    MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());
    profile->won = true;
    profile->savedGold = m_iPlayerGold; // Lưu gold sau khi thắng

    // Xóa dữ liệu game đã lưu (vì đã hoàn thành)
    ClearSavedGameData();

    m_MenuManager.SaveProfilesToFile();
}

void Game::ResetGameState() {
    m_enemies.clear();
    m_projectiles.clear();
    m_Towers.clear();

    for (int i = 0; i < 4; ++i) {
        m_TowerCounts[i] = 0;
        m_spawnedEnemies[i] = 0;
        m_killedEnemies[i] = 0;
    }

    m_iGoldGainedThisUpdate = 0;
    m_fTimeInPlayMode = 0.0f;
    m_fDifficulty = 1.0f;
    m_fGoldPerSecond = 0.0f;
    m_fGoldPerSecondTimer = 0.0f;
    m_bGameRunning = true;
    m_bGameOverSoundPlayed = false;
    m_bGameWonSoundPlayed = false;
    m_bGameOverTriggered = false;

    m_GameModeText.setString("Play Mode");
    m_PlayerText.setString("Player Gold: " + std::to_string(m_iPlayerGold) + "\nHealth: " + std::to_string(m_iPlayerHealth));
}

void Game::ReturnToMenu() {
    m_MenuManager.SetMenuState(MenuManager::MenuState::MainMenu);
    m_GameModeText.setString("Menu Mode");

    SoundManager::getInstance().PauseBackgroundMusic();
    SoundManager::getInstance().ResumeBackgroundMusic();
}

void Game::HandleGameExit() {

    if (m_MenuManager.IsGuestProfile()) {
        m_MenuManager.DeleteGuestProfile();
    }
        
    if (m_MenuManager.GetCurrentProfile() && !m_MenuManager.IsGuestProfile()) {
        MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());

        if (m_MenuManager.IsInGamePlay() && m_iPlayerHealth > 0 &&
            !m_MenuManager.IsGameOver() && !m_MenuManager.IsGameWon()) {
            SaveCurrentGameState();
        }

        m_MenuManager.SaveProfilesToFile();
        std::cout << "Profile saved: " << profile->name << std::endl;
    }
    
    m_MenuManager.SaveSettingsToFile();

    // Cleanup sound manager
    SoundManager::getInstance().Cleanup();

    // Close window
    m_Window.close();

}


void Game::ExitGame() {
    HandleGameExit();
}

void Game::LoadGameState() {
    if (m_MenuManager.GetCurrentProfile()) {
        const MenuManager::PlayerProfile* profile = m_MenuManager.GetCurrentProfile();

        // Get game state for current map and difficulty
        const auto& gameState = profile->GetGameState(m_iCurrentMap, static_cast<MenuManager::Difficulty>(m_eDifficulty));

        // Load game state
        m_iPlayerGold = gameState.savedGoldForState;
        m_iPlayerHealth = gameState.playerHealth;
        m_fTimeInPlayMode = gameState.timeInPlayMode;

        // Load tower and enemy counts
        if (gameState.towerCounts.size() == 4) {
            m_TowerCounts = gameState.towerCounts;
        }
        if (gameState.spawnedEnemies.size() == 4) {
            m_spawnedEnemies = gameState.spawnedEnemies;
        }
        if (gameState.killedEnemies.size() == 4) {
            m_killedEnemies = gameState.killedEnemies;
        }

        // Load saved towers
        m_Towers.clear();
        for (const MenuManager::TowerData& towerData : gameState.savedTowers) {
            if (towerData.type >= 1 && towerData.type <= 4) {
                Entity newTower = m_TowerTemplates[towerData.type - 1];
                newTower.SetPosition(sf::Vector2f(towerData.x * 64.0f + 32.0f, towerData.y * 64.0f + 32.0f));
                newTower.SetType(towerData.type);
                m_Towers.push_back(newTower);
            }
        }

        std::cout << "Loaded game state for Map " << m_iCurrentMap << " Difficulty " << static_cast<int>(m_eDifficulty) << std::endl;
    }
}

void Game::OnGameCompleted() {
    if (m_MenuManager.GetCurrentProfile()) {
        MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());

        // Mark current map and difficulty as completed
        profile->SetMapCompleted(m_iCurrentMap, static_cast<MenuManager::Difficulty>(m_eDifficulty));

        m_MenuManager.SaveProfilesToFile();
        std::cout << "Game completed! Map " << m_iCurrentMap << " on difficulty " << static_cast<int>(m_eDifficulty) << " marked as completed." << std::endl;
    }
}

bool Game::CreateTowerAtPosition(const sf::Vector2f& pos, int towerType) {
    if (CanPlaceTowerAtPosition(pos)) {
        // Snap to grid
        int gridX = static_cast<int>(pos.x / 64.0f);
        int gridY = static_cast<int>(pos.y / 64.0f);
        sf::Vector2f gridCenterPos(gridX * 64.0f + 32.0f, gridY * 64.0f + 32.0f);

        Entity newTower = m_TowerTemplates[towerType];
        newTower.SetColor(sf::Color::White);
        newTower.SetPosition(gridCenterPos);

        newTower.SetType(towerType + 1);
        m_Towers.push_back(newTower);

        SoundManager::getInstance().PlayTowerPlaceSound();
        return true;
    }
    return false;
}

bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
    // Convert to grid coordinates
    int gridX = static_cast<int>(pos.x / 64.0f);
    int gridY = static_cast<int>(pos.y / 64.0f);

    // Check bounds
    if (gridX < 0 || gridX >= m_MapGrid.getWidth() ||
        gridY < 0 || gridY >= m_MapGrid.getHeight()) {
        return false;
    }

    // Check tile type
    const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
    if (nodeMatrix[gridY][gridX] != MapGrid::TileType::Aesthetic) {
        return false;
    }

    // Check collision with existing towers
    Entity towerAtPosition = m_TowerTemplates[0]; // Use first template for collision check
    towerAtPosition.SetPosition(sf::Vector2f(gridX * 64.0f + 32.0f, gridY * 64.0f + 32.0f));

    for (const Entity& tower : m_Towers) {
        if (isColiding(tower, towerAtPosition)) {
            return false;
        }
    }

    return true;
}

void Game::AddGold(int gold) {
    m_iPlayerGold += gold;
    m_iGoldGainedThisUpdate += gold;
}



void Game::SetMusicVolume(float volume) {
    SoundManager::getInstance().SetMusicVolume(volume);
}

void Game::SetSoundVolume(float volume) {
    SoundManager::getInstance().SetSoundVolume(volume);
}