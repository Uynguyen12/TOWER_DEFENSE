#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "TileOptions.h"
#include <vector>
#include "MapGrid.h"

class Map {
public:
    struct PathTile {
        const Entity* pCurrentTile;
        const Entity* pNextTile;
    };

    typedef std::vector<PathTile> Path;

    Map();
    ~Map();

    // Initialization
    void Initialize(const std::string& textureFilePath);

    // Tile access (read-only)
    std::vector<Entity>& GetListOfTiles(TileOptions::TileType eTileType);
    const std::vector<Entity>& GetListOfTiles(TileOptions::TileType eTileType) const;

    // Pathfinding
    void ConstructPath();
    const std::vector<Path>& GetPaths() const { return m_Paths; }

    // Tile access (read-only)
    const std::vector<Entity>& GetAestheticTiles() const { return m_AestheticTiles; }
    const std::vector<Entity>& GetSpawnTiles() const { return m_SpawnTiles; }
    const std::vector<Entity>& GetEndTiles() const { return m_EndTiles; }
    const std::vector<Entity>& GetPathTiles() const { return m_PathTiles; }

    // Tile options (read-only)
    const std::vector<TileOptions>& GetTileOptions() const { return m_TileOptions; }
    const TileOptions& GetTileOption(int index) const { return m_TileOptions[index]; }
    size_t GetTileOptionsCount() const { return m_TileOptions.size(); }

    // Drawing - only game tiles, no level editor
    void DrawTiles(sf::RenderWindow& window) const;

    // Populate map from MapGrid (read from file)
    void PopulateFromMapGrid(const MapGrid& mapGrid);
    void ClearTiles();

private:
    // Pathfinding helpers
    void VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords);
    bool DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates) const;

    // Tile collections
    std::vector<Entity> m_AestheticTiles;
    std::vector<Entity> m_SpawnTiles;
    std::vector<Entity> m_EndTiles;
    std::vector<Entity> m_PathTiles;

    // Tile options
    std::vector<TileOptions> m_TileOptions;
    sf::Texture m_TileMapTexture;

    // Pathfinding
    std::vector<Path> m_Paths;
};

#endif // MAP_H