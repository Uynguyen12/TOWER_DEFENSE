#include "MapGrid.h" 
#include <stdexcept> 
#include <fstream>   
#include <sstream>   
#include <iostream>

// Constructor
MapGrid::MapGrid(int width, int height)
    : m_width(width), m_height(height)
{
    m_NodeMatrix.resize(m_height, std::vector<TileOptions::TileType>(m_width, TileOptions::TileType::Null));
}

MapGrid::MapGrid()
    : m_width(0), m_height(0) {
    std::cout << "MapGrid default constructor called." << std::endl;
}

// Initialize Function - loads texture atlas and sets up tile atlas
void MapGrid::Initialize(const std::string& textureFilePath) {
    if (!m_TileTexture.loadFromFile(textureFilePath)) {
        throw std::runtime_error("Failed to load tile texture from '" + textureFilePath + "'");
    }

    float tileSizeInTexture = 16.0f; // Original tile size in texture atlas

    // Configure each Tile type in Atlas
    // Aesthetic (row 0, col 0)
    sf::Sprite aestheticSprite;
    aestheticSprite.setTexture(m_TileTexture);
    aestheticSprite.setTextureRect(sf::IntRect(0 * tileSizeInTexture, 0 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Aesthetic] = TileOptions(TileOptions::TileType::Aesthetic);
    m_TileAtlas[TileOptions::TileType::Aesthetic].setSprite(aestheticSprite);
    std::cout << "Aesthetic tile initialized." << std::endl;

    // Spawn (row 1, col 0)
    sf::Sprite spawnSprite;
    spawnSprite.setTexture(m_TileTexture);
    spawnSprite.setTextureRect(sf::IntRect(0 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Spawn] = TileOptions(TileOptions::TileType::Spawn);
    m_TileAtlas[TileOptions::TileType::Spawn].setSprite(spawnSprite);
    std::cout << "Spawn tile initialized." << std::endl;

    // End (row 1, col 1)
    sf::Sprite endSprite;
    endSprite.setTexture(m_TileTexture);
    endSprite.setTextureRect(sf::IntRect(1 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::End] = TileOptions(TileOptions::TileType::End);
    m_TileAtlas[TileOptions::TileType::End].setSprite(endSprite);
    std::cout << "End tile initialized." << std::endl;

    // Path (row 1, col 2)
    sf::Sprite pathSprite;
    pathSprite.setTexture(m_TileTexture);
    pathSprite.setTextureRect(sf::IntRect(2 * tileSizeInTexture, 1 * tileSizeInTexture, tileSizeInTexture, tileSizeInTexture));

    m_TileAtlas[TileOptions::TileType::Path] = TileOptions(TileOptions::TileType::Path);
    m_TileAtlas[TileOptions::TileType::Path].setSprite(pathSprite);
    std::cout << "Path tile initialized." << std::endl;
}

// Draw function - for optional visualization
void MapGrid::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            TileOptions::TileType type = m_NodeMatrix[y][x];

            auto it = m_TileAtlas.find(type);
            if (it != m_TileAtlas.end()) {
                const TileOptions& tileOptions = it->second;
                sf::Sprite spriteToDraw = tileOptions.getSprite();

                // Get position using TileOptions::SIZE
                spriteToDraw.setPosition(mapToWorld(x, y));

                target.draw(spriteToDraw, states);
            }
        }
    }
}

// Main function - Load map data from file
void MapGrid::loadMapDataFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + filePath);
    }

    m_NodeMatrix.clear();
    std::string line;
    int rowCount = 0;

    std::cout << "Loading map from file: " << filePath << std::endl;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        std::vector<TileOptions::TileType> row;

        while (std::getline(ss, segment, ' ')) {
            try {
                int tileTypeInt = std::stoi(segment);
                // Convert number from level file to TileType
                TileOptions::TileType tileType;
                switch (tileTypeInt) {
                case 0: // Wall/Aesthetic
                    tileType = TileOptions::TileType::Aesthetic;
                    break;
                case 1: // Spawn
                    tileType = TileOptions::TileType::Spawn;
                    break;
                case 2: // End
                    tileType = TileOptions::TileType::End;
                    break;
                case 3: // Path
                    tileType = TileOptions::TileType::Path;
                    break;
                default:
                    tileType = TileOptions::TileType::Null;
                    break;
                }
                row.push_back(tileType);
            }
            catch (...) { // Catch all exceptions from stoi
                row.push_back(TileOptions::TileType::Null);
            }
        }

        if (!row.empty()) {
            m_NodeMatrix.push_back(row);
            rowCount++;
        }
    }
    file.close();

    m_height = rowCount;
    if (m_height > 0) {
        m_width = m_NodeMatrix[0].size();
    }
    else {
        m_width = 0;
    }

    std::cout << "Map loaded successfully: " << m_width << "x" << m_height << " tiles" << std::endl;
}