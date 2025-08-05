#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

class TileOptions : public sf::Drawable
{
public:
    enum TileType {
        Null = -1,
        Aesthetic, //0 - Wall/blocking tiles
        Spawn,     //1 - Enemy spawn point
        End,       //2 - End point (player base)
        Path,      //3 - Path tiles for enemies
        NumTileTypes //4
    };

    // Define SIZE as a static constant
    static const float SIZE;

    // Default constructor
    TileOptions()
        : m_tileType(TileType::Null)
    {
    }

    // Constructor with tile type
    TileOptions(TileType type);

    // Sprite management (read-only after initialization)
    void setSprite(const sf::Sprite& sprite);
    const sf::Sprite& getSprite() const { return m_sprite; }
    void setPosition(const sf::Vector2f& position) { m_sprite.setPosition(position); }

    // Tile type access (read-only)
    TileType getTileType() const { return m_tileType; }

    // Draw function
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        target.draw(m_sprite, states);
    }

private:
    // Helper function to resize sprite based on SIZE constant
    void reSize() {
        if (m_sprite.getTextureRect().width > 0 && m_sprite.getTextureRect().height > 0) {
            // Use static SIZE from class
            float scaleX = SIZE / m_sprite.getTextureRect().width;
            float scaleY = SIZE / m_sprite.getTextureRect().height;
            m_sprite.setScale(scaleX, scaleY);

            // Set origin to center for proper positioning
            m_sprite.setOrigin(
                m_sprite.getTextureRect().width / 2.0f,
                m_sprite.getTextureRect().height / 2.0f
            );
        }
    }

private:
    sf::Sprite m_sprite;
    TileType m_tileType;
};