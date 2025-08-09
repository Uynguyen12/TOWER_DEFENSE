#include "TitleScreen.h"
#include <iostream>

bool TitleScreen::Initialize(sf::RenderWindow& window) {
    if (!m_BackgroundTexture.loadFromFile("image/ttBackground.jpg")) {
        std::cerr << "Failed to load background image!" << std::endl;
        return false;
    }

    m_BackgroundSprite.setTexture(m_BackgroundTexture);

    sf::Vector2u winSize = window.getSize();
    sf::Vector2u texSize = m_BackgroundTexture.getSize();

    float scaleX = (float)winSize.x / texSize.x;
    float scaleY = (float)winSize.y / texSize.y;
    m_BackgroundSprite.setScale(scaleX, scaleY);


    if (!m_Font.loadFromFile("Fonts/MedievalSharp-Regular.ttf")) {
        std::cerr << "Failed to load font for title screen!" << std::endl;
        return false;
    }

    // Title Text
    m_TitleText.setFont(m_Font);
    m_TitleText.setString("TINY DEFENDERS");
    m_TitleText.setCharacterSize(100);
    m_TitleText.setFillColor(sf::Color::Yellow);

    sf::FloatRect titleBounds = m_TitleText.getLocalBounds();
    m_TitleText.setOrigin(titleBounds.width / 2, titleBounds.height / 2);
    m_TitleText.setPosition(window.getSize().x / 2.f, window.getSize().y / 3.f);

    // Press Key Text
    m_PressKeyText.setFont(m_Font);
    m_PressKeyText.setString("Press Any Key To Continue");
    m_PressKeyText.setCharacterSize(40);
    m_PressKeyText.setFillColor(sf::Color::White);

    sf::FloatRect pressKeyBounds = m_PressKeyText.getLocalBounds();
    m_PressKeyText.setOrigin(pressKeyBounds.width / 2, pressKeyBounds.height / 2);
    m_PressKeyText.setPosition(window.getSize().x / 2.f, window.getSize().y * 2.f / 3.f);

    return true;
}

void TitleScreen::HandleResize(sf::Vector2u newSize) {
    // Cập nhật vị trí và kích thước text
    m_TitleText.setPosition(newSize.x / 2.f, newSize.y / 3.f);
    m_PressKeyText.setPosition(newSize.x / 2.f, newSize.y * 2.f / 3.f);
    float scaleX = (float)newSize.x / m_BackgroundTexture.getSize().x;
    float scaleY = (float)newSize.y / m_BackgroundTexture.getSize().y;
    m_BackgroundSprite.setScale(scaleX, scaleY);
}

void TitleScreen::Update(float dt) {
    // Nhấp nháy alpha của dòng chữ "Press Any Key..."
    float speed = 150.0f;
    if (m_bIncreasing) {
        m_fPressKeyAlpha += speed * dt;
        if (m_fPressKeyAlpha >= 255.0f) {
            m_fPressKeyAlpha = 255.0f;
            m_bIncreasing = false;
        }
    }
    else {
        m_fPressKeyAlpha -= speed * dt;
        if (m_fPressKeyAlpha <= 50.0f) {
            m_fPressKeyAlpha = 50.0f;
            m_bIncreasing = true;
        }
    }

    sf::Color color = m_PressKeyText.getFillColor();
    color.a = static_cast<sf::Uint8>(m_fPressKeyAlpha);
    m_PressKeyText.setFillColor(color);
}

void TitleScreen::Draw(sf::RenderWindow& window) {
    window.draw(m_BackgroundSprite);
    window.draw(m_TitleText);
    window.draw(m_PressKeyText);
}

void TitleScreen::HandleInput(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed) {
        m_bShouldExit = true;
    }
}

bool TitleScreen::ShouldExit() const {
    return m_bShouldExit;
}
