#include "PlayerTextManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

PlayerTextManager::PlayerTextManager()
    : m_GroupPosition(0.0f, 0.0f),
    m_LineHeight(40.0f),
    m_ShowExpBar(false),
    m_ShowTopRightExpBar(false),
    m_CurrentExp(0),
    m_ExpToNext(100),
    m_CurrentLevel(1) {

    // Khởi tạo 5 text elements
    m_TextElements.resize(TEXT_COUNT);

    // Set labels cho từng text
    m_TextElements[DIFFICULTY].label = "Difficulty: ";
    m_TextElements[GOLD].label = "Player's Gold: ";
    m_TextElements[HEALTH].label = "Player's Health: ";
    m_TextElements[GOLD_PER_SECOND].label = "Gold Per Second: ";
    m_TextElements[ENEMIES_REMAINING].label = "Enemies Remaining: ";

    // Initialize experience bar background
    m_ExpBarBackground.setSize(sf::Vector2f(300.0f, 30.0f));
    m_ExpBarBackground.setFillColor(sf::Color(50, 50, 50, 200));
    m_ExpBarBackground.setOutlineThickness(2.0f);
    m_ExpBarBackground.setOutlineColor(sf::Color(150, 150, 150));

    // Initialize experience bar fill
    m_ExpBarFill.setSize(sf::Vector2f(0.0f, 26.0f));
    m_ExpBarFill.setFillColor(sf::Color(255, 215, 0)); // Blue color for experience

    m_TopRightExpBarBackground.setSize(sf::Vector2f(250.0f, 25.0f));
    m_TopRightExpBarBackground.setFillColor(sf::Color(40, 40, 40, 220));
    m_TopRightExpBarBackground.setOutlineThickness(2.0f);
    m_TopRightExpBarBackground.setOutlineColor(sf::Color(100, 100, 100));

    m_TopRightExpBarFill.setSize(sf::Vector2f(0.0f, 21.0f));
    m_TopRightExpBarFill.setFillColor(sf::Color(255, 215, 0));

    if (!m_CoinTexture.loadFromFile("image/menu/coin.png")) {
        // If coin texture doesn't exist, create a simple circle
        std::cout << "Warning: Could not load coin.png, using default circle" << std::endl;
    }
    else {
        m_CoinSprite.setTexture(m_CoinTexture);
        m_CoinSprite.setScale(0.15f, 0.15f); 
    }

}

void PlayerTextManager::Initialize(const sf::Font& font) {
    for (auto& element : m_TextElements) {
        element.text.setFont(font);
        element.text.setCharacterSize(35);
        element.text.setFillColor(element.defaultColor);
    }

    // Initialize experience text
    m_ExpText.setFont(font);
    m_ExpText.setCharacterSize(20);
    m_ExpText.setFillColor(sf::Color::White);
    m_ExpText.setStyle(sf::Text::Bold);

    // Initialize level text
    m_LevelText.setFont(font);
    m_LevelText.setCharacterSize(35);
    m_LevelText.setFillColor(sf::Color::White); 
    m_LevelText.setStyle(sf::Text::Bold);

    m_TopRightExpText.setFont(font);
    m_TopRightExpText.setCharacterSize(20);
    m_TopRightExpText.setFillColor(sf::Color::White);
    m_TopRightCoinText.setStyle(sf::Text::Bold);

    m_TopRightLevelText.setFont(font);
    m_TopRightLevelText.setCharacterSize(28);
    m_TopRightLevelText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    m_TopRightLevelText.setStyle(sf::Text::Bold);

    m_TopRightCoinText.setFont(font);
    m_TopRightCoinText.setCharacterSize(30);
    m_TopRightCoinText.setFillColor(sf::Color(255, 215, 0)); // Gold color
    m_TopRightCoinText.setStyle(sf::Text::Bold);

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

    if (m_ShowExpBar) {
        UpdateExpBarVisuals();
    }
}

void PlayerTextManager::UpdateExperience(int currentExp, int expToNext, int currentLevel) {
    m_CurrentExp = currentExp;
    m_ExpToNext = expToNext;
    m_CurrentLevel = currentLevel;

    if (m_ShowExpBar) {
        UpdateExpBarVisuals();
    }
}

void PlayerTextManager::UpdateCoins(int coins) {
    m_CurrentCoins = coins;
    m_TopRightCoinText.setString(std::to_string(coins));
}

void PlayerTextManager::SetPosition(const sf::Vector2f& position) {
    m_GroupPosition = position;

    // Update positions of all text elements
    for (int i = 0; i < TEXT_COUNT; ++i) {
        m_TextElements[i].text.setPosition(m_GroupPosition.x, m_GroupPosition.y + i * m_LineHeight);
    }

    if (m_ShowExpBar) {
        UpdateExpBarVisuals();
    }
}

void PlayerTextManager::SetCharacterSize(unsigned int size) {
    for (auto& element : m_TextElements) {
        element.text.setCharacterSize(size);
    }
}

void PlayerTextManager::SetShowExperienceBar(bool show) {
    m_ShowExpBar = show;
    if (show) {
        UpdateExpBarVisuals();
    }
}

void PlayerTextManager::SetShowTopRightExpBar(bool show) {
    m_ShowTopRightExpBar = show;
}

void PlayerTextManager::UpdateTopRightExpBar(sf::Vector2u windowSize) {
    if (!m_ShowTopRightExpBar) return;

    UpdateTopRightExpBarVisuals(windowSize);
}

// Thêm vào UpdateTopRightExpBarVisuals() method

void PlayerTextManager::UpdateTopRightExpBarVisuals(sf::Vector2u windowSize) {
    float margin = 10.0f;
    float spacing = 5.0f;
    float levelToExpSpacing = 8.0f;
    float rightX = windowSize.x - margin;

    // === Tính toán vị trí Y cho exp bar và coin (thẳng hàng) ===
    float expBarY = margin + 30.0f + levelToExpSpacing;

    // === 1. Experience Bar dimensions ===
    float expBarWidth = m_TopRightExpBarBackground.getSize().x;
    float expBarHeight = m_TopRightExpBarBackground.getSize().y;

    // === 2. Coin Text ===
    m_TopRightCoinText.setString(std::to_string(m_CurrentCoins));
    sf::FloatRect coinTextBounds = m_TopRightCoinText.getLocalBounds();

    // === 3. Coin Sprite dimensions ===
    float coinSpriteWidth = 0.0f;
    float coinSpriteHeight = 0.0f;
    if (m_CoinTexture.getSize().x > 0) {
        sf::FloatRect spriteLocalBounds = m_CoinSprite.getLocalBounds();
        coinSpriteWidth = spriteLocalBounds.width * m_CoinSprite.getScale().x;
        coinSpriteHeight = spriteLocalBounds.height * m_CoinSprite.getScale().y;
    }

    // === Đặt vị trí từ phải sang trái ===

    // 1. Coin text (góc phải nhất, căn giữa với exp bar)
    float coinTextX = rightX - coinTextBounds.width;
    float coinTextY = expBarY + (expBarHeight - coinTextBounds.height) / 2.0f - 3.0f;
    m_TopRightCoinText.setPosition(coinTextX, coinTextY);

    // 2. Coin sprite (nằm giữa coin text và exp bar)
    float coinSpriteX = coinTextX - coinSpriteWidth - spacing;
    if (m_CoinTexture.getSize().x > 0) {
        float coinSpriteY = expBarY + (expBarHeight - coinSpriteHeight) / 2.0f;
        m_CoinSprite.setPosition(coinSpriteX, coinSpriteY);
    }

    // 3. Experience Bar (nằm bên trái coin sprite)
    float expBarX = coinSpriteX - expBarWidth - spacing;
    m_TopRightExpBarBackground.setPosition(expBarX, expBarY);
    m_TopRightExpBarFill.setPosition(expBarX + 2.0f, expBarY + 2.0f);

    // Calculate exp bar fill
    float progress = 0.0f;
    if (m_ExpToNext > 0) {
        progress = static_cast<float>(m_CurrentExp) / static_cast<float>(m_ExpToNext);
    }
    progress = std::min(1.0f, std::max(0.0f, progress));
    float fillWidth = (expBarWidth - 4.0f) * progress;
    m_TopRightExpBarFill.setSize(sf::Vector2f(fillWidth, expBarHeight - 4.0f));

    // Experience text on bar (center aligned)
    std::string expString = std::to_string(m_CurrentExp) + " / " + std::to_string(m_ExpToNext) + " EXP";
    m_TopRightExpText.setString(expString);
    sf::FloatRect expTextBounds = m_TopRightExpText.getLocalBounds();
    float expTextX = expBarX + (expBarWidth - expTextBounds.width) / 2.0f;
    float expTextY = expBarY + (expBarHeight - expTextBounds.height) / 2.0f - 1.0f;
    m_TopRightExpText.setPosition(expTextX, expTextY);

    // ===== DYNAMIC COLOR LOGIC - Thay đổi màu text dựa trên fill bar =====

    // Tính toán vị trí bắt đầu và kết thúc của text trong thanh exp
    float textStartX = expTextX - expBarX; // Vị trí relative của text trong bar
    float textEndX = textStartX + expTextBounds.width;

    // Tính toán vị trí hiện tại của thanh fill
    float currentFillEnd = fillWidth; // Vị trí kết thúc của thanh fill

    float colorChangeThreshold = 10.0f; // Khoảng cách trigger đổi màu

    if (currentFillEnd >= (textStartX - colorChangeThreshold)) {
        // Thanh exp đã tới gần text hoặc đè lên text -> đổi màu tối
        m_TopRightExpText.setFillColor(sf::Color(0, 0, 139)); // Navy blue
    }
    else {
        // Thanh exp chưa tới text -> giữ màu trắng
        m_TopRightExpText.setFillColor(sf::Color::White); // Trắng
    }

    // === TƯƠNG TỰ CHO EXP BAR CHÍNH (nếu cần) ===
    // (Áp dụng logic tương tự cho m_ExpText trong UpdateExpBarVisuals)

    // === 4. Level Text (phía trên exp bar, căn giữa với exp bar) ===
    m_TopRightLevelText.setString("Level " + std::to_string(m_CurrentLevel));
    sf::FloatRect levelBounds = m_TopRightLevelText.getLocalBounds();

    // Căn giữa level text với exp bar
    float levelTextX = expBarX + (expBarWidth - levelBounds.width) / 2.0f;
    float levelTextY = margin;

    m_TopRightLevelText.setPosition(levelTextX, levelTextY);
}

void PlayerTextManager::UpdateExpBarVisuals() {
    // Y chung cho cả level và exp bar
    float expBarY = m_GroupPosition.y + TEXT_COUNT * m_LineHeight + 10.0f;

    // ===== 1. Đặt Level dựa vào group position =====
    m_LevelText.setString("Level " + std::to_string(m_CurrentLevel));
    sf::FloatRect levelTextBounds = m_LevelText.getLocalBounds();
    float levelTextX = m_GroupPosition.x;
    float levelTextY = expBarY + (20.0f - levelTextBounds.height) / 2.0f - 2.0f; // 20.0f = chiều cao thanh exp
    m_LevelText.setPosition(levelTextX, levelTextY);

    // ===== 2. Thanh exp nằm ngay bên phải Level =====
    float expBarX = levelTextX + levelTextBounds.width + 10.0f; // cách 10px
    m_ExpBarBackground.setPosition(expBarX, expBarY);
    m_ExpBarFill.setPosition(expBarX + 2.0f, expBarY + 2.0f);

    // ===== 3. Tính toán độ đầy =====
    float progress = 0.0f;
    if (m_ExpToNext > 0) {
        progress = static_cast<float>(m_CurrentExp) / static_cast<float>(m_ExpToNext);
    }
    progress = std::min(1.0f, std::max(0.0f, progress));
    m_ExpBarFill.setPosition(expBarX + 2.0f, expBarY + 2.0f);
    float fillWidth = (m_ExpBarBackground.getSize().x - 4.0f) *
        std::min(1.0f, std::max(0.0f, (float)m_CurrentExp / m_ExpToNext));
    m_ExpBarFill.setSize(sf::Vector2f(fillWidth, m_ExpBarBackground.getSize().y - 4.0f));

    // ===== 4. Căn text EXP vào giữa thanh =====
    std::string expString = std::to_string(m_CurrentExp) + " / " + std::to_string(m_ExpToNext) + " EXP";
    m_ExpText.setString(expString);
    sf::FloatRect expTextBounds = m_ExpText.getLocalBounds();
    float expTextX = expBarX + (m_ExpBarBackground.getSize().x - expTextBounds.width) / 2.0f;
    float expTextY = expBarY + (m_ExpBarBackground.getSize().y - expTextBounds.height) / 2.0f - 2.0f;
    m_ExpText.setPosition(expTextX, expTextY);

    float textStartX = expTextX - expBarX;
    float textEndX = textStartX + expTextBounds.width;
    float currentFillEnd = fillWidth;
    float colorChangeThreshold = 10.0f;

    if (currentFillEnd >= (textStartX - colorChangeThreshold)) {
        m_ExpText.setFillColor(sf::Color(0, 0, 0));
    }
    else {
        m_ExpText.setFillColor(sf::Color::White); 
    }
}


void PlayerTextManager::Draw(sf::RenderWindow& window) {
    for (const auto& element : m_TextElements) {
        window.draw(element.text);
    }

    if (m_ShowExpBar) {
        window.draw(m_ExpBarBackground);
        window.draw(m_ExpBarFill);
        window.draw(m_ExpText);
        window.draw(m_LevelText);
    }

    if (m_ShowTopRightExpBar) {
        window.draw(m_TopRightExpBarBackground);
        window.draw(m_TopRightExpBarFill);
        window.draw(m_TopRightExpText);
        window.draw(m_TopRightLevelText);

        // Draw coin sprite if available
        if (m_CoinTexture.getSize().x > 0) {
            window.draw(m_CoinSprite);
        }
        window.draw(m_TopRightCoinText);
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

    // Include experience bar in bounds if showing
    if (m_ShowExpBar) {
        sf::FloatRect expBounds = m_ExpBarBackground.getGlobalBounds();
        float left = std::min(bounds.left, expBounds.left);
        float top = std::min(bounds.top, expBounds.top);
        float right = std::max(bounds.left + bounds.width, expBounds.left + expBounds.width);
        float bottom = std::max(bounds.top + bounds.height, expBounds.top + expBounds.height);

        bounds = sf::FloatRect(left, top, right - left, bottom - top);
    }

    return bounds;
}