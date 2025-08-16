#include "SpeedControlPanel.h"
#include <stdexcept>
#include <iostream>
#include <cmath>

SpeedControlPanel::SpeedControlPanel()
    : m_Position(50.0f, 50.0f)
    , m_PanelSize(200.0f, 60.0f)
    , m_ButtonSize(35.0f, 35.0f)
    , m_CurrentSpeed(Speed_1x)
    , m_IsSlowButtonHovered(false)
    , m_IsFastButtonHovered(false)
    , m_GlowTime(0.0f)
    , m_windowSize(0.0f, 0.0f)
{
}

SpeedControlPanel::~SpeedControlPanel() {
}

bool SpeedControlPanel::Initialize(sf::RenderWindow& window) {
    // Load font
    if (!m_Font.loadFromFile("Fonts/MedievalSharp-Regular.ttf")) {
        throw std::runtime_error("Failed to load font from 'Fonts/MedievalSharp-Regular.ttf'");
    }

    m_windowSize = sf::Vector2f(window.getSize());

    m_Position.x = m_windowSize.x / 3;
    m_Position.y = 0;

    // Setup panel background
    m_PanelBackground.setSize(m_PanelSize);
    m_PanelBackground.setPosition(m_Position);
    m_PanelBackground.setFillColor(sf::Color(60, 45, 30, 240));
    m_PanelBackground.setOutlineColor(sf::Color(180, 140, 100));
    m_PanelBackground.setOutlineThickness(2.0f);

    // Setup slow button (left)
    float buttonY = m_Position.y + (m_PanelSize.y - m_ButtonSize.y) / 2;
    float slowButtonX = m_Position.x + 15.0f;

    m_SlowButton.setSize(m_ButtonSize);
    m_SlowButton.setPosition(slowButtonX, buttonY);
    m_SlowButton.setFillColor(sf::Color(40, 35, 30, 200));

    m_SlowButtonBorder.setSize(sf::Vector2f(m_ButtonSize.x + 2.0f, m_ButtonSize.y + 2.0f));
    m_SlowButtonBorder.setPosition(sf::Vector2f(slowButtonX - 1.0f, buttonY - 1.0f));
    m_SlowButtonBorder.setFillColor(sf::Color::Transparent);
    m_SlowButtonBorder.setOutlineThickness(1.0f);

    // Setup fast button (right)
    float fastButtonX = m_Position.x + m_PanelSize.x - m_ButtonSize.x - 15.0f;

    m_FastButton.setSize(m_ButtonSize);
    m_FastButton.setPosition(fastButtonX, buttonY);
    m_FastButton.setFillColor(sf::Color(40, 35, 30, 200));

    m_FastButtonBorder.setSize(sf::Vector2f(m_ButtonSize.x + 2.0f, m_ButtonSize.y + 2.0f));
    m_FastButtonBorder.setPosition(sf::Vector2f(fastButtonX - 1.0f, buttonY - 1.0f));
    m_FastButtonBorder.setFillColor(sf::Color::Transparent);
    m_FastButtonBorder.setOutlineThickness(1.0f);

    // Setup button texts
    m_SlowButtonText.setFont(m_Font);
    m_SlowButtonText.setCharacterSize(18);
    m_SlowButtonText.setString("<<");
    m_SlowButtonText.setFillColor(sf::Color(255, 215, 0));
    m_SlowButtonText.setStyle(sf::Text::Bold);

    m_FastButtonText.setFont(m_Font);
    m_FastButtonText.setCharacterSize(18);
    m_FastButtonText.setString(">>");
    m_FastButtonText.setFillColor(sf::Color(255, 215, 0));
    m_FastButtonText.setStyle(sf::Text::Bold);

    // Setup speed display text
    m_SpeedText.setFont(m_Font);
    m_SpeedText.setCharacterSize(16);
    m_SpeedText.setFillColor(sf::Color(255, 240, 200));
    m_SpeedText.setStyle(sf::Text::Bold);

    UpdateButtonAppearance();

    return true;
}

void SpeedControlPanel::Update(float deltaTime, const sf::Vector2f& mousePos) {
    UpdateHoverEffects(mousePos);

    // Update glow animation
    m_GlowTime += deltaTime * 3.0f;
    UpdateButtonAppearance();
}

void SpeedControlPanel::UpdateHoverEffects(const sf::Vector2f& mousePos) {
    bool wasSlowHovered = m_IsSlowButtonHovered;
    bool wasFastHovered = m_IsFastButtonHovered;

    m_IsSlowButtonHovered = IsInSlowButton(mousePos);
    m_IsFastButtonHovered = IsInFastButton(mousePos);

    // Update slow button hover effect
    if (m_IsSlowButtonHovered != wasSlowHovered) {
        if (m_IsSlowButtonHovered) {
            m_SlowButton.setFillColor(sf::Color(60, 50, 40, 240));
        }
        else {
            m_SlowButton.setFillColor(sf::Color(40, 35, 30, 200));
        }
    }

    // Update fast button hover effect
    if (m_IsFastButtonHovered != wasFastHovered) {
        if (m_IsFastButtonHovered) {
            m_FastButton.setFillColor(sf::Color(60, 50, 40, 240));
        }
        else {
            m_FastButton.setFillColor(sf::Color(40, 35, 30, 200));
        }
    }
}

bool SpeedControlPanel::IsInSlowButton(const sf::Vector2f& mousePos) const {
    return m_SlowButton.getGlobalBounds().contains(mousePos);
}

bool SpeedControlPanel::IsInFastButton(const sf::Vector2f& mousePos) const {
    return m_FastButton.getGlobalBounds().contains(mousePos);
}

bool SpeedControlPanel::HandleClick(const sf::Vector2f& mousePos) {
    if (IsInSlowButton(mousePos)) {
        // Decrease speed
        switch (m_CurrentSpeed) {
        case Speed_2x:
            m_CurrentSpeed = Speed_1_5x;
            break;
        case Speed_1_5x:
            m_CurrentSpeed = Speed_1x;
            break;
        case Speed_1x:
            m_CurrentSpeed = Speed_0_5x;
            break;
        case Speed_0_5x:
            // Already at minimum, do nothing
            break;
        }

        UpdateButtonAppearance();
        std::cout << "Speed changed to: " << GetSpeedText() << std::endl;
        return true;
    }
    else if (IsInFastButton(mousePos)) {
        // Increase speed
        switch (m_CurrentSpeed) {
        case Speed_0_5x:
            m_CurrentSpeed = Speed_1x;
            break;
        case Speed_1x:
            m_CurrentSpeed = Speed_1_5x;
            break;
        case Speed_1_5x:
            m_CurrentSpeed = Speed_2x;
            break;
        case Speed_2x:
            // Already at maximum, do nothing
            break;
        }

        UpdateButtonAppearance();
        std::cout << "Speed changed to: " << GetSpeedText() << std::endl;
        return true;
    }
    return false;
}

float SpeedControlPanel::GetSpeedMultiplier() const {
    switch (m_CurrentSpeed) {
    case Speed_0_5x:
        return 0.5f;
    case Speed_1x:
        return 1.0f;
    case Speed_1_5x:
        return 1.5f;
    case Speed_2x:
        return 2.0f;
    default:
        return 1.0f;
    }
}

std::string SpeedControlPanel::GetSpeedText() const {
    switch (m_CurrentSpeed) {
    case Speed_0_5x:
        return "0.5x";
    case Speed_1x:
        return "1.0x";
    case Speed_1_5x:
        return "1.5x";
    case Speed_2x:
        return "2.0x";
    default:
        return "1.0x";
    }
}

void SpeedControlPanel::UpdateButtonAppearance() {
    // Update speed display text
    m_SpeedText.setString(GetSpeedText());
    m_SpeedText.setCharacterSize(18);
    m_SpeedText.setFillColor(sf::Color(255, 220, 150));
    m_SpeedText.setStyle(sf::Text::Bold);

    // Center speed text in panel
    sf::FloatRect textBounds = m_SpeedText.getLocalBounds();
    m_SpeedText.setPosition(
        m_Position.x + (m_PanelSize.x - textBounds.width) / 2,
        m_Position.y + (m_PanelSize.y - textBounds.height) / 2 - 2.0f
    );

    // Calculate glow intensity for animation
    float glowIntensity = (std::sin(m_GlowTime) + 1.0f) / 2.0f; // 0.0 to 1.0

    // Update button colors and borders based on current speed
    if (m_CurrentSpeed == Speed_0_5x) {
        // Slow button is active - make it glow blue
        sf::Uint8 blueIntensity = static_cast<sf::Uint8>(150 + glowIntensity * 105); // 150-255
        m_SlowButtonText.setFillColor(sf::Color(100, 150, blueIntensity));
        m_SlowButtonBorder.setOutlineColor(sf::Color(80, 120, blueIntensity));

        // Fast button is normal
        m_FastButtonText.setFillColor(sf::Color(255, 215, 0));
        m_FastButtonBorder.setOutlineColor(sf::Color(100, 85, 70));
    }
    else if (m_CurrentSpeed == Speed_1_5x || m_CurrentSpeed == Speed_2x) {
        // Fast button is active - make it glow red
        sf::Uint8 redIntensity = static_cast<sf::Uint8>(150 + glowIntensity * 105); // 150-255
        m_FastButtonText.setFillColor(sf::Color(redIntensity, 100, 100));
        m_FastButtonBorder.setOutlineColor(sf::Color(redIntensity, 80, 80));

        // Slow button is normal
        m_SlowButtonText.setFillColor(sf::Color(255, 215, 0));
        m_SlowButtonBorder.setOutlineColor(sf::Color(100, 85, 70));
    }
    else {
        // Normal speed - both buttons are normal (no glow)
        m_SlowButtonText.setFillColor(sf::Color(255, 215, 0));
        m_SlowButtonBorder.setOutlineColor(sf::Color(100, 85, 70));

        m_FastButtonText.setFillColor(sf::Color(255, 215, 0));
        m_FastButtonBorder.setOutlineColor(sf::Color(100, 85, 70));
    }

    // Center button texts
    sf::FloatRect slowTextBounds = m_SlowButtonText.getLocalBounds();
    m_SlowButtonText.setPosition(
        m_SlowButton.getPosition().x + (m_ButtonSize.x - slowTextBounds.width) / 2,
        m_SlowButton.getPosition().y + (m_ButtonSize.y - slowTextBounds.height) / 2 - 2.0f
    );

    sf::FloatRect fastTextBounds = m_FastButtonText.getLocalBounds();
    m_FastButtonText.setPosition(
        m_FastButton.getPosition().x + (m_ButtonSize.x - fastTextBounds.width) / 2,
        m_FastButton.getPosition().y + (m_ButtonSize.y - fastTextBounds.height) / 2 - 2.0f
    );
}

void SpeedControlPanel::SetPosition(const sf::Vector2f& position) {
    sf::Vector2f offset = position - m_Position;
    m_Position = position;

    // Update all elements positions
    m_PanelBackground.setPosition(m_Position);

    m_SlowButton.move(offset);
    m_SlowButtonBorder.move(offset);
    m_FastButton.move(offset);
    m_FastButtonBorder.move(offset);

    UpdateButtonAppearance();
}

void SpeedControlPanel::SetWindowSize(const sf::Vector2u& windowSize) {
    m_windowSize = static_cast<sf::Vector2f>(windowSize);
}

void SpeedControlPanel::Draw(sf::RenderWindow& window) const {
    // Draw panel
    window.draw(m_PanelBackground);

    // Draw buttons
    window.draw(m_SlowButton);
    window.draw(m_SlowButtonBorder);
    window.draw(m_FastButton);
    window.draw(m_FastButtonBorder);

    // Draw texts
    window.draw(m_SlowButtonText);
    window.draw(m_FastButtonText);
    window.draw(m_SpeedText);
}