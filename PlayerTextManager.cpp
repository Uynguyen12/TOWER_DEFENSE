#include "PlayerTextManager.h"
#include <sstream>
#include <iomanip>

PlayerTextManager::PlayerTextManager()
    : m_GroupPosition(0.0f, 0.0f), m_LineHeight(40.0f) {

    // Khởi tạo 5 text elements
    m_TextElements.resize(TEXT_COUNT);

    // Set labels cho từng text
    m_TextElements[DIFFICULTY].label = "Difficulty: ";
    m_TextElements[GOLD].label = "Player's Gold: ";
    m_TextElements[HEALTH].label = "Player's Health: ";
    m_TextElements[GOLD_PER_SECOND].label = "Gold Per Second: ";
    m_TextElements[ENEMIES_REMAINING].label = "Enemies Remaining: ";
}

void PlayerTextManager::Initialize(const sf::Font& font) {
    for (auto& element : m_TextElements) {
        element.text.setFont(font);
        element.text.setCharacterSize(35);
        element.text.setFillColor(element.defaultColor);
    }
}

void PlayerTextManager::UpdateContent(const std::string& difficulty, int gold, int health,
    float goldPerSecond, int enemiesRemaining) {

    // Format gold per second
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << goldPerSecond;

    // Update content
    m_TextElements[DIFFICULTY].text.setString(m_TextElements[DIFFICULTY].label + difficulty);
    m_TextElements[DIFFICULTY].defaultColor = sf::Color(255, 215, 0);
    m_TextElements[DIFFICULTY].text.setFillColor(m_TextElements[DIFFICULTY].defaultColor);  

    m_TextElements[GOLD].text.setString(m_TextElements[GOLD].label + std::to_string(gold));
    m_TextElements[GOLD].defaultColor = sf::Color(255, 250, 205);
    m_TextElements[GOLD].text.setFillColor(m_TextElements[GOLD].defaultColor);  

    m_TextElements[HEALTH].text.setString(m_TextElements[HEALTH].label + std::to_string(health));
    m_TextElements[HEALTH].defaultColor = sf::Color(102, 255, 102);
    m_TextElements[HEALTH].text.setFillColor(m_TextElements[HEALTH].defaultColor);  

    m_TextElements[GOLD_PER_SECOND].text.setString(m_TextElements[GOLD_PER_SECOND].label + ss.str());
    m_TextElements[GOLD_PER_SECOND].defaultColor = sf::Color(255, 255, 102);
    m_TextElements[GOLD_PER_SECOND].text.setFillColor(m_TextElements[GOLD_PER_SECOND].defaultColor); 

    m_TextElements[ENEMIES_REMAINING].text.setString(m_TextElements[ENEMIES_REMAINING].label + std::to_string(enemiesRemaining));
    m_TextElements[ENEMIES_REMAINING].defaultColor = sf::Color(255, 69, 0);
    m_TextElements[ENEMIES_REMAINING].text.setFillColor(m_TextElements[ENEMIES_REMAINING].defaultColor); 

    // Update positions
    for (int i = 0; i < TEXT_COUNT; ++i) {
        m_TextElements[i].text.setPosition(m_GroupPosition.x, m_GroupPosition.y + i * m_LineHeight);
    }
}

void PlayerTextManager::SetPosition(const sf::Vector2f& position) {
    m_GroupPosition = position;

    // Update positions of all text elements
    for (int i = 0; i < TEXT_COUNT; ++i) {
        m_TextElements[i].text.setPosition(m_GroupPosition.x, m_GroupPosition.y + i * m_LineHeight);
    }
}

void PlayerTextManager::SetCharacterSize(unsigned int size) {
    for (auto& element : m_TextElements) {
        element.text.setCharacterSize(size);
    }
}

void PlayerTextManager::Draw(sf::RenderWindow& window) {
    for (const auto& element : m_TextElements) {
        window.draw(element.text);
    }
}

sf::FloatRect PlayerTextManager::GetGlobalBounds() const {
    if (m_TextElements.empty()) return sf::FloatRect();

    sf::FloatRect bounds = m_TextElements[0].text.getGlobalBounds();

    for (int i = 1; i < TEXT_COUNT; ++i) {
        sf::FloatRect elementBounds = m_TextElements[i].text.getGlobalBounds();

        // Expand bounds to include this element
        float left = std::min(bounds.left, elementBounds.left);
        float top = std::min(bounds.top, elementBounds.top);
        float right = std::max(bounds.left + bounds.width, elementBounds.left + elementBounds.width);
        float bottom = std::max(bounds.top + bounds.height, elementBounds.top + elementBounds.height);

        bounds = sf::FloatRect(left, top, right - left, bottom - top);
    }

    return bounds;
}