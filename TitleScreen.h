#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class TitleScreen {
public:
    bool Initialize(sf::RenderWindow& window);
    void Update(float dt);
    void Draw(sf::RenderWindow& window);
    void HandleInput(sf::Event& event);
    bool ShouldExit() const;
    void HandleResize(sf::Vector2u newSize);

private:
    sf::Font m_Font;
    sf::Text m_TitleText;
    sf::Text m_PressKeyText;
    sf::Texture m_BackgroundTexture;
    sf::Sprite m_BackgroundSprite;

    bool m_bShouldExit = false;
    float m_fPressKeyAlpha = 255.0f;
    bool m_bIncreasing = false;

    // Optional: bạn có thể thêm hiệu ứng sao (nếu dùng)
    struct Star {
        sf::Vector2f position;
        float alpha = 255.0f;
        float size = 1.0f;
        float twinkleSpeed = 1.0f;
        bool increasing = true;
    };
    std::vector<Star> m_Stars;
};
