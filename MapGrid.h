#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class MapGrid {
public:
    enum TileType {
        Null = -1,
        Aesthetic, // 0 - Wall/blocking tiles
        Spawn,     // 1 - Enemy spawn point
        End,       // 2 - End point (player base)
        Path,      // 3 - Path tiles for enemies
		Etc,       // 4 - Other tiles (e.g., decorative)
        NumTileTypes // 5
    };

    MapGrid(int width, int height);
    MapGrid();

    void loadMapDataFromFile(const std::string& filePath);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const std::vector<std::vector<TileType>>& getNodeMatrix() const { return m_NodeMatrix; }

private:
    std::vector<std::vector<TileType>> m_NodeMatrix;
    int m_width, m_height;
};