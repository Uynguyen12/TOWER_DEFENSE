#include "MapGrid.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

MapGrid::MapGrid(int width, int height)
    : m_width(width), m_height(height) {
    m_NodeMatrix.resize(m_height, std::vector<TileType>(m_width, TileType::Null));
}

MapGrid::MapGrid()
    : m_width(0), m_height(0) {
    std::cout << "MapGrid default constructor called." << std::endl;
}

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
        std::vector<TileType> row;

        while (std::getline(ss, segment, ' ')) {
            try {
                int tileTypeInt = std::stoi(segment);
                TileType tileType;
                switch (tileTypeInt) {
                case 0: tileType = TileType::Aesthetic; break;
                case 1: tileType = TileType::Spawn; break;
                case 2: tileType = TileType::End; break;
                case 3: tileType = TileType::Path; break;
                default: tileType = TileType::Null; break;
                }
                row.push_back(tileType);
            }
            catch (...) {
                row.push_back(TileType::Null);
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