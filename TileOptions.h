#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <map> // Đảm bảo include map nếu MapGrid sử dụng nó

class TileOptions : public sf::Drawable
{
public:
	enum TileType {
		Null = -1,
		Aesthetic, //0
		Spawn, //1
		End, //2
		Path, //3
		NumTileTypes //4
	};

	// Định nghĩa SIZE là một hằng số static
	// Nó thuộc về lớp TileOptions chứ không phải một instance cụ thể
	static const float SIZE;

	// Constructor bây giờ không cần nhận displaySize nữa, vì nó sẽ lấy từ SIZE static
	TileOptions()
		: m_tileType(TileType::Null) // Khởi tạo với loại Null
		// Sprite sẽ không có texture hoặc rect ban đầu, cần được set sau
	{
	}
	TileOptions(TileType type);

	void setSprite(const sf::Sprite& sprite); // Hàm này sẽ gọi reSize()
	const sf::Sprite& getSprite() const { return m_sprite; }
	void setPosition(const sf::Vector2f& position) { m_sprite.setPosition(position); }
	TileType getTileType() const { return m_tileType; }

	// Helper function đã có, giờ sẽ sử dụng SIZE từ lớp
	void reSize() {
		if (m_sprite.getTextureRect().width > 0 && m_sprite.getTextureRect().height > 0) {
			// Sử dụng SIZE static của lớp
			m_sprite.setScale(SIZE / m_sprite.getTextureRect().width, SIZE / m_sprite.getTextureRect().height);
		}
		else {
			// Xử lý lỗi hoặc đặt scale mặc định nếu cần
		}
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(m_sprite, states);
	}
private:
	sf::Sprite m_sprite;
	TileType m_tileType;
	// Không còn cần m_size nữa, vì ta dùng SIZE static
};