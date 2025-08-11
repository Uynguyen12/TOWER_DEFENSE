#include "UIManager.h"
#include <algorithm>
#include <iostream>

UIManager::UIManager()
    : m_iMaxHealth(100)
    , m_iCurrentHealth(100)
    , m_HealthBarPosition(50.0f, 50.0f)
    , m_HealthBarSize(200.0f, 20.0f)
    , m_HealthBarBackgroundColor(sf::Color(50, 50, 50, 200))
    , m_HealthBarForegroundColor(sf::Color::Green)
    , m_HealthBarBorderColor(sf::Color::White)
    , m_fAnimationSpeed(2.0f)
    , m_fCurrentHealthPercent(1.0f)
    , m_fTargetHealthPercent(1.0f)
{
}
UIManager::~UIManager() {
}

bool UIManager::Initialize() {
    // Initialize health bar components
    m_HealthBarBackground.setSize(m_HealthBarSize);
    m_HealthBarBackground.setPosition(m_HealthBarPosition);
    m_HealthBarBackground.setFillColor(m_HealthBarBackgroundColor);

    m_HealthBarForeground.setSize(m_HealthBarSize);
    m_HealthBarForeground.setPosition(m_HealthBarPosition);
    m_HealthBarForeground.setFillColor(m_HealthBarForegroundColor);

    // Border setup
    m_HealthBarBorder.setSize(sf::Vector2f(m_HealthBarSize.x + 4.0f, m_HealthBarSize.y + 4.0f));
    m_HealthBarBorder.setPosition(sf::Vector2f(m_HealthBarPosition.x - 2.0f, m_HealthBarPosition.y - 2.0f));
    m_HealthBarBorder.setFillColor(sf::Color::Transparent);
    m_HealthBarBorder.setOutlineColor(m_HealthBarBorderColor);
    m_HealthBarBorder.setOutlineThickness(2.0f);

    if (!m_Font.loadFromFile("Fonts/MedievalSharp-Regular.ttf")) {
        throw std::runtime_error("Failed to load font from 'Fonts/MedievalSharp-Regular.ttf'");
    }
    m_warningText.setFont(m_Font);
    m_warningText.setCharacterSize(40);
    m_warningText.setFillColor(sf::Color(255, 0, 0));
    m_warningText.setStyle(sf::Text::Bold);
    m_warningText.setPosition(500, 1000);

    return true;
}

void UIManager::SetWarningMessage(const std::string& message, const sf::Vector2u& windowSize) {
    m_warningMessage = message;
    m_warningText.setString(message);
    m_warningTimer = m_warningDuration;

    sf::FloatRect bounds = m_warningText.getLocalBounds();
    float textWidth = bounds.width;
    float textHeight = bounds.height;
    float offsetX = bounds.left;
    float offsetY = bounds.top;

    m_warningText.setPosition(
        (windowSize.x - textWidth) / 2.f - offsetX,
        windowSize.y - textHeight - 2.f - offsetY - 100
    );
}

void UIManager::Update(float deltaTime) {
    AnimateHealthBar(deltaTime);

    if (m_warningTimer > 0.f) {
        m_warningTimer -= deltaTime;
        if (m_warningTimer <= 0.f) {
            m_warningMessage.clear();
            m_warningText.setString("");
        }
    }
}

void UIManager::Draw(sf::RenderWindow& window) const {
    // Draw health bar components
    window.draw(m_HealthBarBorder);
    window.draw(m_HealthBarBackground);
    window.draw(m_HealthBarForeground);

    if (!m_warningMessage.empty()) {
        window.draw(m_warningText);
    }
}

void UIManager::SetMaxHealth(int maxHealth) {
    m_iMaxHealth = maxHealth;
    UpdateHealthBarVisual();
}

void UIManager::SetCurrentHealth(int currentHealth) {
    m_iCurrentHealth = std::max(0, currentHealth);
    m_fTargetHealthPercent = static_cast<float>(m_iCurrentHealth) / static_cast<float>(m_iMaxHealth);

    // Update health bar color based on health percentage
    if (m_fTargetHealthPercent > 0.6f) {
        m_HealthBarForegroundColor = sf::Color::Green;
    }
    else if (m_fTargetHealthPercent > 0.3f) {
        m_HealthBarForegroundColor = sf::Color::Yellow;
    }
    else {
        m_HealthBarForegroundColor = sf::Color::Red;
    }
}

void UIManager::UpdateHealthBar(int currentHealth, int maxHealth) {
    SetMaxHealth(maxHealth);
    SetCurrentHealth(currentHealth);
}

void UIManager::SetHealthBarPosition(const sf::Vector2f& position) {
    m_HealthBarPosition = position;

    m_HealthBarBackground.setPosition(m_HealthBarPosition);
    m_HealthBarForeground.setPosition(m_HealthBarPosition);
    m_HealthBarBorder.setPosition(sf::Vector2f(m_HealthBarPosition.x - 2.0f, m_HealthBarPosition.y - 2.0f));
}

void UIManager::SetHealthBarSize(const sf::Vector2f& size) {
    m_HealthBarSize = size;

    m_HealthBarBackground.setSize(m_HealthBarSize);
    m_HealthBarBorder.setSize(sf::Vector2f(m_HealthBarSize.x + 4.0f, m_HealthBarSize.y + 4.0f));

    UpdateHealthBarVisual();
}

sf::FloatRect UIManager::GetHealthBarBounds() const {
    return m_HealthBarBorder.getGlobalBounds();
}

void UIManager::UpdateHealthBarVisual() {
    // Update foreground size based on current health percentage
    sf::Vector2f foregroundSize(m_HealthBarSize.x * m_fCurrentHealthPercent, m_HealthBarSize.y);
    m_HealthBarForeground.setSize(foregroundSize);
    m_HealthBarForeground.setFillColor(m_HealthBarForegroundColor);
}

void UIManager::AnimateHealthBar(float deltaTime) {
    // Smooth animation towards target health
    if (std::abs(m_fCurrentHealthPercent - m_fTargetHealthPercent) > 0.01f) {
        float direction = (m_fTargetHealthPercent > m_fCurrentHealthPercent) ? 1.0f : -1.0f;
        m_fCurrentHealthPercent += direction * m_fAnimationSpeed * deltaTime;

        // Clamp to target
        if (direction > 0) {
            m_fCurrentHealthPercent = std::min(m_fCurrentHealthPercent, m_fTargetHealthPercent);
        }
        else {
            m_fCurrentHealthPercent = std::max(m_fCurrentHealthPercent, m_fTargetHealthPercent);
        }

        UpdateHealthBarVisual();
    }
}