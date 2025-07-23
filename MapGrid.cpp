#include "MapGrid.h" 
#include <stdexcept> 
#include <fstream>   
#include <sstream>   
#include <iostream>

// --- Constructor ---
// Chỉ cần kích thước lưới. Tile size sẽ được lấy trực tiếp từ TileOptions::SIZE.
MapGrid::MapGrid(int width, int height)
    : m_width(width), m_height(height)
{
    m_NodeMatrix.resize(m_height, std::vector<TileOptions::TileType>(m_width, TileOptions::TileType::Null));
}

MapGrid::MapGrid()
    : m_width(0), m_height(0) {
    std::cout << "MapGrid default constructor called." << std::endl; // Để debug
}

// --- Initialize Function ---
// Hàm này tải texture atlas và điền vào m_TileAtlas.
// Nó không cần tham số tileSize vì đã có TileOptions::SIZE.
void MapGrid::Initialize(const std::string& textureFilePath) {
    if (!m_TileTexture.loadFromFile(textureFilePath)) {
        throw std::runtime_error("Failed to load tile texture from '" + textureFilePath + "'");
    }

    float tileSizeInTexture = 16.0f; // Kích thước tile gốc trong texture atlas

    // --- Cấu hình từng loại Tile trong Atlas ---
    // Aesthetic (row 0, col 0)
    sf::Sprite aestheticSprite;
    aestheticSprite.setTexture(m_TileTexture);
    aestheticSprite.setTextureRect(sf::IntRect(0 * tileSizeInTexture, 0 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Aesthetic] = TileOptions(TileOptions::TileType::Aesthetic);
    m_TileAtlas[TileOptions::TileType::Aesthetic].setSprite(aestheticSprite); // setSprite sẽ gọi reSize() dùng TileOptions::SIZE
	std::cout << "Aesthetic tile initialized." << std::endl; // Debug log

    // Spawn (row 1, col 0)
    sf::Sprite spawnSprite;
    spawnSprite.setTexture(m_TileTexture);
    spawnSprite.setTextureRect(sf::IntRect(0 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Spawn] = TileOptions(TileOptions::TileType::Spawn);
    m_TileAtlas[TileOptions::TileType::Spawn].setSprite(spawnSprite);
	std::cout << "Spawn tile initialized." << std::endl; // Debug log

    // End (row 1, col 1)
    sf::Sprite endSprite;
    endSprite.setTexture(m_TileTexture);
    endSprite.setTextureRect(sf::IntRect(1 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::End] = TileOptions(TileOptions::TileType::End);
    m_TileAtlas[TileOptions::TileType::End].setSprite(endSprite);
	std::cout << "End tile initialized." << std::endl; // Debug log

    // Path (row 1, col 2)
    sf::Sprite pathSprite;
    pathSprite.setTexture(m_TileTexture);
    pathSprite.setTextureRect(sf::IntRect(2 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Path] = TileOptions(TileOptions::TileType::Path);
    m_TileAtlas[TileOptions::TileType::Path].setSprite(pathSprite);
	std::cout << "Path tile initialized." << std::endl; // Debug log

    // ... (tiếp tục với các loại tile khác nếu có)
}

// --- Implement draw function ---
void MapGrid::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            TileOptions::TileType type = m_NodeMatrix[y][x];

            auto it = m_TileAtlas.find(type);
            if (it != m_TileAtlas.end()) {
                const TileOptions& tileOptions = it->second;
                sf::Sprite spriteToDraw = tileOptions.getSprite();

                // Lấy vị trí bằng cách sử dụng TileOptions::SIZE
                spriteToDraw.setPosition(mapToWorld(x, y));

                target.draw(spriteToDraw, states);
            }
        }
    }
}


void MapGrid::loadMapDataFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + filePath);
    }

    m_NodeMatrix.clear();
    std::string line;
    int rowCount = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        std::vector<TileOptions::TileType> row;

        while (std::getline(ss, segment, ' ')) {
            try {
                int tileTypeInt = std::stoi(segment);
                if (tileTypeInt >= static_cast<int>(TileOptions::TileType::Null) && tileTypeInt < static_cast<int>(TileOptions::TileType::NumTileTypes)) {
                    row.push_back(static_cast<TileOptions::TileType>(tileTypeInt));
                }
                else {
                    row.push_back(TileOptions::TileType::Null);
                }
            }
            catch (...) { // Catch all exceptions from stoi for simplicity here
                row.push_back(TileOptions::TileType::Null);
            }
        }
        m_NodeMatrix.push_back(row);
        rowCount++;
    }
    file.close();

    m_height = rowCount;
    if (m_height > 0) {
        m_width = m_NodeMatrix[0].size();
    }
    else {
        m_width = 0;
    }
}