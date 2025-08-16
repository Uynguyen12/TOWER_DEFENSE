#ifndef SPEEDCONTROLPANEL_H
#define SPEEDCONTROLPANEL_H

#include <SFML/Graphics.hpp>

class SpeedControlPanel {
public:
    enum SpeedState {
        Speed_0_5x,    // 0.5x speed
        Speed_1x,      // Normal speed
        Speed_1_5x,
        Speed_2x       // 2x speed
    };

    SpeedControlPanel();
    ~SpeedControlPanel();

    bool Initialize(sf::RenderWindow& window);

    void Update(float deltaTime, const sf::Vector2f& mousePos);

    void Draw(sf::RenderWindow& window) const;

    bool HandleClick(const sf::Vector2f& mousePos);

    float GetSpeedMultiplier() const;

    SpeedState GetSpeedState() const { return m_CurrentSpeed; }

    void SetPosition(const sf::Vector2f& position);

    void SetWindowSize(const sf::Vector2u& windowSize);

private:
    void UpdateButtonAppearance();
    void UpdateHoverEffects(const sf::Vector2f& mousePos);
    std::string GetSpeedText() const;

    bool IsInSlowButton(const sf::Vector2f& mousePos) const;
    bool IsInFastButton(const sf::Vector2f& mousePos) const;

    // Panel background
    sf::RectangleShape m_PanelBackground;

    // Speed buttons
    sf::RectangleShape m_SlowButton;
    sf::RectangleShape m_SlowButtonBorder;
    sf::RectangleShape m_FastButton;
    sf::RectangleShape m_FastButtonBorder;

    // Button texts (arrows)
    sf::Text m_SlowButtonText;
    sf::Text m_FastButtonText;

    // Speed display
    sf::Text m_SpeedText;
    sf::Font m_Font;

    // Properties
    sf::Vector2f m_Position;
    sf::Vector2f m_PanelSize;
    sf::Vector2f m_ButtonSize;
    SpeedState m_CurrentSpeed;
    bool m_IsSlowButtonHovered;
    bool m_IsFastButtonHovered;

    // Animation
    float m_GlowTime;

    // Window properties
    sf::Vector2f m_windowSize;
};

#endif