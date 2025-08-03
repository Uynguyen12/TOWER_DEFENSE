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
#include "MapGrid.h"

Game::Game()
    : m_Window()
    , m_eGameMode(Play)
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
    , m_bGameOverSoundPlayed(false)
    , m_iCurrentLevel(0)
{
    m_MenuManager.LoadSettingsFromFile();

    auto settings = m_MenuManager.GetSettings();
    m_Window.create(sf::VideoMode(settings.resolution.x, settings.resolution.y), "SFML window");

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
    m_GameModeText.setPosition(sf::Vector2f(windowSize.x * 0.75f, windowSize.y * 0.1f));
    m_GameModeText.setString("Play Mode");

    m_PlayerText.setFont(m_Font);
    m_GameOverText.setCharacterSize(100);
    m_GameOverText.setPosition(sf::Vector2f(windowSize.x * 0.5f - m_GameOverText.getLocalBounds().width / 2, windowSize.y * 0.5f));
    m_GameOverText.setString("GAME OVERRR");


    // Set up MenuManager callbacks
    m_MenuManager.SetExitCallback([this](sf::RenderWindow& window) {
        ExitGame();
        });
    m_MenuManager.SetStartGameCallback([this](int level) {
        StartGame(level);
        });

    // Load textures for templates
    if (!towerTexture.loadFromFile("image/player.png")) {
        throw std::runtime_error("Failed to load player texture from 'image/player.png'");
    }
    if (!enemyTexture.loadFromFile("image/enemy.png")) {
        throw std::runtime_error("Failed to load enemy texture from 'image/enemy.png'");
    }
    if (!axeTexture.loadFromFile("image/axe.png")) {
        throw std::runtime_error("Failed to load axe texture from 'image/axe.png'");
    }

    // Set up templates
    m_TowerTemplate.SetTexture(towerTexture);
    m_TowerTemplate.SetScale(sf::Vector2f(5, 5));
    m_TowerTemplate.SetOrigin(sf::Vector2f(8, 8));
    m_TowerTemplate.setCirclePhysics(40.f);
    m_TowerTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Tower);

    m_enemyTemplate.SetTexture(enemyTexture);
    m_enemyTemplate.SetScale(sf::Vector2f(5, 5));
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
            if (!m_MenuManager.IsGamePaused()) {
                UpdatePlay();
            }
            else {
                m_MenuManager.Update(m_Window, m_deltaTime.asSeconds());
            }
        }
        Draw();
    }
}

void Game::LoadLevel(int level) {
    std::string levelFileName = "levels/level" + std::to_string(level) + ".txt";

    try {
        m_Map.Initialize("image/TileMap.png");
        m_MapGrid.loadMapDataFromFile(levelFileName);
        m_Map.PopulateFromMapGrid(m_MapGrid);
        m_Map.ConstructPath();

        std::cout << "Level " << level << " loaded successfully from " << levelFileName << std::endl;
        std::cout << "Aesthetic tiles count: " << m_Map.GetListOfTiles(TileOptions::TileType::Aesthetic).size() << std::endl;
        std::cout << "Spawn tiles count: " << m_Map.GetListOfTiles(TileOptions::TileType::Spawn).size() << std::endl;
        std::cout << "End tiles count: " << m_Map.GetListOfTiles(TileOptions::TileType::End).size() << std::endl;
        std::cout << "Path tiles count: " << m_Map.GetListOfTiles(TileOptions::TileType::Path).size() << std::endl;
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

void Game::UpdatePlay() {
    if (m_MenuManager.IsGamePaused()) {
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
    UpdateAxe();

    const int iMaxEnemies = 30;
    const std::vector<Entity>& spawnTiles = m_Map.GetSpawnTiles();
    const std::vector<Map::Path>& paths = m_Map.GetPaths();

    std::cout << "Spawn tiles count: " << spawnTiles.size() << std::endl;
    std::cout << "Paths count: " << paths.size() << std::endl;
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

        std::cout << "Processing enemy " << i << " at position (" << rEnemy.GetPosition().x << ", " << rEnemy.GetPosition().y << ")" << std::endl;
        const Map::Path& path = paths[rEnemy.GetPathIndex()];

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
                m_iPlayerHealth--;
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
}

void Game::Draw() {
    m_Window.clear();

    if (!m_MenuManager.IsInGamePlay()) {
        m_MenuManager.Draw(m_Window);
    }
    else {
        m_Map.DrawTiles(m_Window);
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
                m_GameModeText.setString("Play Mode");
                ResetGameState();
            }
        }
        else {
            //Tắt music khi paused

            /*static bool wasPaused = false;
            bool currentlyPaused = m_MenuManager.IsGamePaused();

            if (currentlyPaused && !wasPaused) {
                SoundManager::getInstance().PauseBackgroundMusic();
            }
            else if (!currentlyPaused && wasPaused) {
                SoundManager::getInstance().ResumeBackgroundMusic();
            }
            wasPaused = currentlyPaused;*/

            if (m_MenuManager.IsGamePaused()) {
                m_MenuManager.HandleInput(event, m_Window);
            }
            else {
                HandleGameInput(event);
            }
        }
    }

    if (m_MenuManager.IsInGamePlay() && !m_MenuManager.IsGamePaused()) {
        HandleKeyboardInput();
    }
}

void Game::HandleGameInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_MenuManager.TogglePauseMenu();
        }
    }
}

void Game::HandleKeyboardInput() {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
        if (m_iPlayerGold >= 3) {
            if (CreateTowerAtPosition(vMousePosition)) {
                m_iPlayerGold -= 3;
            }
        }
    }
}

void Game::StartGame(int level) {
    if (level <= 0) {
        std::cerr << "Invalid level selected: " << level << std::endl;
        ReturnToMenu();
        return;
    }

    m_iCurrentLevel = level;
    LoadLevel(m_iCurrentLevel);

    ResetGameState();
    m_iPlayerHealth = 10;
    m_iPlayerGold = 10;

    m_eGameMode = Play;
    m_GameModeText.setString("Play Mode");

    SoundManager::getInstance().StopBackgroundMusic();
    SoundManager::getInstance().PlayBackgroundMusic();

    m_MenuManager.SetMenuState(MenuManager::MenuState::GamePlay);
    UpdatePlayerText();
}

void Game::UpdatePlayerText() {
    m_PlayerText.setString("Difficulty: " + std::to_string(static_cast<int>(m_fDifficulty)) +
        "\nPlayer's Gold: " + std::to_string(m_iPlayerGold) +
        "\nGold Per Second: " + std::to_string(static_cast<int>(m_fGoldPerSecond)));
}

void Game::ResetGameState() {
    m_enemies.clear();
    m_axes.clear();
    m_Towers.clear();

    m_iGoldGainedThisUpdate = 0;
    m_fTimeInPlayMode = 0.0f;
    m_fDifficulty = 1.0f;
    m_fGoldPerSecond = 0.0f;
    m_fGoldPerSecondTimer = 0.0f;
    m_bGameRunning = true;
    m_bGameOverSoundPlayed = false;

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
    sf::IntRect wallRect(0, 0, 16, 16);
    vector<Entity>& ListOfTiles = m_Map.GetListOfTiles(TileOptions::TileType::Aesthetic);
    bool isOnWall = false;
    Entity copyOfTowerWithRadiusOf1 = m_TowerTemplate;
    copyOfTowerWithRadiusOf1.setCirclePhysics(1.0f);
    copyOfTowerWithRadiusOf1.SetPosition(pos);

    std::cout << "Checking tower placement at position: (" << pos.x << ", " << pos.y << ")" << std::endl;
    std::cout << "Number of Aesthetic tiles: " << ListOfTiles.size() << std::endl;

    for (const Entity& tile : ListOfTiles) {
        const sf::Sprite& rTileSprite = tile.GetSprite();
        sf::IntRect tileRect = rTileSprite.getTextureRect();
        std::cout << "Tile texture rect: (" << tileRect.left << ", " << tileRect.top << ", " << tileRect.width << ", " << tileRect.height << ")" << std::endl;

        if (tileRect != wallRect) {
            std::cout << "Tile is not a wall, skipping..." << std::endl;
            continue;
        }

        std::cout << "Found wall tile, checking collision..." << std::endl;
        if (isColiding(tile, copyOfTowerWithRadiusOf1)) {
            std::cout << "Tower is on wall!" << std::endl;
            isOnWall = true;
            break;
        }
        else {
            std::cout << "Tower is not colliding with this wall tile" << std::endl;
        }
    }

    if (!isOnWall) {
        std::cout << "Tower is not on any wall, cannot place" << std::endl;
        return false;
    }

    Entity towerAtPosition = m_TowerTemplate;
    towerAtPosition.SetPosition(pos);

    for (const Entity& tower : m_Towers) {
        if (isColiding(tower, towerAtPosition)) {
            std::cout << "Tower would collide with existing tower" << std::endl;
            return false;
        }
    }

    std::cout << "Tower can be placed successfully!" << std::endl;
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