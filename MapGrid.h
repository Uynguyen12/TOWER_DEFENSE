#pragma once

#include <SFML/Graphics.hpp>
#include "TileOptions.h" 
#include <vector>
#include <map>

class MapGrid : public sf::Drawable {
    sf::Texture m_TileTexture;
    std::vector<std::vector<TileOptions::TileType>> m_NodeMatrix;
    int m_width, m_height;

    std::map<TileOptions::TileType, TileOptions> m_TileAtlas;

public:
    // Constructor only needs grid size
    MapGrid(int width, int height);
    MapGrid();

    // Initialize texture atlas
    void Initialize(const std::string& textureFilePath);

    // Load map data from file (main function for this simplified version)
    void loadMapDataFromFile(const std::string& filePath);

private:
    // Helper to convert grid coordinates to world coordinates
    sf::Vector2f mapToWorld(int x, int y) const {
        return sf::Vector2f(x * TileOptions::SIZE, y * TileOptions::SIZE);
    }

public:
    // Draw function for visualization (optional)
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    // Getters (read-only access)
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const std::vector<std::vector<TileOptions::TileType>>& getNodeMatrix() const { return m_NodeMatrix; }
};