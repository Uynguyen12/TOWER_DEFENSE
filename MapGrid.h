#pragma once

#include <SFML/Graphics.hpp>
#include "TileOptions.h" 
#include <vector>
#include <map>

class MapGrid : public sf::Drawable {
    sf::Texture m_TileTexture;
    std::vector<std::vector<TileOptions::TileType>> m_NodeMatrix;
    // m_fTileSize sẽ không còn là một biến độc lập của MapGrid nữa
    // int m_width, m_height; // Kích thước lưới vẫn cần thiết
    int m_width, m_height;

    std::map<TileOptions::TileType, TileOptions> m_TileAtlas;

public:
    // Constructor giờ chỉ cần kích thước lưới
    MapGrid(int width, int height);
    MapGrid();

    void Initialize(const std::string& textureFilePath);

    void loadMapDataFromFile(const std::string& filePath);

private:
    // Helper to convert grid coordinates to world coordinates
    // Sử dụng TileOptions::SIZE để xác định khoảng cách giữa các tile
    sf::Vector2f mapToWorld(int x, int y) const {
        // Lấy kích thước tile từ hằng số của TileOptions
        return sf::Vector2f(x * TileOptions::SIZE, y * TileOptions::SIZE);
    }

public:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    // Không còn getter cho TileSize nữa, vì nó là hằng số toàn cục của TileOptions
    const std::vector<std::vector<TileOptions::TileType>>& getNodeMatrix() const { return m_NodeMatrix; }
};