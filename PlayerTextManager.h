#pragma once
#ifndef PLAYERTEXTMANAGER_H
#define PLAYERTEXTMANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class PlayerTextManager {
public:
    PlayerTextManager();
    ~PlayerTextManager() = default;

    // Khởi tạo với font
    void Initialize(const sf::Font& font);

    // Cập nhật nội dung và vị trí
    void UpdateContent(const std::string& difficulty, int gold, int health,
        float goldPerSecond, int enemiesRemaining);

    // Set vị trí cho toàn bộ text group
    void SetPosition(const sf::Vector2f& position);

    // Set kích thước chữ cho tất cả
    void SetCharacterSize(unsigned int size);

    // Vẽ tất cả text
    void Draw(sf::RenderWindow& window);

    // Tính toán kích thước bounding box của toàn bộ text group
    sf::FloatRect GetGlobalBounds() const;

private:
    struct ColoredText {
        sf::Text text;
        sf::Color defaultColor;
        std::string label;
    };

    std::vector<ColoredText> m_TextElements;
    sf::Vector2f m_GroupPosition;
    float m_LineHeight;

    enum TextIndex {
        DIFFICULTY = 0,
        GOLD = 1,
        HEALTH = 2,
        GOLD_PER_SECOND = 3,
        ENEMIES_REMAINING = 4,
        TEXT_COUNT = 5
    };
};

#endif