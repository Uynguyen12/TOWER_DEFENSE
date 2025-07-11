#include "Map.h"
#include <stdexcept>
#include <iostream>

Map::Map() {
}

Map::~Map() {
}

void Map::Initialize() {
    // Load tile map texture
    if (!m_TileMapTexture.loadFromFile("image/TileMap.png")) {
        throw std::runtime_error("Failed to load tilemap texture from 'image/TileMap.png'");
    }

    // Initialize tile options
    m_TileOptions.clear();

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
            }
            else {
                if (j == 1) {
                    if (i == 0) {
                        eTileType = TileOptions::TileType::Spawn;
                    }
                    else if (i == 1) {
                        eTileType = TileOptions::TileType::End;
                    }
                    else if (i == 2) {
                        eTileType = TileOptions::TileType::Path;
                    }
                }
            }

            TileOptions& tileOption = m_TileOptions.emplace_back(eTileType);
            tileOption.setSprite(tileSprite);
        }
    }
}

void Map::CreateTileAtPosition(const sf::Vector2f& pos, int optionIndex) {
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }

    int x = pos.x / 160;
    int y = pos.y / 160;

    TileOptions::TileType eTileType = m_TileOptions[optionIndex].getTileType();
    if (eTileType == TileOptions::TileType::Null) return;

    std::vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

    if (eTileType == TileOptions::TileType::Spawn || eTileType == TileOptions::TileType::End) {
        ListOfTiles.clear(); // Clear existing spawn or end tiles (if more than 1)
    }

    sf::Sprite tile = m_TileOptions[optionIndex].getSprite();
    tile.setPosition(x * 160 + 80, y * 160 + 80);

    // Check if tile already exists at this position
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

    ConstructPath();
}

void Map::DeleteTileAtPosition(const sf::Vector2f& pos, int optionIndex) {
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }

    int x = pos.x / 160;
    int y = pos.y / 160;

    // Calculate the tile position based on the grid size (160x160)
    sf::Vector2f tilePosition(x * 160 + 80, y * 160 + 80);

    TileOptions::TileType eTileType = m_TileOptions[optionIndex].getTileType();
    if (eTileType == TileOptions::TileType::Null) return;

    std::vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

    for (int i = 0; i < ListOfTiles.size(); i++) {
        if (ListOfTiles[i].GetPosition() == tilePosition) {
            ListOfTiles[i] = ListOfTiles.back(); // Move the last tile to the current position
            ListOfTiles.pop_back(); // Remove the last tile
            break; // Tile found and removed
        }
    }

    ConstructPath();
}

std::vector<Entity>& Map::GetListOfTiles(TileOptions::TileType eTileType) {
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

const std::vector<Entity>& Map::GetListOfTiles(TileOptions::TileType eTileType) const {
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

void Map::ConstructPath() {
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

void Map::VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords) {
    const sf::Vector2i vCurrentTilePosition = path.back().pCurrentTile->GetClosestGridCoordinates();

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

    const std::vector<Entity>& pathTiles = GetListOfTiles(TileOptions::TileType::Path);

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
            }
            else {
                // Continue visiting neighbors
                VisitPathNeighbors(newPath, rEndCoords);
            }
        }
    }
}

bool Map::DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coords) const {
    for (const PathTile& tile : path) {
        if (tile.pCurrentTile->GetClosestGridCoordinates() == coords) {
            return true; // Found a tile with the same coordinates
        }
    }
    return false; // No tile with the same coordinates found
}

void Map::DrawTiles(sf::RenderWindow& window) const {
    for (const Entity& entity : m_AestheticTiles) {
        window.draw(entity);
    }
}

void Map::DrawLevelEditor(sf::RenderWindow& window, int optionIndex, const sf::Vector2f& mousePos) const {
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }

    // Draw existing tiles
    for (const Entity& entity : m_SpawnTiles) {
        window.draw(entity);
    }

    for (const Entity& entity : m_EndTiles) {
        window.draw(entity);
    }

    for (const Entity& entity : m_PathTiles) {
        window.draw(entity);
    }

    // Draw tile preview at mouse position
    TileOptions tilePreview = m_TileOptions[optionIndex];
    tilePreview.setPosition(mousePos);
    window.draw(tilePreview);
}