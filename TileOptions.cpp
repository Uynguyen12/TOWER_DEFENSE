#include "TileOptions.h" // Đảm bảo include file header tương ứng

// Khởi tạo hằng số static member bên ngoài lớp
const float TileOptions::SIZE = 80.0f;


// Constructor: chỉ nhận loại tile
TileOptions::TileOptions(TileType type)
    : m_tileType(type)
{
    // Sprite sẽ được tạo và gán bởi MapGrid::Initialize
}

// Hàm setSprite
void TileOptions::setSprite(const sf::Sprite& sprite) {
    m_sprite = sprite;
    reSize(); // Gọi reSize sau khi gán sprite, nó sẽ sử dụng TileOptions::SIZE
}
