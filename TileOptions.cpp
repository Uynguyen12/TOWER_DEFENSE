#include "TileOptions.h"

// Initialize static member constant outside the class
const float TileOptions::SIZE = 64.0f;

// Constructor: only receives tile type
TileOptions::TileOptions(TileType type)
    : m_tileType(type)
{
    // Sprite will be created and assigned by MapGrid::Initialize or Map::Initialize
}

// setSprite function
void TileOptions::setSprite(const sf::Sprite& sprite) {
    m_sprite = sprite;
    reSize(); // Call reSize after assigning sprite, it will use TileOptions::SIZE
}