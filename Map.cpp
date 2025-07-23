#include "Map.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

Map::Map() {
}

Map::~Map() {
}

void Map::Initialize(const std::string& textureFilePath) {
    // Load tile map texture
    if (!m_TileMapTexture.loadFromFile(textureFilePath)) {
        throw std::runtime_error("Failed to load tilemap texture from 'image/TileMap.png'");
    }

    // Initialize tile options
    m_TileOptions.clear();

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            sf::Sprite tileSprite;
            tileSprite.setTexture(m_TileMapTexture);
            tileSprite.setTextureRect(sf::IntRect(i * 16, j * 16, 16, 16));
            float scaleFactor = TileOptions::SIZE / 16.0f;
            tileSprite.setScale(sf::Vector2f(scaleFactor, scaleFactor)); 

            tileSprite.setOrigin(sf::Vector2f(16.0f / 2.0f, 16.0f / 2.0f));
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

void Map::CreateTileAtPosition(const sf::Vector2f& pos, int optionIndex) {
    float size = TileOptions::SIZE;
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }

    int x = pos.x / size;
    int y = pos.y / size;

    TileOptions::TileType eTileType = m_TileOptions[optionIndex].getTileType();
    if (eTileType == TileOptions::TileType::Null) return;

    std::vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

    if (eTileType == TileOptions::TileType::Spawn || eTileType == TileOptions::TileType::End) {
        ListOfTiles.clear(); // Clear existing spawn or end tiles (if more than 1)
    }

    sf::Sprite tile = m_TileOptions[optionIndex].getSprite();
    tile.setPosition(x * size + size, y * size + size);

    // Check if tile already exists at this position
    for (int i = 0; i < ListOfTiles.size(); i++) {
        if (ListOfTiles[i].GetPosition() == tile.getPosition()) {
            ListOfTiles[i] = ListOfTiles.back(); // Move the last tile to the current position
            ListOfTiles.pop_back(); // Remove the last tile
            break; // Tile already exists at this position, do not add a duplicate
        }
    }

    Entity& new_tiles = ListOfTiles.emplace_back(Entity::PhysicsData::Type::Static);
    new_tiles.SetSprite(tile);
    new_tiles.setRectanglePhysics(size, size);

    ConstructPath();
}

void Map::DeleteTileAtPosition(const sf::Vector2f& pos, int optionIndex) {
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }
    int x = static_cast<int>(pos.x / TileOptions::SIZE);
    int y = static_cast<int>(pos.y / TileOptions::SIZE);

    sf::Vector2f tilePosition(x * TileOptions::SIZE + TileOptions::SIZE / 2.0f, y * TileOptions::SIZE + TileOptions::SIZE / 2.0f);

    TileOptions::TileType eTileType = m_TileOptions[optionIndex].getTileType();
    if (eTileType == TileOptions::TileType::Null) return;

    std::vector<Entity>& ListOfTiles = GetListOfTiles(eTileType);

    for (int i = 0; i < ListOfTiles.size(); i++) {
        if (ListOfTiles[i].GetPosition() == tilePosition) {
            ListOfTiles[i] = ListOfTiles.back(); // Swap and pop
            ListOfTiles.pop_back();
            break; 
        }
    }

    ConstructPath();
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

        // If any of our paths are next to the end tile, they should probably go straight to end and terminate.
        // If we didn't return here, we could move around the end tile before going into it.
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
    for (const Entity& entity : m_AestheticTiles) {
        window.draw(entity);
    }
}

void Map::DrawLevelEditor(sf::RenderWindow& window, int optionIndex, const sf::Vector2f& mousePos) const {
    if (optionIndex < 0 || optionIndex >= m_TileOptions.size()) {
        return;
    }

    // Draw existing tiles
    for (const Entity& entity : m_SpawnTiles) {
        window.draw(entity);
    }

    for (const Entity& entity : m_EndTiles) {
        window.draw(entity);
    }

    for (const Entity& entity : m_PathTiles) {
        window.draw(entity);
    }

    // Draw tile preview at mouse position
    TileOptions tilePreview = m_TileOptions[optionIndex];
    tilePreview.setPosition(mousePos);
    window.draw(tilePreview);
}

void Map::PopulateFromMatrix() {
    ClearTiles(); // Xóa sạch bản đồ cũ

    // Sử dụng TileOptions::SIZE làm kích thước của mỗi ô
    const float tileSize = TileOptions::SIZE;

    // --- Chuẩn bị cho việc tìm kiếm TileOptions nhanh hơn ---
    // Nếu m_TileOptions lớn, nên có một map để truy cập nhanh
    std::map<TileOptions::TileType, const TileOptions*> tileOptionLookup;
    for (const auto& option : m_TileOptions) {
        tileOptionLookup[option.getTileType()] = &option;
    }
    // --- Hết chuẩn bị ---

    for (int y = 0; y < m_NodeMatrix.size(); ++y) {
        for (int x = 0; x <m_NodeMatrix[0].size(); ++x) {
            TileOptions::TileType type = m_NodeMatrix[y][x];

            // Sử dụng map để tìm TileOptions
            auto it = tileOptionLookup.find(type);
            const TileOptions* selectedTileOption = nullptr;
            if (it != tileOptionLookup.end()) {
                selectedTileOption = it->second;
            }

            if (selectedTileOption) {
                // Lấy sprite đã được cấu hình (với texture, textureRect, scale, origin)
                sf::Sprite tileSprite = selectedTileOption->getSprite();

                // Tính toán vị trí tâm của ô lưới
                // Giả sử TileOptions::SIZE là kích thước của ô, và origin của sprite là tâm
                float tileCenterX = x * tileSize + tileSize / 2.0f;
                float tileCenterY = y * tileSize + tileSize / 2.0f;
                tileSprite.setPosition(tileCenterX, tileCenterY);

                // Tạo Entity mới và gán sprite đã đặt vị trí
                Entity newTileEntity(Entity::PhysicsData::Type::Static);
                newTileEntity.SetSprite(tileSprite); // Entity lưu trữ sprite đã có origin và position
                // Entity cũng cần biết vị trí của nó trong grid nếu cần cho logic khác
                // newTileEntity.SetGridPosition(x, y); // Có thể thêm hàm này vào Entity

                // Thêm vào danh sách tương ứng
                switch (type) {
                case TileOptions::TileType::Aesthetic:
                    m_AestheticTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::Spawn:
                    // Cần xử lý trường hợp có nhiều spawn/end tiles nếu logic yêu cầu
                    // Ví dụ: trong CreateTileAtPosition bạn có clear() nếu thêm spawn/end
                    // Nếu ở đây bạn chỉ nạp, có lẽ nên cho phép nhiều spawn/end
                    m_SpawnTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::End:
                    m_EndTiles.push_back(newTileEntity);
                    break;
                case TileOptions::TileType::Path:
                    m_PathTiles.push_back(newTileEntity);
                    break;
                default:
                    // TileType::Null hoặc các loại chưa được định nghĩa sẽ không được thêm vào
                    break;
                }
            }
            else {
                // Cảnh báo nếu không tìm thấy TileOptions cho một loại tile trong MapGrid
                // Điều này có thể xảy ra nếu MapGrid chứa một loại tile mà Map chưa khai báo TileOptions
                if (type != TileOptions::TileType::Null) { // Bỏ qua Null type
                    std::cerr << "Warning: Map could not find TileOptions for type " << static_cast<int>(type) << " from MapGrid." << std::endl;
                }
            }
        }
    }
    std::cout << "Map populated from MapGrid." << std::endl;
}

// Clears all stored tile entities
void Map::ClearTiles() {
    m_AestheticTiles.clear();
    m_SpawnTiles.clear();
    m_EndTiles.clear();
    m_PathTiles.clear();
    m_Paths.clear();
}

// Add methods to populate tile lists based on MapGrid data
void Map::AddAestheticTile(const Entity& tile) {
    m_AestheticTiles.push_back(tile);
}
void Map::AddSpawnTile(const Entity& tile) {
    m_SpawnTiles.push_back(tile);
}
void Map::AddEndTile(const Entity& tile) {
    m_EndTiles.push_back(tile);
}
void Map::AddPathTile(const Entity& tile) {
    m_PathTiles.push_back(tile);
}

void Map::loadMapDataFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + filePath);
    }

    m_NodeMatrix.clear();
    std::string line;
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
    }
    file.close();
}