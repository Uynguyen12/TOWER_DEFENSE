#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <SFML/Graphics.hpp>
#include <string>

class UIManager {
public:
    UIManager();
    ~UIManager();

    // Initialize UI elements
    bool Initialize();

    // Update UI elements
    void Update(float deltaTime);

    // Draw UI elements
    void Draw(sf::RenderWindow& window) const;

    // Health bar functions
    void SetMaxHealth(int maxHealth);
    void SetCurrentHealth(int currentHealth);
    void UpdateHealthBar(int currentHealth, int maxHealth);

    // Position and size settings
    void SetHealthBarPosition(const sf::Vector2f& position);
    void SetHealthBarSize(const sf::Vector2f& size);

    // Get health bar bounds for positioning
    sf::FloatRect GetHealthBarBounds() const;

    //Message
    void SetWarningMessage(const std::string& messag, const sf::Vector2u& windowSize);

private:
    // Health bar components
    sf::RectangleShape m_HealthBarBackground;
    sf::RectangleShape m_HealthBarForeground;
    sf::RectangleShape m_HealthBarBorder;

    // Health bar properties
    int m_iMaxHealth;
    int m_iCurrentHealth;
    sf::Vector2f m_HealthBarPosition;
    sf::Vector2f m_HealthBarSize;
    sf::Vector2u windowSize;

    // Colors
    sf::Color m_HealthBarBackgroundColor;
    sf::Color m_HealthBarForegroundColor;
    sf::Color m_HealthBarBorderColor;

    // Animation properties
    float m_fAnimationSpeed;
    float m_fCurrentHealthPercent;
    float m_fTargetHealthPercent;

    //Message
    std::string m_warningMessage;
    sf::FloatRect bounds;
    sf::Text m_warningText;
    sf::Font m_Font;
    float m_warningTimer = 0.f;
    float m_warningDuration = 2.0f;

    // Helper functions
    void UpdateHealthBarVisual();
    void AnimateHealthBar(float deltaTime);

    
};

#endif // UIMANAGER_H