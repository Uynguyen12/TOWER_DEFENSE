#include "Map.h"
#include <stdexcept>
#include <iostream>

Map::Map() {
}

Map::~Map() {
}

void Map::Initialize(const std::string& mapTextureFilePath) {
    if (!m_MapTexture.loadFromFile(mapTextureFilePath)) {
        throw std::runtime_error("Failed to load map texture from '" + mapTextureFilePath + "'");
    }
    m_MapSprite.setTexture(m_MapTexture);
}

void Map::ConstructPath(const MapGrid& mapGrid) {
    m_Paths.clear();
    const auto& nodeMatrix = mapGrid.getNodeMatrix();
    if (nodeMatrix.empty()) {
        return;
    }   

    // Find spawn and end coordinates
    sf::Vector2i spawnCoords(-1, -1);
    sf::Vector2i endCoords(-1, -1);
    for (int y = 0; y < mapGrid.getHeight(); ++y) {
        for (int x = 0; x < mapGrid.getWidth(); ++x) {
            if (nodeMatrix[y][x] == MapGrid::TileType::Spawn) {
                spawnCoords = sf::Vector2i(x, y);
            }
            else if (nodeMatrix[y][x] == MapGrid::TileType::End) {
                endCoords = sf::Vector2i(x, y);
            }
        }
    }

    if (spawnCoords == sf::Vector2i(-1, -1) || endCoords == sf::Vector2i(-1, -1)) {
        std::cout << "No spawn or end tile found in map grid." << std::endl;
        return;
    }

    Path newPath;
    PathTile& start = newPath.emplace_back();
    start.coords = spawnCoords;
    start.nextCoords = spawnCoords; // Will be updated in VisitPathNeighbors
    VisitPathNeighbors(newPath, endCoords, mapGrid);
}

void Map::VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords, const MapGrid& mapGrid) {
    const sf::Vector2i vCurrentCoords = path.back().coords;

    const sf::Vector2i vNorthCoords(vCurrentCoords.x, vCurrentCoords.y - 1);
    const sf::Vector2i vEastCoords(vCurrentCoords.x + 1, vCurrentCoords.y);
    const sf::Vector2i vSouthCoords(vCurrentCoords.x, vCurrentCoords.y + 1);
    const sf::Vector2i vWestCoords(vCurrentCoords.x - 1, vCurrentCoords.y);

    // Check if we reached the end
    if (vCurrentCoords == rEndCoords) {
        m_Paths.push_back(path);
        return;
    }

    // Check neighboring coordinates
    std::vector<sf::Vector2i> neighbors = { vNorthCoords, vEastCoords, vSouthCoords, vWestCoords };
    for (const auto& neighborCoords : neighbors) {
        if (!IsValidGridCoord(neighborCoords, mapGrid)) {
            continue;
        }
        if (DoesPathContainCoordinates(path, neighborCoords)) {
            continue;
        }
        if (!IsPathTile(neighborCoords, mapGrid) && neighborCoords != rEndCoords) {
            continue;
        }

        Path newPath = path;
        newPath.back().nextCoords = neighborCoords;
        PathTile& newTile = newPath.emplace_back();
        newTile.coords = neighborCoords;
        newTile.nextCoords = neighborCoords; // Will be updated in next iteration
        VisitPathNeighbors(newPath, rEndCoords, mapGrid);
    }
}

bool Map::DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates) const {
    for (const PathTile& tile : path) {
        if (tile.coords == coordinates) {
            return true;
        }
    }
    return false;
}

bool Map::IsValidGridCoord(const sf::Vector2i& coords, const MapGrid& mapGrid) const {
    return coords.x >= 0 && coords.x < mapGrid.getWidth() && coords.y >= 0 && coords.y < mapGrid.getHeight();
}

bool Map::IsPathTile(const sf::Vector2i& coords, const MapGrid& mapGrid) const {
    const auto& nodeMatrix = mapGrid.getNodeMatrix();
    return nodeMatrix[coords.y][coords.x] == MapGrid::TileType::Path;
}

void Map::Draw(sf::RenderWindow& window) const {
    window.draw(m_MapSprite);
}