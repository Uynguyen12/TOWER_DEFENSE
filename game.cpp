#include "game.h"
#include <SFML/Graphics.hpp>
#include "MathHelpers.h"
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cassert>
#include "DamageTextManager.h"
#include <fstream>
#include <iostream> 

// ADDED: Implementation for the mouse position helper
sf::Vector2f Game::getMouseWorldPosition() const {
	// mapPixelToCoords requires the view to be active, which it is during the main loop
	return m_Window.mapPixelToCoords(sf::Mouse::getPosition(m_Window));
}

Game::Game()
	/*: m_Window(sf::VideoMode({ 2560, 1600 }), "SFML window")
	, m_eGameMode(Play)*/
	: m_Window(sf::VideoMode(1920, 1080), "SFML window")
	, m_gameView(sf::FloatRect(0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT)) // ADDED: Initialize the view
	, m_eGameMode(MainMenu)
	, m_optionIndex(0)
	, m_eScrollWheelInput(None)
	, m_TowerTemplate(Entity::PhysicsData::Type::Static)
	, m_enemyTemplate(Entity::PhysicsData::Type::Dynamic)
	, m_axeTemplate(Entity::PhysicsData::Type::Dynamic)
	, m_bDrawPath(true)
	, m_iPlayerHealth(10)
	, m_iPlayerGold(10)
	, m_fTimeInPlayMode(0.0f)
	, m_fDifficulty(1.0f)
	, m_iGoldGainedThisUpdate(0)
	, m_fGoldPerSecond(0.0f)
	, m_fGoldPerSecondTimer(0.0f)
	, m_iCurrentLevel(1)
{
	// Load textures and check return values
	if (!towerTexture.loadFromFile("assets/images/player.png")) {
		throw std::runtime_error("Failed to load player texture from 'assets/images/player.png'");
	}
	if (!enemyTexture.loadFromFile("assets/images/enemy.png")) {
		throw std::runtime_error("Failed to load enemy texture from 'assets/images/enemy.png'");
	}
	if (!axeTexture.loadFromFile("assets/images/axe.png")) {
		throw std::runtime_error("Failed to load axe texture from 'assets/images/axe.png'");
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
	m_enemyTemplate.setCirclePhysics(40.f); // Set the enemy as a circle with a radius of 80 pixels
	m_enemyTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Enemy);
	m_enemyTemplate.SetHealth(3);

	m_axeTemplate.SetTexture(axeTexture);
	m_axeTemplate.SetScale(sf::Vector2f(5, 5));
	m_axeTemplate.SetOrigin(sf::Vector2f(8, 8));
	m_axeTemplate.setCirclePhysics(40.f); // Set the axe as a circle with a radius of 80 pixels
	m_axeTemplate.GetPhysicsDataNonConst().setLayers(Entity::PhysicsData::Layer::Projectile);
	m_axeTemplate.GetPhysicsDataNonConst().setLayersToIgnore(Entity::PhysicsData::Layer::Projectile | Entity::PhysicsData::Layer::Tower);

	m_Font.loadFromFile("assets/fonts/Kreon-Medium.ttf");

	// Initialize UI Text
	m_TitleText.setFont(m_Font);
	m_TitleText.setString("TOWER DEFENSE");
	m_TitleText.setCharacterSize(80);
	m_TitleText.setFillColor(sf::Color::Yellow);
	m_TitleText.setOrigin(m_TitleText.getLocalBounds().width / 2.f, m_TitleText.getLocalBounds().height / 2.f);
	m_TitleText.setPosition(LOGICAL_WIDTH / 2.f, 300.f);

	m_GameModeText.setPosition(sf::Vector2f(1280, 200));
	m_GameModeText.setFont(m_Font);
	m_GameModeText.setString("Play Mode");

	/*m_PlayerText.setPosition(sf::Vector2f(1900, 100));
	m_PlayerText.setString("Player");
	m_PlayerText.setFont(m_Font);*/
	// --- ADD THIS BLOCK TO INITIALIZE STATS UI ---

	// Common properties for all stat text
	const int statCharacterSize = 32;
	const sf::Color statTextColor = sf::Color::White;
	const sf::Color statOutlineColor = sf::Color::Black;
	const float statOutlineThickness = 2.0f;

	// Position variables
	float statStartX = 2200.f; // X position for the stats block
	float statStartY = 100.f;  // Starting Y position
	float statYSpacing = 50.f; // Spacing between each line of text

	// Level Text
	m_levelText.setFont(m_Font);
	m_levelText.setCharacterSize(statCharacterSize);
	m_levelText.setFillColor(statTextColor);
	m_levelText.setOutlineColor(statOutlineColor);
	m_levelText.setOutlineThickness(statOutlineThickness);
	m_levelText.setPosition(statStartX, statStartY);

	// Health Text
	m_healthText.setFont(m_Font);
	m_healthText.setCharacterSize(statCharacterSize);
	m_healthText.setFillColor(sf::Color::Green); // Give health a distinct color
	m_healthText.setOutlineColor(statOutlineColor);
	m_healthText.setOutlineThickness(statOutlineThickness);
	m_healthText.setPosition(statStartX, statStartY + statYSpacing * 1);

	// Gold Text
	m_goldText.setFont(m_Font);
	m_goldText.setCharacterSize(statCharacterSize);
	m_goldText.setFillColor(sf::Color::Yellow); // Give gold a distinct color
	m_goldText.setOutlineColor(statOutlineColor);
	m_goldText.setOutlineThickness(statOutlineThickness);
	m_goldText.setPosition(statStartX, statStartY + statYSpacing * 2);

	// Difficulty Text
	m_difficultyText.setFont(m_Font);
	m_difficultyText.setCharacterSize(statCharacterSize);
	m_difficultyText.setFillColor(statTextColor);
	m_difficultyText.setOutlineColor(statOutlineColor);
	m_difficultyText.setOutlineThickness(statOutlineThickness);
	m_difficultyText.setPosition(statStartX, statStartY + statYSpacing * 3);

	// Enemies Remaining Text
	m_enemiesRemainingText.setFont(m_Font);
	m_enemiesRemainingText.setCharacterSize(statCharacterSize);
	m_enemiesRemainingText.setFillColor(statTextColor);
	m_enemiesRemainingText.setOutlineColor(statOutlineColor);
	m_enemiesRemainingText.setOutlineThickness(statOutlineThickness);
	m_enemiesRemainingText.setPosition(statStartX, statStartY + statYSpacing * 4);

	// --- END OF NEW BLOCK ---

	m_GameOverText.setPosition(sf::Vector2f(1280, 800));
	m_GameOverText.setString("GAME OVERRR");
	m_GameOverText.setFont(m_Font);
	m_GameOverText.setCharacterSize(100);

	m_TileMapTexture.loadFromFile("assets/images/TileMap.png");
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			sf::Sprite tileSprite;
			tileSprite.setTexture(m_TileMapTexture);
			tileSprite.setTextureRect(sf::IntRect(i * 16, j * 16, 16, 16)); 
			tileSprite.setScale(sf::Vector2f(10, 10));
			tileSprite.setOrigin(sf::Vector2f(8, 8));

			TileOptions::TileType eTileType = TileOptions::TileType::Null;

			if (j == 0) {
				eTileType = TileOptions::TileType::Aesthetic;
			} else {
				if (j == 1) {
					if (i == 0){
						eTileType = TileOptions::TileType::Spawn;
					} else if (i == 1) {
						eTileType = TileOptions::TileType::End;
					} else if (i == 2) {
						eTileType = TileOptions::TileType::Path;
					}
				}
			}
			
			TileOptions& tileOption = m_TileOptions.emplace_back(eTileType);
			tileOption.setSprite(tileSprite);
		}
	}

	// Initialize Buttons
	sf::Vector2f buttonSize(300, 60);
	m_NewGameButton = std::make_unique<Button>("New Game", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 500.f), buttonSize);
	m_LoadGameButton = std::make_unique<Button>("Load Game", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 600.f), buttonSize);
	m_SettingsButton = std::make_unique<Button>("Settings", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 700.f), buttonSize);
	m_ExitButton = std::make_unique<Button>("Exit", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 800.f), buttonSize);

	m_ResumeButton = std::make_unique<Button>("Resume", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 500.f), buttonSize);
	m_BackToMenuButton = std::make_unique<Button>("Main Menu", m_Font, sf::Vector2f(LOGICAL_WIDTH / 2.f - 150.f, 600.f), buttonSize);

	// Start background music
	SoundManager::getInstance().playMusic("assets/audio/music/background_music.mp3");

	// ADDED: Set up the initial view for the window size
	resizeView(m_Window.getSize().x, m_Window.getSize().y);
}

Game::~Game() {}

// ADDED: Implementation for the resize function
void Game::resizeView(unsigned int width, unsigned int height) {
	float windowRatio = static_cast<float>(width) / static_cast<float>(height);
	float viewRatio = static_cast<float>(LOGICAL_WIDTH) / static_cast<float>(LOGICAL_HEIGHT);
	float sizeX = 1.0f;
	float sizeY = 1.0f;
	float posX = 0.0f;
	float posY = 0.0f;

	bool horizontalSpacing = true;
	if (windowRatio < viewRatio)
		horizontalSpacing = false;

	// If horizontalSpacing is true, black bars on top and bottom
	if (horizontalSpacing) {
		sizeX = viewRatio / windowRatio;
		posX = (1.0f - sizeX) / 2.0f;
	}
	// Otherwise, black bars on left and right
	else {
		sizeY = windowRatio / viewRatio;
		posY = (1.0f - sizeY) / 2.0f;
	}

	m_gameView.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
}

void Game::run() {
	sf::Clock clock;
	while (m_Window.isOpen()) {
		m_deltaTime = clock.restart();
		HandleInput();
		switch (m_eGameMode) {
			case MainMenu:    UpdateMainMenu();    break;
			case GetReady:    UpdateGetReady();    break;
			case Play:        UpdatePlay();        break;
			case Paused:      UpdatePaused();      break;
			case GameOver:    UpdateGameOver();    break;
			case LevelEditor: UpdateLevelEditor(); break;
		}
		Draw();
	}
}

void Game::UpdatePlay() {
	if (m_fTimeInPlayMode >= m_fLevelTimeGoal) {
		WinGame(); // You've won the level!
		return;    // Stop updating this frame, the next level will load.
	}

	m_fTimeInPlayMode += m_deltaTime.asSeconds();
	m_fDifficulty += m_deltaTime.asSeconds() / 10.0f;

	if (m_iPlayerHealth <= 0) {
		SoundManager::getInstance().playSfx(SoundManager::SfxType::Defeat);
		m_eGameMode = GameOver;
		return; // Stop updating the play state
	}

	DamageTextManager::getInstanceNonConst().Update(m_deltaTime);
	UpdateTower();
	UpdateAxe();

	const int iMaxEnemies = 30;
	if (m_SpawnTiles.size() > 0 && !m_Paths.empty()) {
		m_enemyTemplate.SetPosition(m_SpawnTiles[0].GetPosition());
		if (m_enemies.size() < iMaxEnemies) {
			static float fSpawnTimer = 0.0f;
			//Speed up the Spawn Rate after 5 seconds
			float fSpawnRate = m_fDifficulty;
			// After 1 minutes, the spawn rate will be 2.2f
			fSpawnTimer += m_deltaTime.asSeconds() * fSpawnRate;
			if (fSpawnTimer > 1.0f) {
				// Randomly spawn enemies
				Entity& newEnemy = m_enemies.emplace_back(m_enemyTemplate);
				newEnemy.SetPathIndex(rand() % m_Paths.size()); // Assign a random path index
				fSpawnTimer = 0.0f;
			}
		}
	}

	for (int i = m_enemies.size() - 1; i >= 0; --i) {
		Entity& rEnemy = m_enemies[i];
		Path& path = m_Paths[rEnemy.GetPathIndex()];

		//Find closest PathTile to the enemy
		PathTile* pClosestTile = nullptr;
		float fClosestDistance = std::numeric_limits<float>::max();

		for (PathTile& tile : path) {
			sf::Vector2f vEnemyToTile = tile.pCurrentTile -> GetPosition() - rEnemy.GetPosition();
			float fDistance = MathHelpers::flength(vEnemyToTile);

			if (fDistance < fClosestDistance) {
			  fClosestDistance = fDistance;
			  pClosestTile = &tile;
			}
		}
		// Find the next path tile
		if (!pClosestTile || !pClosestTile -> pNextTile) continue;
		const Entity* pNextTile = pClosestTile -> pNextTile;

		if (pNextTile->GetClosestGridCoordinates() == m_EndTiles[0].GetClosestGridCoordinates()) {
			if (fClosestDistance < 40.0f) {
				// Enemy reached the end tile, remove it
				m_enemies.erase(m_enemies.begin() + i);
				//m_iPlayerHealth -= 1;
				m_fDifficulty *= 0.9f;
				continue; // Skip to the next enemy
			}
		}

		float fEnemySpeed = 250.0f;
		sf::Vector2f vEnemyToNextTile = pNextTile -> GetPosition() - rEnemy.GetPosition();
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
	for (Entity& tower : m_Towers) {
		//Check if it is time to throw an axe
		tower.m_fAttackTimer -= m_deltaTime.asSeconds();
		if (tower.m_fAttackTimer > 0.0f) continue; // Not time to throw an axe yet

		//Find the closest enemy to the tower
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
			continue; // No enemies in range
		}

		// Rotate the tower to face the enemy
		sf::Vector2f vTowerToEnemy = pClosestEnemy -> GetPosition() - tower.GetPosition();
		float fAngle = MathHelpers::Angle(vTowerToEnemy);
		tower.GetSpriteNonConst().setRotation(fAngle);

		//Create an axe and set its velocity
		Entity& newAxe = m_axes.emplace_back(m_axeTemplate);
		newAxe.SetPosition(tower.GetPosition());
		vTowerToEnemy = MathHelpers::normalize(vTowerToEnemy);
		newAxe.SetVelocity(vTowerToEnemy * 500.0f);
		SoundManager::getInstance().playSfx(SoundManager::SfxType::TowerShoot);

		//Reset the axe throw
		tower.m_fAttackTimer = 1.0f;
	}
}

void Game::UpdateAxe() {
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
			//m_iPlayerGold += 1;
			AddGold(1);
		}
	}
}

void Game::UpdateLevelEditor() {
	m_enemies.clear(); // Clear enemies in level editor mode
	m_axes.clear();
	m_Towers.clear();
	
	m_iPlayerGold = 10;
	m_iPlayerHealth = 10;
	m_iGoldGainedThisUpdate = 0;
	m_fTimeInPlayMode = 0.0f;
	m_fDifficulty = 1.0f;
	m_fGoldPerSecond = 0.0f;
	m_fGoldPerSecondTimer = 0.0f;
}

void Game::UpdateMainMenu() {
    sf::Vector2f mousePos = getMouseWorldPosition();
    m_NewGameButton->isMouseOver(mousePos);
    m_LoadGameButton->isMouseOver(mousePos);
    m_SettingsButton->isMouseOver(mousePos);
    m_ExitButton->isMouseOver(mousePos);
}

void Game::UpdatePhysics() {
	const float fMaxDeltaTime = 0.1f; // Cap the delta time to prevent large jumps
	const float fDeltaTime = std::min(m_deltaTime.asSeconds(), fMaxDeltaTime);

	vector <Entity*> AllEntities;

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
		entity -> GetPhysicsDataNonConst().ClearCollisions();
	}

	for (Entity* entity : AllEntities) {

		if (entity -> GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic) {
			entity -> move(entity -> GetPhysicsData().m_vVelocity * fDeltaTime + entity -> GetPhysicsData().m_vImpulse);
			entity -> GetPhysicsDataNonConst().ClearImpulse();

			// Check collisions
			for (Entity* otherEntity : AllEntities) {
				if (entity == otherEntity) continue; // Skip self-collision
				if (entity -> shouldIgnoreEntityForPhysics(otherEntity)) continue; // Skip ignored entities

				if (!entity -> GetPhysicsDataNonConst().HasCollidedThisUpdate(otherEntity) && isColiding(*entity, *otherEntity)) {
					entity -> OnCollision(*otherEntity);
					otherEntity -> OnCollision(*entity);

					entity -> GetPhysicsDataNonConst().AddEntityCollision(otherEntity);
					otherEntity -> GetPhysicsDataNonConst().AddEntityCollision(entity);
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
	else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle){
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
						} else {
							entity1.move(sf::Vector2f(-fOverlapX, 0));
						}
					} else {
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
					} else {
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
		} else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle) {
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
	/*sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	m_TowerTemplate.SetPosition(vMousePosition);*/

	// MODIFIED: Use the helper function
	sf::Vector2f vMousePosition = getMouseWorldPosition();
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

	/*for (const Entity& enemy : m_enemies) {
		m_Window.draw(enemy);
	}*/

	for (Entity& enemy : m_enemies) { // Note: changed to non-const to update timer
		if (enemy.m_fFlashTimer > 0) {
			enemy.GetSpriteNonConst().setColor(sf::Color::Red);
			enemy.m_fFlashTimer -= m_deltaTime.asSeconds();
		}
		else {
			enemy.GetSpriteNonConst().setColor(sf::Color::White);
		}
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

	// Calculate time remaining
	float timeRemaining = m_fLevelTimeGoal - m_fTimeInPlayMode;

    // In your Game constructor, after loading m_Font, initialize m_playingTimeText:
    m_playingTimeText.setFont(m_Font);
    m_playingTimeText.setCharacterSize(32);
    m_playingTimeText.setFillColor(sf::Color::Cyan);
    m_playingTimeText.setOutlineColor(sf::Color::Black);
    m_playingTimeText.setOutlineThickness(2.0f);
    m_playingTimeText.setPosition(2200.f, 100.f + 50.f * 5); // Below the other stats

    // In DrawPlay(), after calculating timeRemaining, add:
    float playingTime = m_fTimeInPlayMode;
    int playMinutes = static_cast<int>(playingTime) / 60;
    int playSeconds = static_cast<int>(playingTime) % 60;
    char playTimeBuffer[10];
    sprintf_s(playTimeBuffer, "%02d:%02d", playMinutes, playSeconds);
    m_playingTimeText.setString(std::string("Time: ") + playTimeBuffer);

    // After drawing the other stat texts, add:
    m_Window.draw(m_playingTimeText);
	if (timeRemaining < 0) timeRemaining = 0;

	// Convert seconds to a MM:SS format
	int minutes = static_cast<int>(timeRemaining) / 60;
	int seconds = static_cast<int>(timeRemaining) % 60;
	char timeBuffer[10];
	sprintf_s(timeBuffer, "%02d:%02d", minutes, seconds); // Use sprintf_s for safety in VS

	/*m_PlayerText.setString("Difficulty: " + to_string(m_fDifficulty) + 
		"\nPlayer's Gold: " + to_string(m_iPlayerGold) + 
		"\nGold Per Second: " + to_string(m_fGoldPerSecond));
	m_Window.draw(m_PlayerText);*/

	// Update the strings with the latest values
	m_levelText.setString("Level: " + std::to_string(m_iCurrentLevel));
	m_healthText.setString("Health: " + std::to_string(m_iPlayerHealth));
	m_goldText.setString("Gold: " + std::to_string(m_iPlayerGold));
	m_difficultyText.setString("Difficulty: " + std::to_string((int)(m_fDifficulty * 100.f) / 100.f)); // Format to 2 decimal places
	m_enemiesRemainingText.setString("Enemies: " + std::to_string(m_enemies.size()));

	// Draw all the stat text objects
	m_Window.draw(m_levelText);
	m_Window.draw(m_healthText);
	m_Window.draw(m_goldText);
	m_Window.draw(m_difficultyText);
	m_Window.draw(m_enemiesRemainingText);
}

void Game::Draw() {
	// Erase the previous frame
	m_Window.clear(sf::Color::Black);

	// ADDED: Set the view for the game world
	m_Window.setView(m_gameView);

	// --- CHANGE #1: Draw the loaded background map first ---
	if (m_eGameMode == GetReady || m_eGameMode == Play || m_eGameMode == Paused || m_eGameMode == GameOver) {
		m_Window.draw(m_backgroundSprite);
	}

	// --- CHANGE #2: REMOVE the old loop for drawing aesthetic tiles ---
	// The aesthetic tiles are now handled by the background sprite, so we don't need to draw them separately.
	//

	// Draw background/common elements if any
	
	for (const Entity& entity : m_AestheticTiles) {
		m_Window.draw(entity);
	}
	
	// The background PNG now handles the visuals for aesthetic areas.
	//
	//Draw the game mode text for debugging purposes
	//m_Window.draw(m_GameModeText);

	switch (m_eGameMode) {
		case MainMenu:    DrawMainMenu();    break;
		case GetReady:    DrawGetReady();    break;
		case Play:        DrawPlay();        break;
		case Paused:      DrawPaused();      break;
		case GameOver:    DrawGameOver();    break;
		case LevelEditor: DrawLevelEditor(); break;
	}

	// If you had UI that should NOT scale, you would reset the view here:
	// m_Window.setView(m_Window.getDefaultView());
	// m_Window.draw(my_non_scaling_ui_element);

	m_Window.display();
}

void Game::HandleInput() {
	/*static bool bTwasPressedLastUpdate = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T)) {
		if (!bTwasPressedLastUpdate) {
			if (m_eGameMode == Play) {
				m_eGameMode = LevelEditor;
				m_GameModeText.setString("Level Editor Mode");
			} else {
				m_eGameMode = Play;
				m_GameModeText.setString("Play Mode");
			}
		}
		bTwasPressedLastUpdate = true;
	}
	else {
		bTwasPressedLastUpdate = false;
	}*/

	sf::Event event;
	m_eScrollWheelInput = None;
	while (m_Window.pollEvent(event)) {
		switch (event.type) {
		case sf::Event::Closed:
			m_Window.close();
			break;

		case sf::Event::Resized: // This was also part of the previous change
			resizeView(event.size.width, event.size.height);
			break;

			// --- ADD THIS ENTIRE CASE FOR THE ESCAPE KEY ---
		case sf::Event::KeyPressed:
			// If we are in the game over state, ANY key press will return to the menu.
			if (m_eGameMode == GameOver) {
				m_eGameMode = MainMenu;
			}
			// Handle pausing with Escape key
			else if (event.key.code == sf::Keyboard::Escape) {
				if (m_eGameMode == Play) {
					m_eGameMode = Paused;
				}
				else if (m_eGameMode == Paused) {
					m_eGameMode = Play;
				}
			}
			break;
			// --- END OF ADDED CODE FOR ESCAPE KEY ---

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
		}
	}

	switch (m_eGameMode) {
		case MainMenu:    HandleMainMenuInput();    break;
		case GetReady:    HandleGetReadyInput();    break;
		case Play:        HandlePlayInput();        break;
		case Paused:      HandlePausedInput();      break;
		case GameOver:    HandleGameOverInput();    break;
		case LevelEditor: HandleLevelEditorInput(); break;
	}
}

void Game::CreateTileAtPosition(const sf::Vector2f& pos) {
	int x = pos.x / 160;
	int y = pos.y / 160;

	TileOptions::TileType eTileType = m_TileOptions[m_optionIndex].getTileType();
	if (eTileType == TileOptions::TileType::Null) return;

	vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

	if (eTileType == TileOptions::TileType::Spawn || eTileType == TileOptions::TileType::End) {
		ListOfTiles.clear(); // Clear existing spawn or end tiles (if more than 1)
	}

	sf::Sprite tile = m_TileOptions[m_optionIndex].getSprite();
	tile.setPosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < ListOfTiles.size(); i++) {
		if (ListOfTiles[i].GetPosition() == tile.getPosition()) {
			ListOfTiles[i] = ListOfTiles.back(); // Move the last tile to the current position
			ListOfTiles.pop_back(); // Remove the last tile
			break; // Tile already exists at this position, do not add a duplicate
		}
	}

	Entity& new_tiles = ListOfTiles.emplace_back(Entity::PhysicsData::Type::Static);
	new_tiles.SetSprite(tile);
	new_tiles.setRectanglePhysics(160.0f, 160.0f);
	ConstructionPath();
}

void Game::DeleteTileAtPosition(const sf::Vector2f& pos) {
	int x = pos.x / 160;
	int y = pos.y / 160;

	// Calculate the tile position based on the grid size (160x160)
	sf::Vector2f tilePosition(x * 160 + 80, y * 160 + 80);

	TileOptions::TileType eTileType = m_TileOptions[m_optionIndex].getTileType();
	if (eTileType == TileOptions::TileType::Null) return;
	vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

	for (int i = 0; i < ListOfTiles.size(); i++) {
		if (ListOfTiles[i].GetPosition() == tilePosition) {
			ListOfTiles[i] = ListOfTiles.back(); // Move the last tile to the current position
			ListOfTiles.pop_back(); // Remove the last tile
			break; // Tile found and removed
		}
	}
}

void Game::ConstructionPath() {
	m_Paths.clear();
	if (m_SpawnTiles.empty() || m_EndTiles.empty()) {
		return;
	}

	Path newPath;
	PathTile& start = newPath.emplace_back();
	start.pCurrentTile = &m_SpawnTiles[0];

	sf::Vector2i vEndCoords = m_EndTiles[0].GetClosestGridCoordinates();
	VisitPathNeighbors(newPath, vEndCoords);
}

void Game::VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords) {
	const sf::Vector2i vCurrentTilePosition = path.back().pCurrentTile -> GetClosestGridCoordinates();
	
	const sf::Vector2i vNorthCoords(vCurrentTilePosition.x, vCurrentTilePosition.y - 1);
	const sf::Vector2i vEastCoords(vCurrentTilePosition.x + 1, vCurrentTilePosition.y);
	const sf::Vector2i vSouthCoords(vCurrentTilePosition.x, vCurrentTilePosition.y + 1);
	const sf::Vector2i vWestCoords(vCurrentTilePosition.x - 1, vCurrentTilePosition.y);

	if (rEndCoords == vNorthCoords || rEndCoords == vEastCoords || rEndCoords == vSouthCoords || rEndCoords == vWestCoords) {
		// Set the last tile in our current path to point to the next tile
		path.back().pNextTile = &m_EndTiles[0];
		// Add the next tile, and set it.
		PathTile& newTile = path.emplace_back();
		newTile.pCurrentTile = &m_EndTiles[0];
		m_Paths.push_back(path);

		// If any of our paths are next to the end tile, they should probably go straight to end and terminate.
		// If we didn't return here, we could move around the end tile before going into it.
		return;
	}

	const vector<Entity>& pathTiles = GetListOfTiles(TileOptions::TileType::Path);

	for (const Entity& pathTile : pathTiles) {
		const sf::Vector2i vPathTileCoords = pathTile.GetClosestGridCoordinates();

		if (DoesPathContainCoordinates(path, vPathTileCoords)) {
			continue; // Skip if the path already contains this tile
		}

		if (vPathTileCoords == vNorthCoords || vPathTileCoords == vEastCoords || vPathTileCoords == vSouthCoords || vPathTileCoords == vWestCoords) {
			// We have a neighbor tile
			Path newPath = path; // Create a copy of the current path
			newPath.back().pNextTile = &pathTile; // Set the next tile in the path
			PathTile& newTile = newPath.emplace_back();
			newTile.pCurrentTile = &pathTile;

			if (vPathTileCoords == rEndCoords) {
				// We reached the end tile
				m_Paths.push_back(newPath);
			} else {
				// Continue visiting neighbors
				VisitPathNeighbors(newPath, rEndCoords);
			}
		}
	}
}

bool Game::DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coords) {
	for (const PathTile& tile : path) {
		if (tile.pCurrentTile -> GetClosestGridCoordinates() == coords) {
			return true; // Found a tile with the same coordinates
		}
	}
	return false; // No tile with the same coordinates found
}

void Game::DrawLevelEditor() {
	/*sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	m_TileOptions[m_optionIndex].setPosition(vMousePosition);*/
	// MODIFIED: Use the helper function
	sf::Vector2f vMousePosition = getMouseWorldPosition();
	m_TileOptions[m_optionIndex].setPosition(vMousePosition);

	TileOptions::TileType eTileType = m_TileOptions[m_optionIndex].getTileType();

	if (m_bDrawPath) {
		for (const Entity& entity : m_SpawnTiles) {
			m_Window.draw(entity);
		}

		for (const Entity& entity : m_EndTiles) {
			m_Window.draw(entity);
		}

		for (const Entity& entity : m_PathTiles) {
			m_Window.draw(entity);
		}
	}
	m_Window.draw(m_TileOptions[m_optionIndex]);
}

void Game::DrawMainMenu() {
	m_Window.draw(m_TitleText);
	m_NewGameButton->draw(m_Window);
	m_LoadGameButton->draw(m_Window);
	m_SettingsButton->draw(m_Window);
	m_ExitButton->draw(m_Window);
}

void Game::HandlePlayInput() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		/*sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);*/
		// MODIFIED: Use the helper function
		sf::Vector2f vMousePosition = getMouseWorldPosition();
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
		if (m_optionIndex >= m_TileOptions.size()) {
			m_optionIndex = 0;
		}
	}
	else if (m_eScrollWheelInput == ScrollDown) {
		m_optionIndex--;
		if (m_optionIndex < 0) {
			m_optionIndex = m_TileOptions.size() - 1;
		}
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		/*sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
		CreateTileAtPosition(vMousePosition);*/
		// MODIFIED: Use the helper function
		sf::Vector2f vMousePosition = getMouseWorldPosition();
		CreateTileAtPosition(vMousePosition);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
		/*sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
		DeleteTileAtPosition(vMousePosition);*/
		// MODIFIED: Use the helper function
		sf::Vector2f vMousePosition = getMouseWorldPosition();
		DeleteTileAtPosition(vMousePosition);
	}
}

void Game::HandleMainMenuInput() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2f mousePos = getMouseWorldPosition();
		if (m_NewGameButton->isMouseOver(mousePos)) {
			m_NewGameButton->handleClick();
			StartNewGame();
		}
		if (m_LoadGameButton->isMouseOver(mousePos)) {
			m_LoadGameButton->handleClick();
			LoadGame("savegame.txt"); // For now, one save slot
		}
		// ... handle settings button
		if (m_ExitButton->isMouseOver(mousePos)) {
			m_ExitButton->handleClick();
			m_Window.close();
		}
	}
}

vector<Entity>& Game::GetListOfTiles(TileOptions::TileType eTileType) {
	switch (eTileType) {
		case TileOptions::TileType::Aesthetic:
			return m_AestheticTiles;
		case TileOptions::TileType::Spawn:
			return m_SpawnTiles;
		case TileOptions::TileType::End:
			return m_EndTiles;
		case TileOptions::TileType::Path:
			return m_PathTiles;
	}
	return m_AestheticTiles; // Default return if no match found
}

bool Game::CreateTowerAtPosition(const sf::Vector2f& pos) {
	if (CanPlaceTowerAtPosition(pos)) {
		Entity newTower = m_TowerTemplate;
		newTower.SetColor(sf::Color::White);
		m_Towers.push_back(newTower);
		return true;
	}
	return false;
}

//bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
//	sf::IntRect brickRect(0, 0, 16, 16);
//	vector<Entity>& ListOfTiles = GetListOfTiles(TileOptions::TileType::Aesthetic);
//	bool isOnBrick = false;
//	Entity copyOfTowerWithRadiusOf1 = m_TowerTemplate;
//	copyOfTowerWithRadiusOf1.setCirclePhysics(1.0f);
//
//	for (const Entity& tile : ListOfTiles) {
//		const sf::Sprite& rTileSprite = tile.GetSprite();
//		sf::IntRect tileRect = rTileSprite.getTextureRect();
//
//		if (tileRect != brickRect) {
//			continue;
//		}
//
//		if (isColiding(tile, copyOfTowerWithRadiusOf1)) {
//			isOnBrick = true;
//			break;
//		}
//	}
//
//	if (!isOnBrick) {
//		return false;
//	}
//
//	for (const Entity& tower : m_Towers) {
//		if (isColiding(tower, m_TowerTemplate)) {
//			return false;
//		}
//	}
//	return true;
//}

//bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
//	// We need to update the template's position for the collision checks below.
//	m_TowerTemplate.SetPosition(pos);
//
//	// --- Condition 1: Is the mouse over a valid placement tile? ---
//
//	// Create a tiny, temporary "test" tower at the mouse position.
//	// We use this to check for collision with the aesthetic tiles.
//	Entity copyOfTowerAtMouse = m_TowerTemplate;
//	copyOfTowerAtMouse.setCirclePhysics(1.0f); // A small radius is fine for this check.
//
//	bool isOnAestheticTile = false;
//	// Get the list of invisible tiles loaded from 'A' characters.
//	const vector<Entity>& aestheticTiles = GetListOfTiles(TileOptions::TileType::Aesthetic);
//
//	for (const Entity& tile : aestheticTiles) {
//		// We just check for a simple collision between our test tower and the invisible tile area.
//		if (isColiding(tile, copyOfTowerAtMouse)) {
//			isOnAestheticTile = true;
//			break; // We found a valid tile, no need to check further.
//		}
//	}
//
//	if (!isOnAestheticTile) {
//		return false; // If not on a valid tile, we can't build.
//	}
//
//	// --- Condition 2: Does it overlap with an existing tower? ---
//	for (const Entity& existingTower : m_Towers) {
//		// Here we check for collision between the full-sized tower template and existing towers.
//		if (isColiding(existingTower, m_TowerTemplate)) {
//			return false; // Overlaps with another tower, can't build.
//		}
//	}
//
//	// If both checks pass, we can place the tower!
//	return true;
//}

bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& pos) {
	// Update the position of the tower template that follows the mouse
	m_TowerTemplate.SetPosition(pos);

	// --- Check 1: Is the mouse over a valid "Aesthetic" tile area? ---

	// Create a tiny, temporary entity at the mouse position to act as a "test point".
	Entity testPoint = m_TowerTemplate;
	testPoint.setCirclePhysics(1.0f); // A 1-pixel radius is perfect for this check.

	bool onValidTile = false;
	// Get the list of invisible tiles we loaded from the 'A' characters in the map.txt file.
	const std::vector<Entity>& aestheticTiles = GetListOfTiles(TileOptions::TileType::Aesthetic);

	for (const Entity& tile : aestheticTiles) {
		// Check if our mouse "test point" is colliding with one of the valid placement areas.
		if (isColiding(tile, testPoint)) {
			onValidTile = true;
			break; // Found one, no need to check the rest.
		}
	}

	if (!onValidTile) {
		return false; // If we're not on a valid 'A' tile, we can't build.
	}

	// --- Check 2: Does it overlap with an already built tower? ---
	for (const Entity& existingTower : m_Towers) {
		if (isColiding(existingTower, m_TowerTemplate)) {
			return false; // It overlaps, so we can't build.
		}
	}

	// If we passed both checks, it's a valid position!
	return true;
}

void Game::AddGold(int gold) {
	m_iPlayerGold += gold;
	m_iGoldGainedThisUpdate += gold;
}

void Game::UpdatePaused() {
	sf::Vector2f mousePos = getMouseWorldPosition();
	m_ResumeButton->isMouseOver(mousePos);
	m_BackToMenuButton->isMouseOver(mousePos);
}

void Game::DrawPaused() {
	// Draw the play state in the background to show it's paused
	DrawPlay();

	// Draw a semi-transparent overlay
	sf::RectangleShape overlay(sf::Vector2f(LOGICAL_WIDTH, LOGICAL_HEIGHT));
	overlay.setFillColor(sf::Color(0, 0, 0, 150));
	m_Window.draw(overlay);

	// Draw pause menu buttons
	m_ResumeButton->draw(m_Window);
	m_BackToMenuButton->draw(m_Window);
}

void Game::HandlePausedInput() {
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		sf::Vector2f mousePos = getMouseWorldPosition();
		if (m_ResumeButton->isMouseOver(mousePos)) {
			m_ResumeButton->handleClick();
			m_eGameMode = Play;
		}
		if (m_BackToMenuButton->isMouseOver(mousePos)) {
			m_BackToMenuButton->handleClick();
			SaveGame("savegame.txt"); // Auto-save on exit
			m_eGameMode = MainMenu;
		}
	}
}

// In game.cpp

void Game::StartNewGame() {
	m_iCurrentLevel = 1;
	m_iPlayerGold = 10; // Starting gold
	m_iPlayerHealth = 10; // Starting health
	m_fDifficulty = 1.0f;
	m_Towers.clear();
	m_enemies.clear();
	m_axes.clear();
	LoadLevel(m_iCurrentLevel);
}

//void Game::LoadLevel(int levelNumber) {
//	m_iCurrentLevel = levelNumber;
//	m_AestheticTiles.clear();
//	m_SpawnTiles.clear();
//	m_EndTiles.clear();
//	m_PathTiles.clear();
//
//	std::string mapFile = "assets/maps/map" + std::to_string(levelNumber) + ".txt";
//	std::ifstream infile(mapFile);
//	if (!infile) {
//		throw std::runtime_error("Could not open map file: " + mapFile);
//	}
//
//	char tileChar;
//	for (int y = 0; y < 10; ++y) { // 1600 / 160 = 10 tiles high
//		for (int x = 0; x < 16; ++x) { // 2560 / 160 = 16 tiles wide
//			infile >> tileChar;
//			if (tileChar == '.') continue; // Skip empty space
//
//			TileOptions::TileType type;
//			int tileIndex = 0; // Index in m_TileOptions
//
//			switch (tileChar) {
//			case 'A': type = TileOptions::Aesthetic; tileIndex = 0; break;
//			case 'S': type = TileOptions::Spawn;     tileIndex = 4; break;
//			case 'E': type = TileOptions::End;       tileIndex = 5; break;
//			case 'P': type = TileOptions::Path;      tileIndex = 6; break;
//			default: continue;
//			}
//
//			vector<Entity>& tileList = GetListOfTiles(type);
//			Entity& newTile = tileList.emplace_back(Entity::PhysicsData::Type::Static);
//			newTile.SetSprite(m_TileOptions[tileIndex].getSprite());
//			newTile.SetPosition(sf::Vector2f(x * 160.f + 80.f, y * 160.f + 80.f));
//			newTile.setRectanglePhysics(160.f, 160.f);
//		}
//	}
//
//	ConstructionPath(); // Re-build pathfinding data
//	m_fGetReadyTimer = 3.0f; // 3 seconds before the game starts
//	m_eGameMode = GetReady; // Switch to the preview state
//}

void Game::LoadLevel(int levelNumber) {
	m_iCurrentLevel = levelNumber;
	m_AestheticTiles.clear();
	m_SpawnTiles.clear();
	m_EndTiles.clear();
	m_PathTiles.clear();
	m_Towers.clear();
	m_enemies.clear();
	m_axes.clear();

	// --- 1. Load the VISUAL background image ---
	std::string backgroundImageFile = "assets/images/maps/map" + std::to_string(levelNumber) + ".jfif";
	if (!m_backgroundTexture.loadFromFile(backgroundImageFile)) {
		throw std::runtime_error("Failed to load background image: " + backgroundImageFile);
	}
	m_backgroundSprite.setTexture(m_backgroundTexture);
	m_backgroundSprite.setPosition(0, 0); // Position at top-left corner

	// --- 2. Load the LOGICAL layout from the TXT file ---
	std::string mapLayoutFile = "assets/maps/map" + std::to_string(levelNumber) + ".txt";
	std::ifstream infile(mapLayoutFile);
	if (!infile) {
		throw std::runtime_error("Could not open map layout file: " + mapLayoutFile);
	}

	char tileChar;
	for (int y = 0; y < 10; ++y) {       // 10 tiles high (1600 / 160)
		for (int x = 0; x < 16; ++x) {   // 16 tiles wide (2560 / 160)
			infile >> tileChar;
			if (tileChar == '.') continue; // Skip empty space

			vector<Entity>* tileList = nullptr;
			TileOptions::TileType type;

			switch (tileChar) {
			case 'A': // Aesthetic / Tower placement area
				type = TileOptions::Aesthetic;
				tileList = &m_AestheticTiles;
				break;
			case 'S': // Spawn point
				type = TileOptions::Spawn;
				tileList = &m_SpawnTiles;
				break;
			case 'E': // End point
				type = TileOptions::End;
				tileList = &m_EndTiles;
				break;
			case 'P': // Path
				type = TileOptions::Path;
				tileList = &m_PathTiles;
				break;
			default:
				continue; // Skip any other characters
			}

			if (tileList) {
				Entity& newTile = tileList->emplace_back(Entity::PhysicsData::Type::Static);
				// NOTE: We do NOT call SetSprite. These tiles are invisible.
				// Their only purpose is to define an area for gameplay logic.
				newTile.SetPosition(sf::Vector2f(x * 160.f + 80.f, y * 160.f + 80.f));
				newTile.setRectanglePhysics(160.f, 160.f);
			}
		}
	}
	infile.close();

	// --- Finalize level setup ---
	ConstructionPath();      // Re-build pathfinding data
	m_fGetReadyTimer = 3.0f; // 3 seconds before the game starts
	m_eGameMode = GetReady;  // Switch to the preview state

	// --- ADD THIS LOGIC ---
	// Reset the play timer for the new level
	m_fTimeInPlayMode = 0.0f;

	// Set the survival time goal based on the level number
	switch (levelNumber) {
	case 1:
		m_fLevelTimeGoal = 30.0f; // Survive for 1 minute
		break;
	case 2:
		m_fLevelTimeGoal = 90.0f; // Survive for 1.5 minutes
		break;
	case 3:
		m_fLevelTimeGoal = 120.0f; // Survive for 2 minutes
		break;
	case 4:
		m_fLevelTimeGoal = 150.0f; // Survive for 2.5 minutes
		break;
	default:
		m_fLevelTimeGoal = 30.0f; // Default goal
		break;
	}
}

void Game::UpdateGetReady() {
	m_fGetReadyTimer -= m_deltaTime.asSeconds();
	if (m_fGetReadyTimer <= 0.f || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
		m_eGameMode = Play;
	}
}

void Game::DrawGetReady() {
	// Draw the map itself by drawing the play-state elements
	for (const auto& tile : m_PathTiles) m_Window.draw(tile);
	for (const auto& tile : m_SpawnTiles) m_Window.draw(tile);
	for (const auto& tile : m_EndTiles) m_Window.draw(tile);

	// Draw the countdown text
	sf::Text readyText;
	readyText.setFont(m_Font);
	readyText.setString("Level " + std::to_string(m_iCurrentLevel) + "\nGet Ready!\n" + std::to_string((int)ceil(m_fGetReadyTimer)));
	readyText.setCharacterSize(100);
	readyText.setFillColor(sf::Color::White);
	readyText.setOutlineColor(sf::Color::Black);
	readyText.setOutlineThickness(4.f);
	readyText.setOrigin(readyText.getLocalBounds().width / 2.f, readyText.getLocalBounds().height / 2.f);
	readyText.setPosition(LOGICAL_WIDTH / 2.f, LOGICAL_HEIGHT / 2.f);
	m_Window.draw(readyText);
}

void Game::WinGame() {
	SoundManager::getInstance().playSfx(SoundManager::SfxType::Victory);
	m_iCurrentLevel++;
	if (m_iCurrentLevel > 1) { // Assuming 4 levels total
		//Beat the game! Go to a credits screen or back to menu.
		m_eGameMode = MainMenu;
	}
	else {
		SaveGame("autosave.txt"); // Save progress
		LoadLevel(m_iCurrentLevel); // Load the next level
	}
}

// In game.cpp

void Game::SaveGame(const std::string& profileName) {
	std::ofstream savefile("assets/saves/" + profileName);
	if (!savefile) {
		std::cerr << "Error: Cannot create save file " << profileName << std::endl;
		return;
	}

	// Save Game State
	savefile << "Level " << m_iCurrentLevel << std::endl;
	savefile << "Gold " << m_iPlayerGold << std::endl;
	savefile << "Health " << m_iPlayerHealth << std::endl;
	savefile << "Difficulty " << m_fDifficulty << std::endl;

	// Save Towers
	savefile << "Towers " << m_Towers.size() << std::endl;
	for (const auto& tower : m_Towers) {
		savefile << tower.GetPosition().x << " " << tower.GetPosition().y << std::endl;
	}

	// Save Enemies
	savefile << "Enemies " << m_enemies.size() << std::endl;
	for (const auto& enemy : m_enemies) {
		savefile << enemy.GetPosition().x << " " << enemy.GetPosition().y << " ";
		savefile << enemy.getHealth() << " " << enemy.GetPathIndex() << std::endl;
	}
	std::cout << "Game Saved to " << profileName << std::endl;
}

void Game::LoadGame(const std::string& profileName) {
	std::ifstream loadfile("assets/saves/" + profileName);
	if (!loadfile) {
		std::cerr << "Error: No save file found: " << profileName << std::endl;
		// Optional: Show an error message on screen
		return;
	}

	std::string key;
	// Load Game State
	loadfile >> key >> m_iCurrentLevel;
	loadfile >> key >> m_iPlayerGold;
	loadfile >> key >> m_iPlayerHealth;
	loadfile >> key >> m_fDifficulty;

	// Load level layout first
	LoadLevel(m_iCurrentLevel);

	// Clear any default entities
	m_Towers.clear();
	m_enemies.clear();
	m_axes.clear();

	// Load Towers
	int towerCount;
	loadfile >> key >> towerCount;
	for (int i = 0; i < towerCount; ++i) {
		sf::Vector2f pos;
		loadfile >> pos.x >> pos.y;
		Entity& newTower = m_Towers.emplace_back(m_TowerTemplate);
		newTower.SetPosition(pos);
		newTower.SetColor(sf::Color::White);
	}

	// Load Enemies
	int enemyCount;
	loadfile >> key >> enemyCount;
	for (int i = 0; i < enemyCount; ++i) {
		sf::Vector2f pos;
		int health, pathIndex;
		loadfile >> pos.x >> pos.y >> health >> pathIndex;
		Entity& newEnemy = m_enemies.emplace_back(m_enemyTemplate);
		newEnemy.SetPosition(pos);
		newEnemy.SetHealth(health);
		newEnemy.SetPathIndex(pathIndex);
	}

	m_eGameMode = Play; // Go directly into the game
	std::cout << "Game Loaded from " << profileName << std::endl;
}

void Game::UpdateGameOver() {
	// This function can be used for animations or timers on the game over screen.
	// For now, it can be empty.
}

void Game::DrawGameOver() {
	// We already have a DrawPlay() that shows the final state. Let's call that.
	DrawPlay();

	// Draw a semi-transparent overlay to darken the screen
	sf::RectangleShape overlay(sf::Vector2f(LOGICAL_WIDTH, LOGICAL_HEIGHT));
	overlay.setFillColor(sf::Color(128, 0, 0, 150)); // A red tint for game over
	m_Window.draw(overlay);

	// Reuse the m_GameOverText you already created
	m_Window.draw(m_GameOverText);

	// Add a prompt to return to the menu
	sf::Text promptText;
	promptText.setFont(m_Font);
	promptText.setString("Press any key to return to Main Menu");
	promptText.setCharacterSize(40);
	promptText.setFillColor(sf::Color::White);
	promptText.setOrigin(promptText.getLocalBounds().width / 2.f, promptText.getLocalBounds().height / 2.f);
	promptText.setPosition(LOGICAL_WIDTH / 2.f, m_GameOverText.getPosition().y + 150.f);
	m_Window.draw(promptText);
}

void Game::HandleGetReadyInput() {
	// This function can be used to allow the player to skip the countdown.
	// The logic for this is already in UpdateGetReady(), so this can be empty for now.
}

void Game::HandleGameOverInput() {
	// Check if any key has been pressed to go back to the menu
	// This is a simple way to detect a key press event
	//static bool keyWasPressed = true; // Start as true to ignore keys held down
	//if (sf.Mouse.isButtonPressed(sf.Mouse.Left) || sf.Keyboard.isSomethingPressed()) { // Not a real function, see below
	//	if (!keyWasPressed) {
	//		m_eGameMode = MainMenu;
	//	}
	//}
	//else {
	//	keyWasPressed = false;
	//}
}