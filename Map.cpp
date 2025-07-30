#include "Map.h"
#include <stdexcept>
#include <iostream>

Map::Map() {
}

Map::~Map() {
}

void Map::Initialize(const std::string& textureFilePath) {
    // Load tile map texture
    if (!m_TileMapTexture.loadFromFile(textureFilePath)) {
        throw std::runtime_error("Failed to load tilemap texture from '" + textureFilePath + "'");
    }

    // Initialize tile options for display purposes only
    m_TileOptions.clear();

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            sf::Sprite tileSprite;
            tileSprite.setTexture(m_TileMapTexture);
            tileSprite.setTextureRect(sf::IntRect(i * 16, j * 16, 16, 16));
            float scaleFactor = TileOptions::SIZE / 16.0f;
            tileSprite.setScale(sf::Vector2f(scaleFactor, scaleFactor));

            tileSprite.setOrigin(sf::Vector2f(8, 8));

            TileOptions::TileType eTileType = TileOptions::TileType::Null;

            if (j == 0 && i == 0) {
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
    // Draw all tiles for gameplay
    for (const Entity& entity : m_AestheticTiles) {
        window.draw(entity);
    }
}

void Map::PopulateFromMapGrid(const MapGrid& mapGrid) {
    ClearTiles(); // Clear old map

    const float tileSize = TileOptions::SIZE;
    const auto& nodeMatrix = mapGrid.getNodeMatrix();

    // Prepare for faster TileOptions lookup
    std::map<TileOptions::TileType, const TileOptions*> tileOptionLookup;
    for (const auto& option : m_TileOptions) {
        tileOptionLookup[option.getTileType()] = &option;
    }

    for (int y = 0; y < mapGrid.getHeight(); ++y) {
        for (int x = 0; x < mapGrid.getWidth(); ++x) {
            TileOptions::TileType type = nodeMatrix[y][x];

            // Find TileOptions using map
            auto it = tileOptionLookup.find(type);
            const TileOptions* selectedTileOption = nullptr;
            if (it != tileOptionLookup.end()) {
                selectedTileOption = it->second;
            }

            if (selectedTileOption) {
                // Get configured sprite (with texture, textureRect, scale, origin)
                sf::Sprite tileSprite = selectedTileOption->getSprite();

                // Calculate center position of grid cell
                float tileCenterX = x * tileSize + tileSize / 2.0f;
                float tileCenterY = y * tileSize + tileSize / 2.0f;
                tileSprite.setPosition(tileCenterX, tileCenterY);

                // Create new Entity and assign positioned sprite
                Entity newTileEntity(Entity::PhysicsData::Type::Static);
                newTileEntity.SetSprite(tileSprite);

                // Add to appropriate list
                switch (type) {
                case TileOptions::TileType::Aesthetic:
                    m_AestheticTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::Spawn:
                    m_SpawnTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::End:
                    m_EndTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::Path:
                    m_PathTiles.push_back(newTileEntity);
                    break;
                default:
                    // TileType::Null or undefined types won't be added
                    break;
                }
            }
            else {
                // Warning if TileOptions not found for a tile type in MapGrid
                if (type != TileOptions::TileType::Null) { // Skip Null type
                    std::cerr << "Warning: Map could not find TileOptions for type " << static_cast<int>(type) << " from MapGrid." << std::endl;
                }
            }
        }
    }

    // After populating, construct paths for gameplay
    ConstructPath();
    std::cout << "Map populated from MapGrid and paths constructed." << std::endl;
}

// Clears all stored tile entities
void Map::ClearTiles() {
    m_AestheticTiles.clear();
    m_SpawnTiles.clear();
    m_EndTiles.clear();
    m_PathTiles.clear();
    m_Paths.clear();
}