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

    // Cập nhật thông tin experience
    void UpdateExperience(int currentExp, int expToNext, int currentLevel);

    //Cập nhật thông tin coins của người chơi
    void UpdateCoins(int coins);

    // Set vị trí cho toàn bộ text group
    void SetPosition(const sf::Vector2f& position);

    // Set kích thước chữ cho tất cả
    void SetCharacterSize(unsigned int size);

    // Show/Hide experience bar
    void SetShowExperienceBar(bool show);
    void SetShowTopRightExpBar(bool show);
    void UpdateTopRightExpBar(sf::Vector2u windowSize);

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
    bool m_ShowExpBar;

    // Experience bar elements
    sf::RectangleShape m_ExpBarBackground;
    sf::RectangleShape m_ExpBarFill;
    sf::Text m_ExpText;
    sf::Text m_LevelText;

    bool m_ShowTopRightExpBar;
    sf::RectangleShape m_TopRightExpBarBackground;
    sf::RectangleShape m_TopRightExpBarFill;
    sf::Text m_TopRightExpText;
    sf::Text m_TopRightLevelText;
    sf::Text m_TopRightCoinText; 
    sf::Sprite m_CoinSprite; 
    sf::Texture m_CoinTexture;

    // Experience data
    int m_CurrentExp;
    int m_ExpToNext;
    int m_CurrentLevel;
    int m_CurrentCoins;

    enum TextIndex {
        DIFFICULTY = 0,
        GOLD = 1,
        HEALTH = 2,
        GOLD_PER_SECOND = 3,
        ENEMIES_REMAINING = 4,
        TEXT_COUNT = 5
    };
    void UpdateExpBarVisuals();
    void UpdateTopRightExpBarVisuals(sf::Vector2u windowSize);
};

#endif