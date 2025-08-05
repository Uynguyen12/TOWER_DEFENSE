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
    : m_Window(sf::VideoMode({ 1920, 1040 }), "SFML window")
    , m_eGameMode(Play)
    , m_eDifficulty(Easy)
    , m_iPlayerHealth(100) // Set initial player health to 100
    , m_iPlayerGold(50)
    , m_iStartingGold(0)
    , m_fTimeInPlayMode(0.0f)
    , m_fDifficulty(1.0f)
    , m_iGoldGainedThisUpdate(0)
    , m_fGoldPerSecond(0.0f)
    , m_fGoldPerSecondTimer(0.0f)
    , m_bGameRunning(true)
    , m_bGameOverSoundPlayed(false)
    , m_iCurrentLevel(0)
    , m_bShowTowerSelection(false)
{
    // Initialize MenuManager first
    m_MenuManager.Initialize(m_Window);

    // Initialize Font
    if (!m_Font.loadFromFile("Fonts/Kreon-Medium.ttf")) {
        throw std::runtime_error("Failed to load font from 'Fonts/Kreon-Medium.ttf'");
    }

    // Initialize UI Text
    m_GameModeText.setFont(m_Font);
    m_GameModeText.setPosition(sf::Vector2f(1000, 200));
    m_GameModeText.setString("Play Mode");

    m_PlayerText.setPosition(sf::Vector2f(1500, 100));
    m_PlayerText.setFont(m_Font);

    m_GameOverText.setPosition(sf::Vector2f(1080, 800));
    m_GameOverText.setString("GAME OVERRR");
    m_GameOverText.setFont(m_Font);
    m_GameOverText.setCharacterSize(100);

    // Initialize SoundManager
    SoundManager::getInstance().Initialize();

    // Set up MenuManager callbacks
    m_MenuManager.SetExitCallback([this]() { this->ExitGame(); });
    m_MenuManager.SetStartGameCallback([this](int level, MenuManager::Difficulty difficulty) {
        this->StartGame(level, static_cast<Difficulty>(difficulty));
        });

    // Initialize tower and enemy configs
    m_TowerConfigs.resize(4);
    m_EnemyConfigs.resize(4);
    m_spawnedEnemies.resize(4, 0); // Khởi tạo số quái đã sinh
    m_killedEnemies.resize(4, 0);  // Khởi tạo số quái đã tiêu diệt
    for (int i = 0; i < 4; ++i) {
        m_TowerTemplates[i] = Entity(Entity::PhysicsData::Type::Static, i + 1);
        m_EnemyTemplates[i] = Entity(Entity::PhysicsData::Type::Dynamic, i + 1);
        m_BulletTemplates[i] = Entity(Entity::PhysicsData::Type::Dynamic, i + 1);
        m_TowerCounts[i] = 0;
    }

    // Load textures into separate sf::Texture objects first
    for (int i = 0; i < 4; ++i) {
        if (!m_towerTextures[i].loadFromFile("image/sprite/Tower" + std::to_string(i + 1) + ".png")) {
            throw std::runtime_error("Failed to load tower texture from 'image/sprite/Tower" + std::to_string(i + 1) + ".png'");
        }
        if (!m_enemyTextures[i].loadFromFile("image/sprite/Enemy" + std::to_string(i + 1) + ".png")) {
            throw std::runtime_error("Failed to load enemy texture from 'image/sprite/Enemy" + std::to_string(i + 1) + ".png'");
        }
        if (!m_bulletTextures[i].loadFromFile("image/sprite/Bullet" + std::to_string(i + 1) + ".png")) {
            throw std::runtime_error("Failed to load bullet texture from 'image/sprite/Bullet" + std::to_string(i + 1) + ".png'");
        }
    }

    // Assign textures to configs and templates
    for (int i = 0; i < 4; ++i) {
        m_TowerConfigs[i].texture = m_towerTextures[i];
        m_EnemyConfigs[i].texture = m_enemyTextures[i];
        m_TowerTemplates[i].SetTexture(m_TowerConfigs[i].texture);
        m_TowerTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_TowerTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_TowerTemplates[i].setCirclePhysics(32.f);
        m_TowerTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Tower);

        m_EnemyTemplates[i].SetTexture(m_EnemyConfigs[i].texture);
        m_EnemyTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_EnemyTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_EnemyTemplates[i].setCirclePhysics(32.f);
        m_EnemyTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Enemy);

        m_BulletTemplates[i].SetTexture(m_bulletTextures[i]);
        m_BulletTemplates[i].SetScale(sf::Vector2f(1, 1));
        m_BulletTemplates[i].SetOrigin(sf::Vector2f(32, 32));
        m_BulletTemplates[i].setCirclePhysics(32.f);
        m_BulletTemplates[i].GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Projectile);
        m_BulletTemplates[i].GetPhysicsDataNonConst().setLayersToIgnore(Entity::PhysicsData::Layer::Projectile | Entity::PhysicsData::Layer::Tower);
    }

    // Initialize tower selection menu
    m_TowerButtons.resize(4);
    m_TowerButtonTexts.resize(4);
    for (int i = 0; i < 4; ++i) {
        m_TowerButtons[i].setSize(sf::Vector2f(200, 50));
        m_TowerButtons[i].setFillColor(sf::Color(70, 70, 70, 200));
        m_TowerButtons[i].setOutlineThickness(2);
        m_TowerButtons[i].setOutlineColor(sf::Color::White);

        m_TowerButtonTexts[i].setFont(m_Font);
        m_TowerButtonTexts[i].setCharacterSize(20);
        m_TowerButtonTexts[i].setFillColor(sf::Color::White);
    }
}

Game::~Game() {
    SoundManager::getInstance().Cleanup();
}

void Game::run() {
    sf::Clock clock;
    while (m_Window.isOpen()) {
        m_deltaTime = clock.restart();
        HandleInput();
        if (!m_MenuManager.IsInGamePlay()) {
            m_MenuManager.Update(m_Window, m_deltaTime.asSeconds());
        }
        else {
            if (!m_MenuManager.IsGamePaused() && !m_bShowTowerSelection) {
                UpdatePlay();
            }
            else {
                m_MenuManager.Update(m_Window, m_deltaTime.asSeconds());
            }
        }
        Draw();
    }
}

void Game::InitializeDifficulty(Difficulty difficulty) {
    switch (difficulty) {
    case Easy:
        m_EnemyConfigs[0] = { 15, 10, 20, 1.8f, 1, m_EnemyConfigs[0].texture }; // Enemy 1: 1 damage to base
        m_EnemyConfigs[1] = { 8, 20, 25, 1.6f, 2, m_EnemyConfigs[1].texture }; // Enemy 2: 2 damage to base
        m_EnemyConfigs[2] = { 4, 40, 30, 1.3f, 3, m_EnemyConfigs[2].texture }; // Enemy 3: 3 damage to base
        m_EnemyConfigs[3] = { 2, 60, 40, 1.0f, 5, m_EnemyConfigs[3].texture }; // Enemy 4: 5 damage to base
        m_TowerConfigs[0] = { 12, 40, 5, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 10, 80, 10, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 6, 150, 15, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 4, 250, 20, 192.0f, m_TowerConfigs[3].texture };
        m_iStartingGold = 150;
        m_fGoldPerSecond = 3.0f; // 3 gold per second for Easy
        break;
    case Medium:
        m_EnemyConfigs[0] = { 20, 20, 15, 2.0f, 2, m_EnemyConfigs[0].texture }; // Enemy 1: 2 damage to base
        m_EnemyConfigs[1] = { 12, 40, 20, 1.8f, 3, m_EnemyConfigs[1].texture }; // Enemy 2: 3 damage to base
        m_EnemyConfigs[2] = { 6, 80, 25, 1.5f, 5, m_EnemyConfigs[2].texture }; // Enemy 3: 5 damage to base
        m_EnemyConfigs[3] = { 3, 100, 35, 1.2f, 8, m_EnemyConfigs[3].texture }; // Enemy 4: 8 damage to base
        m_TowerConfigs[0] = { 10, 50, 6, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 8, 100, 10, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 5, 200, 15, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 3, 300, 22, 192.0f, m_TowerConfigs[3].texture };
        m_iStartingGold = 200;
        m_fGoldPerSecond = 5.0f; // 5 gold per second for Medium
        break;
    case Hard:
        m_EnemyConfigs[0] = { 25, 30, 12, 2.2f, 3, m_EnemyConfigs[0].texture }; // Enemy 1: 3 damage to base
        m_EnemyConfigs[1] = { 15, 60, 18, 2.0f, 5, m_EnemyConfigs[1].texture }; // Enemy 2: 5 damage to base
        m_EnemyConfigs[2] = { 8, 100, 22, 1.7f, 8, m_EnemyConfigs[2].texture }; // Enemy 3: 8 damage to base
        m_EnemyConfigs[3] = { 4, 130, 30, 1.4f, 12, m_EnemyConfigs[3].texture }; // Enemy 4: 12 damage to base
        m_TowerConfigs[0] = { 8, 60, 7, 384.0f, m_TowerConfigs[0].texture }; 
        m_TowerConfigs[1] = { 6, 120, 14, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 4, 250, 21, 256.0f, m_TowerConfigs[2].texture }; 
        m_TowerConfigs[3] = { 3, 400, 28, 192.0f, m_TowerConfigs[3].texture };
        m_iStartingGold = 250;
        m_fGoldPerSecond = 10.0f; // 10 gold per second for Hard
        break;
    case VeryHard:
        m_EnemyConfigs[0] = { 30, 40, 10, 2.5f, 5, m_EnemyConfigs[0].texture }; // Enemy 1: 5 damage to base
        m_EnemyConfigs[1] = { 20, 60, 15, 2.2f, 8, m_EnemyConfigs[1].texture }; // Enemy 2: 8 damage to base
        m_EnemyConfigs[2] = { 10, 130, 18, 1.9f, 12, m_EnemyConfigs[2].texture }; // Enemy 3: 12 damage to base
        m_EnemyConfigs[3] = { 6, 175, 25, 1.6f, 20, m_EnemyConfigs[3].texture }; // Enemy 4: 20 damage to base
        m_TowerConfigs[0] = { 6, 80, 8, 384.0f, m_TowerConfigs[0].texture };
        m_TowerConfigs[1] = { 5, 150, 16, 320.0f, m_TowerConfigs[1].texture };
        m_TowerConfigs[2] = { 3, 300, 24, 256.0f, m_TowerConfigs[2].texture };
        m_TowerConfigs[3] = { 2, 450, 40, 192.0f, m_TowerConfigs[3].texture };
        m_iStartingGold = 300;
        m_fGoldPerSecond = 20.0f; // 20 gold per second for VeryHard
        break;
    }

    for (int i = 0; i < 4; ++i) {
        m_EnemyTemplates[i].SetHealth(m_EnemyConfigs[i].health);
        m_EnemyTemplates[i].SetGoldReward(m_EnemyConfigs[i].goldReward);
    }
}

void Game::UpdatePlay() {
    if (m_MenuManager.IsGamePaused() || m_bShowTowerSelection) {
        return;
    }

    m_fTimeInPlayMode += m_deltaTime.asSeconds();
    m_fDifficulty += m_deltaTime.asSeconds() / 10.0f;
    if (m_iPlayerHealth <= 0) {
        if (!m_bGameOverSoundPlayed) {
            SoundManager::getInstance().StopBackgroundMusic();
            SoundManager::getInstance().PlayGameOverSound();
            m_bGameOverSoundPlayed = true;
        }
        return;
    }

    DamageTextManager::getInstanceNonConst().Update(m_deltaTime);
    UpdateTower();
    UpdateProjectiles();

    const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
    sf::Vector2i spawnCoords(-1, -1);
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
    if (spawnCoords != sf::Vector2i(-1, -1) && !paths.empty()) {
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
                if (m_spawnedEnemies[type] < m_EnemyConfigs[type].count) { // Kiểm tra số quái đã sinh
                    Entity& newEnemy = m_enemies.emplace_back(m_EnemyTemplates[type]);
                    newEnemy.SetPosition(sf::Vector2f(spawnCoords.x * 64.0f + 32.0f, spawnCoords.y * 64.0f + 32.0f));
                    newEnemy.SetVelocity(sf::Vector2f(0, 0));
                    newEnemy.SetPathIndex(rand() % paths.size());
                    newEnemy.SetHealth(m_EnemyConfigs[type].health);
                    newEnemy.SetGoldReward(m_EnemyConfigs[type].goldReward);
                    m_spawnedEnemies[type]++; // Tăng số quái đã sinh
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

    // Auto gold generation based on difficulty
    static float fAutoGoldTimer = 0.0f;
    fAutoGoldTimer += m_deltaTime.asSeconds();
    if (fAutoGoldTimer >= 1.0f) {
        int goldToAdd = static_cast<int>(m_fGoldPerSecond);
        AddGold(goldToAdd);
        fAutoGoldTimer -= 1.0f; // Reset timer, keeping remainder
    }

    UpdatePhysics();
    CheckForDeletionRequest();
}

void Game::StartGame(int level, Difficulty difficulty) {
    if (level <= 0) {
        std::cerr << "Invalid level selected: " << level << std::endl;
        ReturnToMenu();
        return;
    }

    m_iCurrentLevel = level;
    m_eDifficulty = difficulty;
    InitializeDifficulty(difficulty);
    LoadLevel(m_iCurrentLevel);

    ResetGameState();
    m_iPlayerHealth = 100; // Set player health to 100
    m_iPlayerGold = m_iStartingGold;

    m_eGameMode = Play;
    m_GameModeText.setString("Play Mode");

    SoundManager::getInstance().StopBackgroundMusic();
    SoundManager::getInstance().PlayBackgroundMusic();

    m_MenuManager.SetMenuState(MenuManager::MenuState::GamePlay);
    UpdatePlayerText();
}

void Game::LoadLevel(int level) {
    std::string levelFileName = "levels/level" + std::to_string(level) + ".txt";
    std::string mapImageFile = "image/maps/map" + std::to_string(level) + ".png";

    try {
        m_MapGrid.loadMapDataFromFile(levelFileName);
        m_Map.Initialize(mapImageFile);
        m_Map.ConstructPath(m_MapGrid);
        std::cout << "Level " << level << " loaded successfully from " << levelFileName << std::endl;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error loading level " << level << ": " << e.what() << std::endl;
        ReturnToMenu();
    }
    catch (...) {
        std::cerr << "An unknown error occurred loading level " << level << "." << std::endl;
        ReturnToMenu();
    }
}

void Game::UpdatePlayerText() {
    std::string difficultyStr;
    switch (m_eDifficulty) {
    case Easy: difficultyStr = "Easy"; break;
    case Medium: difficultyStr = "Medium"; break;
    case Hard: difficultyStr = "Hard"; break;
    case VeryHard: difficultyStr = "Very Hard"; break;
    }

    // Tính tổng số quái còn lại
    int totalEnemiesRemaining = 0;
    for (int i = 0; i < 4; ++i) {
        totalEnemiesRemaining += m_EnemyConfigs[i].count - m_killedEnemies[i];
    }

    m_PlayerText.setString("Difficulty: " + difficultyStr +
        "\nPlayer's Gold: " + std::to_string(m_iPlayerGold) +
        "\nPlayer's Health: " + std::to_string(m_iPlayerHealth) +
        "\nGold Per Second: " + std::to_string(static_cast<int>(m_fGoldPerSecond)) +
        "\nEnemies Remaining: " + std::to_string(totalEnemiesRemaining));
}

// Rest of the code remains unchanged
void Game::UpdateTower() {
    if (m_MenuManager.IsGamePaused() || m_bShowTowerSelection) {
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

        Entity& newProjectile = m_axes.emplace_back(m_BulletTemplates[towerType]);
        newProjectile.SetPosition(tower.GetPosition());
        vTowerToEnemy = MathHelpers::normalize(vTowerToEnemy);
        float speedMultiplier = 1.0f + towerType * 0.5f;
        newProjectile.SetVelocity(vTowerToEnemy * 500.0f * speedMultiplier);
        newProjectile.SetType(towerType + 1);

        newProjectile.SetDamage(m_TowerConfigs[towerType].damage);

        SoundManager::getInstance().PlayHitSound();
        tower.m_fAttackTimer = 1.0f / speedMultiplier;
    }
}

// Đổi tên function từ UpdateAxe() thành UpdateProjectiles() để phù hợp hơn
void Game::UpdateProjectiles() {
    if (m_MenuManager.IsGamePaused() || m_bShowTowerSelection) {
        return;
    }

    for (Entity& projectile : m_axes) { // Có thể đổi tên m_axes thành m_projectiles sau
        projectile.m_fAxeTimer -= m_deltaTime.asSeconds();

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
        case 2: // Shuriken - xoay vòng
        {
            const float fRotationSpeed = 360.0f;
            projectile.GetSpriteNonConst().rotate(fRotationSpeed * m_deltaTime.asSeconds());
            break;
        }

        case 3: // Tên lửa nhỏ - xoay theo hướng bay
        case 4: // Tên lửa lớn - xoay theo hướng bay
        {
            if (MathHelpers::flength(projectile.GetVelocity()) > 0.0f) {
                float fAngle = MathHelpers::Angle(projectile.GetVelocity()) + 180.0f;
                projectile.GetSpriteNonConst().setRotation(fAngle);
            }
            break;
        }

        default:
            // Không xoay cho các loại khác
            break;
        }

        // Kiểm tra thời gian sống của đạn
        if (projectile.m_fAxeTimer <= 0.0f) {
            projectile.RequestDeletion();
        }
    }
}

void Game::CheckForDeletionRequest() {
    if (m_MenuManager.IsGamePaused() || m_bShowTowerSelection) {
        return;
    }

    for (int i = m_axes.size() - 1; i >= 0; i--) {
        Entity& axe = m_axes[i];
        if (axe.IsDeletionRequested()) {
            m_axes.erase(m_axes.begin() + i);
        }
    }

    for (int i = m_enemies.size() - 1; i >= 0; i--) {
        Entity& enemy = m_enemies[i];
        if (enemy.IsDeletionRequested()) {
			int type = enemy.GetType() - 1; // Lấy loại quái
            AddGold(enemy.GetGoldReward());
            m_killedEnemies[type]++; // Tăng số quái đã tiêu diệt
            m_enemies.erase(m_enemies.begin() + i);
            SoundManager::getInstance().PlayEnemyDeathSound();
        }
    }
}

void Game::UpdatePhysics() {
    if (m_MenuManager.IsGamePaused() || m_bShowTowerSelection) {
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
    for (Entity& axe : m_axes) {
        AllEntities.push_back(&axe);
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
    for (const Entity& axe : m_axes) {
        m_Window.draw(axe);
    }

    UpdatePlayerText();
    m_Window.draw(m_PlayerText);

    if (m_bShowTowerSelection) {
        DrawTowerSelectionMenu();
    }
}

void Game::Draw() {
    m_Window.clear();

    if (!m_MenuManager.IsInGamePlay()) {
        m_MenuManager.Draw(m_Window);
    }
    else {
        m_Map.Draw(m_Window);
        m_Window.draw(m_GameModeText);
        m_Window.draw(m_PlayerText);
        DrawPlay();

        if (m_MenuManager.IsGamePaused()) {
            m_MenuManager.Draw(m_Window);
        }
        else if (m_iPlayerHealth <= 0) {
            m_Window.draw(m_GameOverText);
        }
    }
    m_Window.display();
}

void Game::DrawTowerSelectionMenu() {
    for (size_t i = 0; i < m_TowerButtons.size(); ++i) {
        m_Window.draw(m_TowerButtons[i]);
        m_Window.draw(m_TowerButtonTexts[i]);
    }
}

void Game::HandleInput() {
    sf::Event event;

    while (m_Window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_Window.close();
            return;
        }

        if (!m_MenuManager.IsInGamePlay()) {
            m_MenuManager.HandleInput(event, m_Window);

            if (m_MenuManager.IsInGamePlay()) {
                m_eGameMode = Play;
                ResetGameState();
            }
        }
        else {
            static bool wasPaused = false;
            bool currentlyPaused = m_MenuManager.IsGamePaused();

            if (currentlyPaused && !wasPaused) {
                SoundManager::getInstance().PauseBackgroundMusic();
            }
            else if (!currentlyPaused && wasPaused) {
                SoundManager::getInstance().ResumeBackgroundMusic();
            }
            wasPaused = currentlyPaused;

            if (m_MenuManager.IsGamePaused()) {
                m_MenuManager.HandleInput(event, m_Window);
            }
            else if (m_bShowTowerSelection) {
                HandleTowerSelectionInput();
            }
            else {
                HandleGameInput(event);
            }
        }
    }

    if (m_MenuManager.IsInGamePlay() && !m_MenuManager.IsGamePaused() && !m_bShowTowerSelection) {
        HandleKeyboardInput();
    }
}

void Game::HandleGameInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_MenuManager.TogglePauseMenu();
        }
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f vMousePosition = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
        ShowTowerSelectionMenu(vMousePosition);
    }
}

void Game::HandleKeyboardInput() {
}

void Game::HandleTowerSelectionInput() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(m_Window));
        for (size_t i = 0; i < m_TowerButtons.size(); ++i) {
            if (m_TowerButtons[i].getGlobalBounds().contains(mousePos)) {
                if (m_iPlayerGold >= m_TowerConfigs[i].cost && m_TowerCounts[i] < m_TowerConfigs[i].maxCount) {
                    if (CreateTowerAtPosition(m_TowerSelectionPos, i + 1)) {
                        m_iPlayerGold -= m_TowerConfigs[i].cost;
                        m_TowerCounts[i]++;
                    }
                }
                m_bShowTowerSelection = false;
                break;
            }
        }
    }
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
        m_bShowTowerSelection = false;
    }
}

void Game::ShowTowerSelectionMenu(const sf::Vector2f& pos) {
    if (CanPlaceTowerAtPosition(pos)) {
        m_bShowTowerSelection = true;
        m_TowerSelectionPos = pos;
        for (size_t i = 0; i < m_TowerButtons.size(); ++i) {
            m_TowerButtons[i].setPosition(sf::Vector2f(pos.x, pos.y + i * 60));
            m_TowerButtonTexts[i].setString("Tower " + std::to_string(i + 1) + " (" + std::to_string(m_TowerConfigs[i].cost) + " gold, " + std::to_string(m_TowerConfigs[i].damage) + " damage)");
            sf::FloatRect textBounds = m_TowerButtonTexts[i].getLocalBounds();
            m_TowerButtonTexts[i].setPosition(pos.x + (200 - textBounds.width) / 2, pos.y + i * 60 + (50 - textBounds.height) / 2);
        }
    }
}

void Game::ResetGameState() {
    m_enemies.clear();
    m_axes.clear();
    m_Towers.clear();
    for (int i = 0; i < 4; ++i) {
        m_TowerCounts[i] = 0;
		m_spawnedEnemies[i] = 0;
        m_killedEnemies[i] = 0;
    }

    m_iGoldGainedThisUpdate = 0;
    m_fTimeInPlayMode = 0.0f;
    m_fDifficulty = 1.0f;
    m_fGoldPerSecondTimer = 0.0f;
    m_bGameRunning = true;
    m_bGameOverSoundPlayed = false;
    m_bShowTowerSelection = false;

    m_GameModeText.setString("Play Mode");
    m_PlayerText.setString("Player Gold: " + std::to_string(m_iPlayerGold) + "\nHealth: " + std::to_string(m_iPlayerHealth));
}

void Game::ReturnToMenu() {
    m_MenuManager.SetMenuState(MenuManager::MenuState::MainMenu);
    m_GameModeText.setString("Menu Mode");

    SoundManager::getInstance().StopBackgroundMusic();
    SoundManager::getInstance().PlayBackgroundMusic();
}

void Game::ExitGame() {
    if (m_MenuManager.GetCurrentProfile()) {
        MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());
        m_MenuManager.SaveProfilesToFile();
        std::cout << "Đã lưu dữ liệu profile: " << profile->name << std::endl;
    }

    m_Window.close();
}

bool Game::CreateTowerAtPosition(const sf::Vector2f& pos, int towerType) {
    if (CanPlaceTowerAtPosition(pos) && towerType >= 1 && towerType <= 4) {
        int index = towerType - 1;
        if (m_TowerCounts[index] < m_TowerConfigs[index].maxCount) {
            Entity newTower = m_TowerTemplates[index];
            newTower.SetColor(sf::Color::White);
            newTower.SetPosition(pos);
            newTower.SetType(towerType);
            m_Towers.push_back(newTower);

            SoundManager::getInstance().PlayTowerPlaceSound();
            return true;
        }
    }
    return false;
}

bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
    int gridX = static_cast<int>(pos.x / 64.0f);
    int gridY = static_cast<int>(pos.y / 64.0f);

    if (gridX < 0 || gridX >= m_MapGrid.getWidth() || gridY < 0 || gridY >= m_MapGrid.getHeight()) {
        std::cout << "Position out of bounds: (" << pos.x << ", " << pos.y << ")" << std::endl;
        return false;
    }

    const auto& nodeMatrix = m_MapGrid.getNodeMatrix();
    if (nodeMatrix[gridY][gridX] != MapGrid::TileType::Aesthetic) {
        std::cout << "Cannot place tower: not on aesthetic tile at (" << gridX << ", " << gridY << ")" << std::endl;
        return false;
    }

    Entity towerAtPosition = m_TowerTemplates[0];
    towerAtPosition.SetPosition(pos);

    for (const Entity& tower : m_Towers) {
        if (isColiding(tower, towerAtPosition)) {
            std::cout << "Tower would collide with existing tower" << std::endl;
            return false;
        }
    }

    cout << "Gold: " << m_iPlayerGold << endl;
    std::cout << "Tower can be placed successfully at (" << pos.x << ", " << pos.y << ")" << std::endl;
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