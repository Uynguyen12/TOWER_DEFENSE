#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include "MapGrid.h"
#include <vector>

class Map {
public:
    struct PathTile {
        sf::Vector2i coords; // Grid coordinates of the tile
        sf::Vector2i nextCoords; // Grid coordinates of the next tile
    };

    typedef std::vector<PathTile> Path;

    Map();
    ~Map();

    void Initialize(const std::string& mapTextureFilePath);
    void ConstructPath(const MapGrid& mapGrid);
    const std::vector<Path>& GetPaths() const { return m_Paths; }
    void Draw(sf::RenderWindow& window) const;

    const sf::Texture& GetMapTexture() const { return m_MapTexture; }
    const sf::Sprite& GetMapSprite() const { return m_MapSprite; }

private:
    void VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords, const MapGrid& mapGrid);
    bool DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates) const;
    bool IsValidGridCoord(const sf::Vector2i& coords, const MapGrid& mapGrid) const;
    bool IsPathTile(const sf::Vector2i& coords, const MapGrid& mapGrid) const;

    sf::Texture m_MapTexture;
    sf::Sprite m_MapSprite;
    std::vector<Path> m_Paths;
};

#endif // MAP_H