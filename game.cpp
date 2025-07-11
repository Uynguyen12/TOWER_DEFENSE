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
    : m_Window(sf::VideoMode({ 2560, 1600 }), "SFML window")
    , m_eGameMode(Play)
    , m_optionIndex(0)
    , m_eScrollWheelInput(None)
    , m_TowerTemplate(Entity::PhysicsData::Type::Static)
    , m_enemyTemplate(Entity::PhysicsData::Type::Dynamic)
    , m_axeTemplate(Entity::PhysicsData::Type::Dynamic)
    , m_iPlayerHealth(10)
    , m_iPlayerGold(10)
    , m_fTimeInPlayMode(0.0f)
    , m_fDifficulty(1.0f)
    , m_iGoldGainedThisUpdate(0)
    , m_fGoldPerSecond(0.0f)
    , m_fGoldPerSecondTimer(0.0f)
    , m_bGameRunning(true)
{
    // Initialize MenuManager first
    m_MenuManager.Initialize(m_Window);

    // Initialize Map
    m_Map.Initialize();

    // Initialize SoundManager
    SoundManager::getInstance().Initialize();
    SoundManager::getInstance().PlayBackgroundMusic();

    // Load textures and check return values
    if (!towerTexture.loadFromFile("image/player.png")) {
        throw std::runtime_error("Failed to load player texture from 'image/player.png'");
    }
    if (!enemyTexture.loadFromFile("image/enemy.png")) {
        throw std::runtime_error("Failed to load enemy texture from 'image/enemy.png'");
    }
    if (!axeTexture.loadFromFile("image/axe.png")) {
        throw std::runtime_error("Failed to load axe texture from 'image/axe.png'");
    }

    // Set textures for sprites
    m_TowerTemplate.SetTexture(towerTexture);
    m_TowerTemplate.SetScale(sf::Vector2f(5, 5));
    m_TowerTemplate.SetOrigin(sf::Vector2f(8, 8));
    m_TowerTemplate.setCirclePhysics(40.f);
    m_TowerTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Tower);

    m_enemyTemplate.SetTexture(enemyTexture);
    m_enemyTemplate.SetScale(sf::Vector2f(5, 5));
    m_enemyTemplate.SetPosition(sf::Vector2f(960, 540));
    m_enemyTemplate.SetOrigin(sf::Vector2f(8, 8));
    m_enemyTemplate.setCirclePhysics(40.f);
    m_enemyTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Enemy);
    m_enemyTemplate.SetHealth(3);

    m_axeTemplate.SetTexture(axeTexture);
    m_axeTemplate.SetScale(sf::Vector2f(5, 5));
    m_axeTemplate.SetOrigin(sf::Vector2f(8, 8));
    m_axeTemplate.setCirclePhysics(40.f);
    m_axeTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Projectile);
    m_axeTemplate.GetPhysicsDataNonConst().setLayersToIgnore(Entity::PhysicsData::Layer::Projectile | Entity::PhysicsData::Layer::Tower);

    m_Font.loadFromFile("Fonts/Kreon-Medium.ttf");

    m_GameModeText.setPosition(sf::Vector2f(1000, 200));
    m_GameModeText.setFont(m_Font);
    m_GameModeText.setString("Menu Mode");

    m_PlayerText.setPosition(sf::Vector2f(1500, 100));
    m_PlayerText.setString("Player");
    m_PlayerText.setFont(m_Font);

    m_GameOverText.setPosition(sf::Vector2f(1080, 800));
    m_GameOverText.setString("GAME OVERRR");
    m_GameOverText.setFont(m_Font);
    m_GameOverText.setCharacterSize(100);

    m_MenuManager.SetExitCallback([this]() {
        this->ExitGame();
    });
}

Game::~Game() {
    SoundManager::getInstance().Cleanup();
}

void Game::run() {
    sf::Clock clock;
    while (m_Window.isOpen()) {
        m_deltaTime = clock.restart();
        HandleInput();

        // Check if in menu
        if (!m_MenuManager.IsInGamePlay()) {
            m_MenuManager.Update(m_Window, m_deltaTime.asSeconds());
        }
        else {
            // Only update game logic when playing and not paused
            if (!m_MenuManager.IsGamePaused()) {
                switch (m_eGameMode) {
                case Play:
                    UpdatePlay();
                    break;
                case LevelEditor:
                    UpdateLevelEditor();
                    break;
                }
            }
        }
        Draw();
    }
}

void Game::UpdatePlay() {
    // Stop all activities if game is paused
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    m_fTimeInPlayMode += m_deltaTime.asSeconds();
    m_fDifficulty += m_deltaTime.asSeconds() / 10.0f;
    if (m_iPlayerHealth <= 0) {
        static bool gameOverSoundPlayed = false;
        if (!gameOverSoundPlayed) {
            SoundManager::getInstance().StopBackgroundMusic();
            SoundManager::getInstance().PlayGameOverSound();
            gameOverSoundPlayed = true;
        }
        return;
    }

    DamageTextManager::getInstanceNonConst().Update(m_deltaTime);
    UpdateTower();
    UpdateAxe();

    const int iMaxEnemies = 30;
    const std::vector<Entity>& spawnTiles = m_Map.GetSpawnTiles();
    const std::vector<Map::Path>& paths = m_Map.GetPaths();

    if (spawnTiles.size() > 0 && !paths.empty()) {
        m_enemyTemplate.SetPosition(spawnTiles[0].GetPosition());
        if (m_enemies.size() < iMaxEnemies) {
            static float fSpawnTimer = 0.0f;
            float fSpawnRate = m_fDifficulty;
            fSpawnTimer += m_deltaTime.asSeconds() * fSpawnRate;
            if (fSpawnTimer > 1.0f) {
                Entity& newEnemy = m_enemies.emplace_back(m_enemyTemplate);
                newEnemy.SetPathIndex(rand() % paths.size());
                fSpawnTimer = 0.0f;
            }
        }
    }

    const std::vector<Entity>& endTiles = m_Map.GetEndTiles();
    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Entity& rEnemy = m_enemies[i];
        const Map::Path& path = paths[rEnemy.GetPathIndex()];

        // Find closest PathTile to the enemy
        const Map::PathTile* pClosestTile = nullptr;
        float fClosestDistance = std::numeric_limits<float>::max();

        for (const Map::PathTile& tile : path) {
            sf::Vector2f vEnemyToTile = tile.pCurrentTile->GetPosition() - rEnemy.GetPosition();
            float fDistance = MathHelpers::flength(vEnemyToTile);

            if (fDistance < fClosestDistance) {
                fClosestDistance = fDistance;
                pClosestTile = &tile;
            }
        }

        if (!pClosestTile || !pClosestTile->pNextTile) continue;
        const Entity* pNextTile = pClosestTile->pNextTile;

        if (endTiles.size() > 0 && pNextTile->GetClosestGridCoordinates() == endTiles[0].GetClosestGridCoordinates()) {
            if (fClosestDistance < 40.0f) {
                m_enemies.erase(m_enemies.begin() + i);
                m_fDifficulty *= 0.9f;
                continue;
            }
        }

        float fEnemySpeed = 250.0f;
        sf::Vector2f vEnemyToNextTile = pNextTile->GetPosition() - rEnemy.GetPosition();
        vEnemyToNextTile = MathHelpers::normalize(vEnemyToNextTile);
        rEnemy.SetVelocity(vEnemyToNextTile * fEnemySpeed);
    }

    UpdatePhysics();
    CheckForDeletionRequest();

    m_fGoldPerSecondTimer += m_deltaTime.asSeconds();
    if (m_fGoldPerSecondTimer > 0.05f) {
        m_fGoldPerSecond = m_fGoldPerSecond * 0.9f + 0.1f * m_iGoldGainedThisUpdate / m_fGoldPerSecondTimer;
        m_fGoldPerSecondTimer = 0.0f;
        m_iGoldGainedThisUpdate = 0;
    }
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
        for (Entity& enemy : m_enemies) {
            sf::Vector2f vTowerToEnemy = enemy.GetPosition() - tower.GetPosition();
            float fDistance = MathHelpers::flength(vTowerToEnemy);
            if (fDistance < fClosestDistance) {
                fClosestDistance = fDistance;
                pClosestEnemy = &enemy;
            }
        }

        if (!pClosestEnemy) {
            continue;
        }

        sf::Vector2f vTowerToEnemy = pClosestEnemy->GetPosition() - tower.GetPosition();
        float fAngle = MathHelpers::Angle(vTowerToEnemy);
        tower.GetSpriteNonConst().setRotation(fAngle);

        Entity& newAxe = m_axes.emplace_back(m_axeTemplate);
        newAxe.SetPosition(tower.GetPosition());
        vTowerToEnemy = MathHelpers::normalize(vTowerToEnemy);
        newAxe.SetVelocity(vTowerToEnemy * 500.0f);

        SoundManager::getInstance().PlayHitSound();
        tower.m_fAttackTimer = 1.0f;
    }
}

void Game::UpdateAxe() {
    if (m_MenuManager.IsGamePaused()) {
        return;
    }

    for (Entity& axe : m_axes) {
        axe.m_fAxeTimer -= m_deltaTime.asSeconds();
        const float fAxeRotationSpeed = 360.0f;
        axe.GetSpriteNonConst().rotate(fAxeRotationSpeed * m_deltaTime.asSeconds());
        if (axe.m_fAxeTimer <= 0.0f) {
            axe.RequestDeletion();
        }
    }
}

void Game::CheckForDeletionRequest() {
    if (m_MenuManager.IsGamePaused()) {
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
            m_enemies.erase(m_enemies.begin() + i);
            AddGold(1);
            // Play enemy death sound
            SoundManager::getInstance().PlayEnemyDeathSound();
        }
    }
}

void Game::UpdateLevelEditor() {
    m_iPlayerGold = 10;
    m_iPlayerHealth = 10;
    m_iGoldGainedThisUpdate = 0;
    m_fTimeInPlayMode = 0.0f;
    m_fDifficulty = 1.0f;
    m_fGoldPerSecond = 0.0f;
    m_fGoldPerSecondTimer = 0.0f;
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

            // Check collisions
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
        // we are circle
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            // Both are circles
            const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
            const float fDistanceBeeenEntities = MathHelpers::flength(vEntity1ToEntity2);
            float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;

            if (fDistanceBeeenEntities < fSumOfRadii) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    // We only need to move entity1
                    entity1.move(-MathHelpers::normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBeeenEntities));
                }
                else {
                    // Both entities are dynamic, we need to move both of them
                    const sf::Vector2f vEntity1ToEntity2Normalized = MathHelpers::normalize(vEntity1ToEntity2);
                    const sf::Vector2f vEntity1Movement = vEntity1ToEntity2Normalized * (fSumOfRadii - fDistanceBeeenEntities) * 0.5f;
                    entity1.move(-vEntity1Movement);
                    entity2.move(vEntity1Movement);
                }
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            // We are circle, they are rectangle
            float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth / 2, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight / 2, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity1.GetPhysicsData().m_fRadius) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    // We only need to move entity1
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
        // we are rectangle
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            // Both are rectangles
            float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
            float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

            float fOverlapX = (entity1.GetPhysicsData().m_fWidth + entity2.GetPhysicsData().m_fWidth) / 2 - fDistanceX;
            float fOverlapY = (entity1.GetPhysicsData().m_fHeight + entity2.GetPhysicsData().m_fHeight) / 2 - fDistanceY;
            if (fOverlapX > 0 && fOverlapY > 0) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                // Guarantee a collision
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
            // We are rectangle, they are circle
            float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth / 2, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth / 2);
            float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight / 2, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight / 2);

            sf::Vector2f vClosestPoint(fClosestX, fClosestY);
            sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
            float fDistanceToClosestPoint = MathHelpers::flength(vCircleToClosestPoint);

            if (fDistanceToClosestPoint < entity2.GetPhysicsData().m_fRadius) {
                const bool isEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
                if (!isEntity2Dynamic) {
                    // We only need to move entity1
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
        // we are circle
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            // Both are circles
            const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
            const float fDistanceBeeenEntities = MathHelpers::flength(vEntity1ToEntity2);
            float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;

            if (fDistanceBeeenEntities < fSumOfRadii) {
                return true;
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            // We are circle, they are rectangle
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
        // we are rectangle
        if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle) {
            // Both are rectangles
            float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
            float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

            float fOverlapX = (entity1.GetPhysicsData().m_fWidth + entity2.GetPhysicsData().m_fWidth) / 2 - fDistanceX;
            float fOverlapY = (entity1.GetPhysicsData().m_fHeight + entity2.GetPhysicsData().m_fHeight) / 2 - fDistanceY;
            if (fOverlapX > 0 && fOverlapY > 0) {
                return true;
            }
        }
        else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
            // We are rectangle, they are circle
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
    sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
    m_TowerTemplate.SetPosition(vMousePosition);

    if (CanPlaceTowerAtPosition(vMousePosition)) {
        m_TowerTemplate.SetColor(sf::Color::Green);
    }
    else {
        m_TowerTemplate.SetColor(sf::Color::Red);
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

    DamageTextManager::getInstanceConst().Draw(m_Window);

    m_Window.draw(m_TowerTemplate); // Draw the tower template

    if (m_iPlayerHealth <= 0) {
        //draw the game over text
        m_Window.draw(m_GameOverText);
    }

    m_PlayerText.setString("Difficulty: " + to_string(m_fDifficulty) +
        "\nPlayer's Gold: " + to_string(m_iPlayerGold) +
        "\nGold Per Second: " + to_string(m_fGoldPerSecond));
    m_Window.draw(m_PlayerText);
}

void Game::Draw() {
    m_Window.clear();

    if (!m_MenuManager.IsInGamePlay()) {
        m_MenuManager.Draw(m_Window);
    }
    else {
        // Draw map tiles
        m_Map.DrawTiles(m_Window);

        m_Window.draw(m_GameModeText);

        switch (m_eGameMode) {
        case Play:
            DrawPlay();
            break;
        case LevelEditor:
            DrawLevelEditor();
            break;
        }

        if (m_MenuManager.IsGamePaused()) {
            m_MenuManager.Draw(m_Window);
        }
    }
    m_Window.display();
}


void Game::HandleInput() {
    sf::Event event;
    m_eScrollWheelInput = None;

    while (m_Window.pollEvent(event)) {
        // Xử lý sự kiện đóng cửa sổ
        if (event.type == sf::Event::Closed) {
            m_Window.close();
            return;
        }

        // Nếu đang trong menu, chuyển input cho MenuManager
        if (!m_MenuManager.IsInGamePlay()) {
            m_MenuManager.HandleInput(event, m_Window);

            // Kiểm tra nếu game đã bắt đầu từ menu
            if (m_MenuManager.IsInGamePlay()) {
                m_eGameMode = Play; // Luôn bắt đầu ở Play mode
                m_GameModeText.setString("Play Mode");
                // Reset game state khi bắt đầu game mới
                ResetGameState();
            }
        }
        else {
            // Xử lý pause/resume music dựa trên trạng thái game
            static bool wasPaused = false;
            bool currentlyPaused = m_MenuManager.IsGamePaused();

            if (currentlyPaused && !wasPaused) {
                // Vừa chuyển sang trạng thái pause
                SoundManager::getInstance().PauseBackgroundMusic();
            }
            else if (!currentlyPaused && wasPaused) {
                // Vừa thoát khỏi trạng thái pause
                SoundManager::getInstance().ResumeBackgroundMusic();
            }
            wasPaused = currentlyPaused;

            // Nếu đang trong gameplay, kiểm tra pause menu trước
            if (m_MenuManager.IsGamePaused()) {
                // Nếu game đang pause, chuyển input cho MenuManager để xử lý pause menu
                m_MenuManager.HandleInput(event, m_Window);
            }
            else {
                // Xử lý input trong game bình thường
                HandleGameInput(event);
            }
        }
    }

    // Chỉ xử lý keyboard input khi đang trong game
    if (m_MenuManager.IsInGamePlay() && !m_MenuManager.IsGamePaused()) {
        HandleKeyboardInput();
    }
}

void Game::HandleGameInput(sf::Event& event) {
    switch (event.type) {
    case sf::Event::MouseWheelScrolled:
        if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            if (event.mouseWheelScroll.delta > 0) {
                m_eScrollWheelInput = ScrollUp;
            }
            else {
                m_eScrollWheelInput = ScrollDown;
            }
        }
        break;
    case sf::Event::KeyPressed:
        // ESC để quay về menu
        if (event.key.code == sf::Keyboard::Escape) {
            m_MenuManager.TogglePauseMenu();
        }
        // Thêm phím Tab để chuyển đổi giữa Play và Level Editor (chỉ khi đang trong game)
        else if (event.key.code == sf::Keyboard::Tab) {
            if (m_eGameMode == Play) {
                m_eGameMode = LevelEditor;
                m_GameModeText.setString("Level Editor Mode");
            }
            else {
                m_eGameMode = Play;
                m_GameModeText.setString("Play Mode");
            }
        }
        break;
    }
}

void Game::HandleKeyboardInput() {
    // Phím T để chuyển đổi giữa Play và Level Editor
    static bool bTwasPressedLastUpdate = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T)) {
        if (!bTwasPressedLastUpdate) {
            if (m_eGameMode == Play) {
                m_eGameMode = LevelEditor;
                m_GameModeText.setString("Level Editor Mode");
            }
            else {
                m_eGameMode = Play;
                m_GameModeText.setString("Play Mode");
            }
        }
        bTwasPressedLastUpdate = true;
    }
    else {
        bTwasPressedLastUpdate = false;
    }

    // Xử lý input theo game mode
    switch (m_eGameMode) {
    case Play:
        HandlePlayInput();
        break;
    case LevelEditor:
        HandleLevelEditorInput();
        break;
    }
}

void Game::StartGame(int level) {
    m_iCurrentLevel = level;
    m_MenuManager.SetMenuState(MenuManager::MenuState::GamePlay);
    m_eGameMode = Play;
    m_GameModeText.setString("Play Mode");

    ResetGameState();

    // Nếu có profile được chọn và đã lưu dữ liệu
    if (m_MenuManager.GetCurrentProfile()) {
        const MenuManager::PlayerProfile* profile = m_MenuManager.GetCurrentProfile();
        if (!profile->savedTowers.empty()) {
            m_MenuManager.SaveProfilesToFile();
        }
    }
}

void Game::ReturnToMenu() {
    m_MenuManager.SetMenuState(MenuManager::MenuState::MainMenu);
    m_GameModeText.setString("Menu Mode");

    m_eGameMode = Play;

    // Dừng nhạc nền game và phát nhạc menu
    SoundManager::getInstance().StopBackgroundMusic();
    SoundManager::getInstance().PlayBackgroundMusic();
}

void Game::ExitGame() {
    // Nếu có profile đang được chọn
    if (m_MenuManager.GetCurrentProfile()) {
        MenuManager::PlayerProfile* profile = const_cast<MenuManager::PlayerProfile*>(m_MenuManager.GetCurrentProfile());

        m_MenuManager.SaveProfilesToFile();

        std::cout << "📁 Đã lưu dữ liệu profile: " << profile->name << std::endl;
    }

    m_Window.close();
}

void Game::ResetGameState() {
    // Reset tất cả game state về trạng thái ban đầu
    m_enemies.clear();
    m_axes.clear();
    m_Towers.clear();

    m_iPlayerHealth = 10;
    m_iPlayerGold = 10;
    m_iGoldGainedThisUpdate = 0;
    m_fTimeInPlayMode = 0.0f;
    m_fDifficulty = 1.0f;
    m_fGoldPerSecond = 0.0f;
    m_fGoldPerSecondTimer = 0.0f;
    m_bGameRunning = true;
}

void Game::DrawLevelEditor() {
    sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
    m_Map.DrawLevelEditor(m_Window, m_optionIndex, vMousePosition);
}

void Game::HandlePlayInput() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
        if (m_iPlayerGold >= 3) {
            if (CreateTowerAtPosition(vMousePosition)) {
                m_iPlayerGold -= 3;
            }
        }
    }
}

void Game::HandleLevelEditorInput() {
    if (m_eScrollWheelInput == ScrollUp) {
        m_optionIndex++;
        if (m_optionIndex >= m_Map.GetTileOptionsCount()) {
            m_optionIndex = 0;
        }
    }
    else if (m_eScrollWheelInput == ScrollDown) {
        m_optionIndex--;
        if (m_optionIndex < 0) {
            m_optionIndex = m_Map.GetTileOptionsCount() - 1;
        }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
        m_Map.CreateTileAtPosition(vMousePosition, m_optionIndex);
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
        sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
        m_Map.DeleteTileAtPosition(vMousePosition, m_optionIndex);
    }
}

bool Game::CreateTowerAtPosition(const sf::Vector2f& pos) {
    if (CanPlaceTowerAtPosition(pos)) {
        Entity newTower = m_TowerTemplate;
        newTower.SetColor(sf::Color::White);
        newTower.SetPosition(pos);
        m_Towers.push_back(newTower);

        SoundManager::getInstance().PlayTowerPlaceSound();
        return true;
    }
    return false;
}

bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
    sf::IntRect brickRect(0, 0, 16, 16);
    vector<Entity>& ListOfTiles = m_Map.GetListOfTiles(TileOptions::TileType::Aesthetic);
    bool isOnBrick = false;
    Entity copyOfTowerWithRadiusOf1 = m_TowerTemplate;
    copyOfTowerWithRadiusOf1.setCirclePhysics(1.0f);
    copyOfTowerWithRadiusOf1.SetPosition(pos);  // Set the position before collision checks

    for (const Entity& tile : ListOfTiles) {
        const sf::Sprite& rTileSprite = tile.GetSprite();
        sf::IntRect tileRect = rTileSprite.getTextureRect();
        if (tileRect != brickRect) {
            continue;
        }
        if (isColiding(tile, copyOfTowerWithRadiusOf1)) {
            isOnBrick = true;
            break;
        }
    }
    if (!isOnBrick) {
        return false;
    }

    // Create a copy of the tower template at the target position for collision checking
    Entity towerAtPosition = m_TowerTemplate;
    towerAtPosition.SetPosition(pos);

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