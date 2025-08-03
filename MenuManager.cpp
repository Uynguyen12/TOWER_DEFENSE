#include "MenuManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "nlohmann/json.hpp" 
#include "SoundManager.h"

using json = nlohmann::json;

const sf::Color MenuManager::BUTTON_NORMAL_COLOR = sf::Color(70, 54, 62, 220);  // Dark wood tone
const sf::Color MenuManager::BUTTON_HOVER_COLOR = sf::Color(212, 175, 55, 240);  // Golden glow
const sf::Color MenuManager::BUTTON_PRESSED_COLOR = sf::Color(50, 40, 45, 220);  // Pressed dark wood
const sf::Color MenuManager::BACKGROUND_OVERLAY_COLOR = sf::Color(46, 42, 38, 180);  // Ancient stone overlay
const sf::Color MenuManager::BUTTON_GRADIENT_START = sf::Color(70, 54, 62, 220);
const sf::Color MenuManager::BUTTON_GRADIENT_END = sf::Color(85, 65, 75, 220);
const sf::Color MenuManager::BUTTON_HOVER_GRADIENT_START = sf::Color(212, 175, 55, 240);
const sf::Color MenuManager::BUTTON_HOVER_GRADIENT_END = sf::Color(255, 200, 80, 240);
const sf::Color MenuManager::INPUT_BOX_BORDER_COLOR = sf::Color(212, 175, 55, 255);  // Gold border
const sf::Color MenuManager::INPUT_BOX_FOCUS_COLOR = sf::Color(255, 215, 0, 255);    // Bright gold focus
const sf::Color MenuManager::TEXT_COLOR = sf::Color(255, 253, 240, 255);  // Ivory white
const sf::Color MenuManager::SLIDER_HANDLE_COLOR = sf::Color(212, 175, 55, 255);
const sf::Color MenuManager::SLIDER_HANDLE_HOVER_COLOR = sf::Color(255, 200, 80, 255);
const sf::Color MenuManager::SLIDER_TRACK_COLOR = sf::Color(70, 54, 62, 255);
const sf::Color MenuManager::TAB_ACTIVE_COLOR = sf::Color(255, 253, 240, 255);
const sf::Color MenuManager::TAB_INACTIVE_COLOR = sf::Color(150, 140, 130, 255);
const sf::Color MenuManager::TAB_UNDERLINE_COLOR = sf::Color(212, 175, 55, 255);
const sf::Color MenuManager::DELETE_BUTTON_COLOR = sf::Color(139, 69, 19, 220);      // Dark red-brown
const sf::Color MenuManager::DELETE_BUTTON_HOVER_COLOR = sf::Color(220, 20, 60, 220); // Crimson red

// Medieval decorative colors
const sf::Color MEDIEVAL_STONE_DARK = sf::Color(46, 42, 38, 255);
const sf::Color MEDIEVAL_STONE_LIGHT = sf::Color(80, 75, 70, 255);
const sf::Color MEDIEVAL_GOLD = sf::Color(212, 175, 55, 255);
const sf::Color MEDIEVAL_GOLD_BRIGHT = sf::Color(255, 215, 0, 255);
const sf::Color MEDIEVAL_IRON = sf::Color(105, 105, 105, 255);
const sf::Color MEDIEVAL_SHADOW = sf::Color(20, 15, 12, 150);
const sf::Color MenuManager::MEDIEVAL_WOOD_DARK = sf::Color(59, 47, 47, 220);      // #3b2f2f
const sf::Color MenuManager::MEDIEVAL_WOOD_LIGHT = sf::Color(90, 59, 29, 220);     // #5a3b1d  
const sf::Color MenuManager::MEDIEVAL_STONE_BASE = sf::Color(68, 68, 68, 220);     // #444
const sf::Color MenuManager::MEDIEVAL_GOLD_FRAME = sf::Color(212, 175, 55, 255);   // #d4af37
const sf::Color MenuManager::MEDIEVAL_SILVER_FRAME = sf::Color(196, 196, 196, 255); // #c4c4c4
const sf::Color MenuManager::MEDIEVAL_FIRE_GLOW = sf::Color(255, 170, 0, 180);     // #ffaa00
const sf::Color MenuManager::MEDIEVAL_MAGIC_GLOW = sf::Color(102, 224, 255, 180);  // #66e0ff
const sf::Color MenuManager::MEDIEVAL_TEXT_GOLD = sf::Color(252, 234, 187, 255);   // #fceabb
const sf::Color MenuManager::MEDIEVAL_RIVET_COLOR = sf::Color(105, 105, 105, 255); // Bạc sắt

const std::string MenuManager::SETTINGS_FILE_PATH = "settings.json";
const std::string MenuManager::PROFILES_FILE_PATH = "profiles.json";
const int MenuManager::MAX_PROFILES = 5;
const float MenuManager::RESOLUTION_SCROLL_SPEED = 50.0f;
const int MenuManager::MAX_VISIBLE_RESOLUTIONS = 5;

MenuManager::MenuManager()
    : m_currentState(MenuState::ProfileMenu)
    , m_previousState(MenuState::ProfileMenu)
    , m_currentProfile(nullptr)
    , m_waitingForNameInput(false)
    , m_showWarning(false)
    , m_warningTimer(0.0f)
    , m_gamePaused(false)
    , m_selectedResolutionIndex(0)
    , m_resolutionDropdownOpen(false)
    , m_resolutionScrollOffset(0.0f)
    , m_maxResolutionScroll(0.0f)
{
 
    if (m_availableResolutions.empty()) {
        m_availableResolutions = {
            {1920, 1080},
            {1680, 1050},
            {1600, 900},
            {1366, 768},
            {1280, 720},
            {1024, 768},
            {800, 600},
            {640, 480}
        };
    }

    std::sort(m_availableResolutions.begin(), m_availableResolutions.end(),
        [](const sf::Vector2u& a, const sf::Vector2u& b) {
            if (a.x != b.x) return a.x > b.x;
            return a.y > b.y;
        });
}


MenuManager::~MenuManager() {
    SaveProfilesToFile();
}

void MenuManager::Initialize(sf::RenderWindow& window) {
    m_windowSize = sf::Vector2f(window.getSize());
    LoadResources();
    LoadProfilesFromFile();
    LoadSettingsFromFile();
    SetMenuState(MenuState::ProfileMenu);

    // Ensure selected resolution is valid
    bool validResolution = false;
    for (size_t i = 0; i < m_availableResolutions.size(); ++i) {
        if (m_availableResolutions[i] == m_settings.resolution) {
            m_selectedResolutionIndex = i;
            validResolution = true;
            break;
        }
    }
    if (!validResolution) {
        m_settings.resolution = m_availableResolutions[0];
        m_selectedResolutionIndex = 0;
        ApplyResolution(window);
    }
}

void MenuManager::CreateGradientBackground() {
    sf::Image gradientImage;
    gradientImage.create(static_cast<unsigned int>(m_windowSize.x), static_cast<unsigned int>(m_windowSize.y));

    for (unsigned int y = 0; y < gradientImage.getSize().y; ++y) {
        float ratio = static_cast<float>(y) / gradientImage.getSize().y;
        sf::Uint8 r = static_cast<sf::Uint8>(20 + ratio * 40);
        sf::Uint8 g = static_cast<sf::Uint8>(20 + ratio * 60);
        sf::Uint8 b = static_cast<sf::Uint8>(40 + ratio * 80);

        for (unsigned int x = 0; x < gradientImage.getSize().x; ++x) {
            gradientImage.setPixel(x, y, sf::Color(r, g, b, 255));
        }
    }

    m_backgroundTexture.loadFromImage(gradientImage);
    m_backgroundSprite.setTexture(m_backgroundTexture);
}

void MenuManager::LoadResources() {
    if (!m_font.loadFromFile("Fonts/Luminari-Regular.ttf")) {
        std::cerr << "Warning: Could not load font. Using default font." << std::endl;
    }
    //Background
    if (!m_backgroundTexture.loadFromFile("image/Background.jpg")) {
        CreateGradientBackground();
    }
    else {
        m_backgroundSprite.setTexture(m_backgroundTexture);
        sf::Vector2u textureSize = m_backgroundTexture.getSize();
        sf::Vector2f scale(
            m_windowSize.x / static_cast<float>(textureSize.x),
            m_windowSize.y / static_cast<float>(textureSize.y)
        );
        m_backgroundSprite.setScale(scale);
    }

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(56);
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setStyle(sf::Text::Bold);

    m_titleShadow.setFont(m_font);
    m_titleShadow.setCharacterSize(56);
    m_titleShadow.setFillColor(sf::Color(0, 0, 0, 120));
    m_titleShadow.setStyle(sf::Text::Bold);

    m_warningText.setFont(m_font);
    m_warningText.setCharacterSize(24);
    m_warningText.setFillColor(sf::Color(255, 100, 100, 255));
    m_warningText.setStyle(sf::Text::Bold);

    m_inputPromptText.setFont(m_font);
    m_inputPromptText.setCharacterSize(36);
    m_inputPromptText.setFillColor(sf::Color::White);
    m_inputPromptText.setStyle(sf::Text::Bold);
    m_inputPromptText.setString("Enter your name:");

    m_inputText.setFont(m_font);
    m_inputText.setCharacterSize(28);
    m_inputText.setFillColor(sf::Color(20, 20, 20, 255));

    m_inputBox.setSize(sf::Vector2f(450, 60));
    m_inputBox.setFillColor(sf::Color(245, 245, 245, 250));
    m_inputBox.setOutlineThickness(3);
    m_inputBox.setOutlineColor(INPUT_BOX_BORDER_COLOR);

    m_inputBoxGlow.setSize(sf::Vector2f(460, 70));
    m_inputBoxGlow.setFillColor(sf::Color(120, 120, 255, 30));
    m_inputBoxGlow.setOutlineThickness(0);

    m_pauseBackground.setSize(sf::Vector2f(500, 400));
    m_pauseBackground.setFillColor(sf::Color(15, 15, 25, 200));
    m_pauseBackground.setOutlineThickness(2);
    m_pauseBackground.setOutlineColor(sf::Color(120, 120, 255, 180));

    m_backgroundOverlay.setSize(m_windowSize);
    m_backgroundOverlay.setFillColor(BACKGROUND_OVERLAY_COLOR);
}

void MenuManager::Update(sf::RenderWindow& window, float deltaTime) {
    UpdateButtons(window);
    UpdateSliders(window);

    if (m_currentState == MenuState::Settings) {
        UpdateTabButtons(window);
    }

    if (m_currentState == MenuState::Settings && m_resolutionDropdownOpen) {
        UpdateResolutionScroll(deltaTime);
    }

    if (m_showWarning) {
        m_warningTimer -= deltaTime;
        if (m_warningTimer <= 0.0f) {
            m_showWarning = false;
        }
    }
}

void MenuManager::CreateEnhancedButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
    std::function<void(sf::RenderWindow&)> callback, bool isDeleteButton) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);

    if (isDeleteButton) {
        button.shape.setFillColor(DELETE_BUTTON_COLOR);
    }
    else {
        button.shape.setFillColor(BUTTON_NORMAL_COLOR);
    }

    button.shape.setOutlineThickness(4);
    button.shape.setOutlineColor(MEDIEVAL_GOLD);
    button.isDeleteButton = isDeleteButton;

    button.text.setFont(m_font);
    button.text.setString(text);

    int fontSize = 26;
    if (text.length() > 20) {
        fontSize = 20;
    }
    else if (text.length() > 15) {
        fontSize = 22;
    }

    button.text.setCharacterSize(fontSize);
    button.text.setFillColor(TEXT_COLOR);
    button.text.setStyle(sf::Text::Bold);

    button.textShadow.setFont(m_font);
    button.textShadow.setString(text);
    button.textShadow.setCharacterSize(fontSize);
    button.textShadow.setFillColor(MEDIEVAL_SHADOW);
    button.textShadow.setStyle(sf::Text::Bold);

    CenterTextWithShadow(button.text, button.textShadow, button.shape);

    button.callback = callback;
    button.state = ButtonState::Normal;
    button.isVisible = true;

    m_buttons.push_back(button);
}

void MenuManager::CenterTextWithShadow(sf::Text& text, sf::Text& shadow, const sf::RectangleShape& shape) {
    sf::FloatRect textBounds = text.getLocalBounds();
    sf::FloatRect shapeBounds = shape.getGlobalBounds();

    float textX = shapeBounds.left + (shapeBounds.width - textBounds.width) / 2;
    float textY = shapeBounds.top + (shapeBounds.height - textBounds.height) / 2 - textBounds.top;

    text.setPosition(textX, textY);
    shadow.setPosition(textX + 2, textY + 2);
}

void MenuManager::DrawButtonShadow(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size) {
    // Multi-layered shadow for depth
    for (int i = 3; i >= 1; i--) {
        sf::RectangleShape shadow;
        shadow.setSize(size);
        shadow.setPosition(pos.x + i * 2, pos.y + i * 2);
        sf::Uint8 alpha = 40 * i;
        shadow.setFillColor(sf::Color(0, 0, 0, alpha));
        window.draw(shadow);
    }
}

void MenuManager::DrawButtonBase(sf::RenderWindow& window, const Button& button,
    sf::Vector2f pos, sf::Vector2f size) {
    sf::Color baseColor;

    switch (button.style) {
    case ButtonStyle::Shield:
        baseColor = button.isDeleteButton ? sf::Color(80, 30, 30, 220) : MEDIEVAL_WOOD_DARK;
        DrawShieldShape(window, pos, size, baseColor, button.state);
        break;

    case ButtonStyle::Scroll:
        baseColor = sf::Color(240, 230, 200, 220); // Parchment color
        DrawScrollShape(window, pos, size, baseColor, button.state);
        break;

    case ButtonStyle::WoodPlank:
        baseColor = MEDIEVAL_WOOD_LIGHT;
        DrawWoodPlankShape(window, pos, size, baseColor, button.state);
        break;

    case ButtonStyle::Stone:
        baseColor = MEDIEVAL_STONE_BASE;
        DrawStoneShape(window, pos, size, baseColor, button.state);
        break;
    }
}

void MenuManager::DrawShieldShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size,
    sf::Color baseColor, ButtonState state) {
    // Create shield-like shape with curves
    sf::ConvexShape shield;
    shield.setPointCount(8);

    // Shield outline points (medieval shield shape)
    shield.setPoint(0, sf::Vector2f(pos.x + size.x * 0.5f, pos.y)); // Top center
    shield.setPoint(1, sf::Vector2f(pos.x + size.x * 0.8f, pos.y + size.y * 0.15f)); // Top right
    shield.setPoint(2, sf::Vector2f(pos.x + size.x, pos.y + size.y * 0.4f)); // Right middle
    shield.setPoint(3, sf::Vector2f(pos.x + size.x * 0.9f, pos.y + size.y * 0.7f)); // Bottom right
    shield.setPoint(4, sf::Vector2f(pos.x + size.x * 0.5f, pos.y + size.y)); // Bottom point
    shield.setPoint(5, sf::Vector2f(pos.x + size.x * 0.1f, pos.y + size.y * 0.7f)); // Bottom left
    shield.setPoint(6, sf::Vector2f(pos.x, pos.y + size.y * 0.4f)); // Left middle
    shield.setPoint(7, sf::Vector2f(pos.x + size.x * 0.2f, pos.y + size.y * 0.15f)); // Top left

    if (state == ButtonState::Hovered) {
        baseColor.r = std::min(255, (int)baseColor.r + 40);
        baseColor.g = std::min(255, (int)baseColor.g + 30);
        baseColor.b = std::min(255, (int)baseColor.b + 20);
    }

    shield.setFillColor(baseColor);
    shield.setOutlineThickness(3);
    shield.setOutlineColor(MEDIEVAL_GOLD_FRAME);
    window.draw(shield);
}

void MenuManager::DrawScrollShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size,
    sf::Color baseColor, ButtonState state) {
    // Main scroll body
    sf::RectangleShape scrollBody;
    scrollBody.setSize(sf::Vector2f(size.x - 20, size.y));
    scrollBody.setPosition(pos.x + 10, pos.y);
    scrollBody.setFillColor(baseColor);
    scrollBody.setOutlineThickness(2);
    scrollBody.setOutlineColor(sf::Color(139, 69, 19, 255)); // Saddle brown
    window.draw(scrollBody);

    // Scroll ends (cylindrical)
    sf::CircleShape leftEnd(size.y / 2);
    leftEnd.setPosition(pos.x - size.y / 2, pos.y);
    leftEnd.setFillColor(sf::Color(139, 69, 19, 200));
    leftEnd.setOutlineThickness(2);
    leftEnd.setOutlineColor(sf::Color(101, 67, 33, 255));
    window.draw(leftEnd);

    sf::CircleShape rightEnd(size.y / 2);
    rightEnd.setPosition(pos.x + size.x - size.y / 2, pos.y);
    rightEnd.setFillColor(sf::Color(139, 69, 19, 200));
    rightEnd.setOutlineThickness(2);
    rightEnd.setOutlineColor(sf::Color(101, 67, 33, 255));
    window.draw(rightEnd);

    // Scroll texture lines
    for (int i = 1; i < 4; i++) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(size.x - 40, 1));
        line.setPosition(pos.x + 20, pos.y + (size.y / 4) * i);
        line.setFillColor(sf::Color(200, 180, 140, 100));
        window.draw(line);
    }
}

void MenuManager::DrawWoodPlankShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size,
    sf::Color baseColor, ButtonState state) {
    // Main wood plank
    sf::RectangleShape plank;
    plank.setSize(size);
    plank.setPosition(pos);
    plank.setFillColor(baseColor);
    window.draw(plank);

    // Wood grain lines
    for (int i = 0; i < 5; i++) {
        sf::RectangleShape grain;
        grain.setSize(sf::Vector2f(size.x, 2));
        grain.setPosition(pos.x, pos.y + (size.y / 6) * i + 5);
        grain.setFillColor(sf::Color(baseColor.r - 20, baseColor.g - 15, baseColor.b - 10, 80));
        window.draw(grain);
    }

    // Wood plank edges
    sf::RectangleShape topEdge, bottomEdge;
    topEdge.setSize(sf::Vector2f(size.x, 4));
    topEdge.setPosition(pos);
    topEdge.setFillColor(sf::Color(baseColor.r + 30, baseColor.g + 20, baseColor.b + 15, 255));
    window.draw(topEdge);

    bottomEdge.setSize(sf::Vector2f(size.x, 4));
    bottomEdge.setPosition(pos.x, pos.y + size.y - 4);
    bottomEdge.setFillColor(sf::Color(baseColor.r - 30, baseColor.g - 20, baseColor.b - 15, 255));
    window.draw(bottomEdge);
}

void MenuManager::DrawStoneShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size,
    sf::Color baseColor, ButtonState state) {
    // Main stone block
    sf::RectangleShape stone;
    stone.setSize(size);
    stone.setPosition(pos);
    stone.setFillColor(baseColor);
    window.draw(stone);

    // Stone texture - random small rectangles for rough surface
    static std::vector<sf::RectangleShape> stoneTexture;
    if (stoneTexture.empty()) {
        for (int i = 0; i < 15; i++) {
            sf::RectangleShape block;
            block.setSize(sf::Vector2f(rand() % 20 + 10, rand() % 15 + 5));
            block.setPosition(pos.x + rand() % (int)(size.x - 30),
                pos.y + rand() % (int)(size.y - 20));
            block.setFillColor(sf::Color(baseColor.r + rand() % 30 - 15,
                baseColor.g + rand() % 30 - 15,
                baseColor.b + rand() % 30 - 15, 40));
            stoneTexture.push_back(block);
        }
    }

    for (const auto& block : stoneTexture) {
        window.draw(block);
    }

    // Stone carved edges
    DrawBeveledEdge(window, pos, size, state);
}

void MenuManager::DrawDecorativeFrame(sf::RenderWindow& window, const Button& button,
    sf::Vector2f pos, sf::Vector2f size) {
    sf::Color frameColor = button.isDeleteButton ? sf::Color(180, 50, 50, 255) : MEDIEVAL_GOLD_FRAME;

    if (button.state == ButtonState::Hovered) {
        frameColor = sf::Color(255, 215, 0, 255); // Bright gold when hovered
    }

    // Outer decorative frame
    sf::RectangleShape outerFrame;
    outerFrame.setSize(sf::Vector2f(size.x + 8, size.y + 8));
    outerFrame.setPosition(pos.x - 4, pos.y - 4);
    outerFrame.setFillColor(sf::Color::Transparent);
    outerFrame.setOutlineThickness(3);
    outerFrame.setOutlineColor(frameColor);
    window.draw(outerFrame);

    // Inner frame detail
    sf::RectangleShape innerFrame;
    innerFrame.setSize(sf::Vector2f(size.x - 8, size.y - 8));
    innerFrame.setPosition(pos.x + 4, pos.y + 4);
    innerFrame.setFillColor(sf::Color::Transparent);
    innerFrame.setOutlineThickness(1);
    innerFrame.setOutlineColor(sf::Color(frameColor.r, frameColor.g, frameColor.b, 120));
    window.draw(innerFrame);

    // Corner decorations (medieval ornaments)
    DrawCornerOrnaments(window, pos, size, frameColor);
}

void MenuManager::DrawCornerOrnaments(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color color) {
    float ornamentSize = 8.0f;

    // Four corner ornaments - diamond shapes
    std::vector<sf::Vector2f> cornerPositions = {
        {pos.x - ornamentSize / 2, pos.y - ornamentSize / 2},           // Top-left
        {pos.x + size.x - ornamentSize / 2, pos.y - ornamentSize / 2}, // Top-right
        {pos.x - ornamentSize / 2, pos.y + size.y - ornamentSize / 2}, // Bottom-left
        {pos.x + size.x - ornamentSize / 2, pos.y + size.y - ornamentSize / 2} // Bottom-right
    };

    for (const auto& cornerPos : cornerPositions) {
        sf::ConvexShape ornament;
        ornament.setPointCount(4);
        ornament.setPoint(0, sf::Vector2f(cornerPos.x + ornamentSize / 2, cornerPos.y)); // Top
        ornament.setPoint(1, sf::Vector2f(cornerPos.x + ornamentSize, cornerPos.y + ornamentSize / 2)); // Right
        ornament.setPoint(2, sf::Vector2f(cornerPos.x + ornamentSize / 2, cornerPos.y + ornamentSize)); // Bottom
        ornament.setPoint(3, sf::Vector2f(cornerPos.x, cornerPos.y + ornamentSize / 2)); // Left

        ornament.setFillColor(color);
        ornament.setOutlineThickness(1);
        ornament.setOutlineColor(sf::Color(color.r - 50, color.g - 50, color.b - 50, 255));
        window.draw(ornament);
    }
}

void MenuManager::DrawButtonRivets(sf::RenderWindow& window, const Button& button,
    sf::Vector2f pos, sf::Vector2f size) {
    float rivetRadius = 3.0f;
    std::vector<sf::Vector2f> rivetPositions = {
        {pos.x + 15, pos.y + 15},                    // Top-left
        {pos.x + size.x - 15, pos.y + 15},          // Top-right
        {pos.x + 15, pos.y + size.y - 15},          // Bottom-left
        {pos.x + size.x - 15, pos.y + size.y - 15}  // Bottom-right
    };

    for (const auto& rivetPos : rivetPositions) {
        // Rivet base
        sf::CircleShape rivet(rivetRadius);
        rivet.setPosition(rivetPos.x - rivetRadius, rivetPos.y - rivetRadius);
        rivet.setFillColor(MEDIEVAL_RIVET_COLOR);
        rivet.setOutlineThickness(1);
        rivet.setOutlineColor(sf::Color(60, 60, 60, 255));
        window.draw(rivet);

        // Rivet highlight
        sf::CircleShape rivetHighlight(rivetRadius - 1);
        rivetHighlight.setPosition(rivetPos.x - rivetRadius + 1, rivetPos.y - rivetRadius + 1);
        rivetHighlight.setFillColor(sf::Color(160, 160, 160, 180));
        window.draw(rivetHighlight);
    }
}

void MenuManager::DrawButtonIcon(sf::RenderWindow& window, const Button& button,
    sf::Vector2f pos, sf::Vector2f size) {
    if (button.icon == ButtonIcon::None) return;

    float iconSize = size.y * 0.4f;
    sf::Vector2f iconPos(pos.x + 15, pos.y + (size.y - iconSize) / 2);

    switch (button.icon) {
    case ButtonIcon::Shield:
        DrawShieldIcon(window, iconPos, iconSize);
        break;
    case ButtonIcon::Sword:
        DrawSwordIcon(window, iconPos, iconSize);
        break;
    case ButtonIcon::Gear:
        DrawGearIcon(window, iconPos, iconSize);
        break;
    case ButtonIcon::Scroll:
        DrawScrollIcon(window, iconPos, iconSize);
        break;
    case ButtonIcon::Crown:
        DrawCrownIcon(window, iconPos, iconSize);
        break;
    case ButtonIcon::Target:
        DrawTargetIcon(window, iconPos, iconSize);
        break;
    }
}

void MenuManager::DrawShieldIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    sf::ConvexShape shield;
    shield.setPointCount(6);
    shield.setPoint(0, sf::Vector2f(pos.x + size / 2, pos.y));
    shield.setPoint(1, sf::Vector2f(pos.x + size, pos.y + size * 0.3f));
    shield.setPoint(2, sf::Vector2f(pos.x + size * 0.8f, pos.y + size * 0.7f));
    shield.setPoint(3, sf::Vector2f(pos.x + size / 2, pos.y + size));
    shield.setPoint(4, sf::Vector2f(pos.x + size * 0.2f, pos.y + size * 0.7f));
    shield.setPoint(5, sf::Vector2f(pos.x, pos.y + size * 0.3f));

    shield.setFillColor(MEDIEVAL_GOLD_FRAME);
    shield.setOutlineThickness(1);
    shield.setOutlineColor(sf::Color(150, 100, 0, 255));
    window.draw(shield);
}

void MenuManager::DrawSwordIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    // Sword blade
    sf::RectangleShape blade;
    blade.setSize(sf::Vector2f(4, size * 0.7f));
    blade.setPosition(pos.x + size / 2 - 2, pos.y);
    blade.setFillColor(sf::Color(200, 200, 200, 255));
    window.draw(blade);

    // Sword handle
    sf::RectangleShape handle;
    handle.setSize(sf::Vector2f(6, size * 0.25f));
    handle.setPosition(pos.x + size / 2 - 3, pos.y + size * 0.7f);
    handle.setFillColor(sf::Color(101, 67, 33, 255));
    window.draw(handle);

    // Cross guard
    sf::RectangleShape guard;
    guard.setSize(sf::Vector2f(size * 0.4f, 3));
    guard.setPosition(pos.x + size * 0.3f, pos.y + size * 0.65f);
    guard.setFillColor(MEDIEVAL_GOLD_FRAME);
    window.draw(guard);
}

void MenuManager::DrawGearIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    sf::CircleShape outerGear(size / 2, 8);
    outerGear.setPosition(pos);
    outerGear.setFillColor(MEDIEVAL_RIVET_COLOR);
    outerGear.setOutlineThickness(2);
    outerGear.setOutlineColor(MEDIEVAL_GOLD_FRAME);
    window.draw(outerGear);

    sf::CircleShape innerGear(size / 4);
    innerGear.setPosition(pos.x + size / 4, pos.y + size / 4);
    innerGear.setFillColor(sf::Color(80, 80, 80, 255));
    window.draw(innerGear);
}

void MenuManager::DrawScrollIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    sf::RectangleShape scroll;
    scroll.setSize(sf::Vector2f(size * 0.8f, size));
    scroll.setPosition(pos.x + size * 0.1f, pos.y);
    scroll.setFillColor(sf::Color(240, 230, 200, 255));
    scroll.setOutlineThickness(1);
    scroll.setOutlineColor(sf::Color(139, 69, 19, 255));
    window.draw(scroll);

    // Scroll lines
    for (int i = 1; i <= 3; i++) {
        sf::RectangleShape line;
        line.setSize(sf::Vector2f(size * 0.6f, 1));
        line.setPosition(pos.x + size * 0.2f, pos.y + (size / 4) * i);
        line.setFillColor(sf::Color(100, 100, 100, 180));
        window.draw(line);
    }
}

void MenuManager::DrawCrownIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    // Crown base
    sf::RectangleShape crownBase;
    crownBase.setSize(sf::Vector2f(size, size * 0.3f));
    crownBase.setPosition(pos.x, pos.y + size * 0.7f);
    crownBase.setFillColor(MEDIEVAL_GOLD_FRAME);
    window.draw(crownBase);

    // Crown points
    for (int i = 0; i < 5; i++) {
        sf::ConvexShape point;
        point.setPointCount(3);
        float pointWidth = size / 5;
        float pointHeight = (i == 2) ? size * 0.8f : size * 0.5f; // Middle point taller

        point.setPoint(0, sf::Vector2f(pos.x + i * pointWidth, pos.y + size * 0.7f));
        point.setPoint(1, sf::Vector2f(pos.x + i * pointWidth + pointWidth / 2, pos.y + size * 0.7f - pointHeight));
        point.setPoint(2, sf::Vector2f(pos.x + (i + 1) * pointWidth, pos.y + size * 0.7f));

        point.setFillColor(MEDIEVAL_GOLD_FRAME);
        point.setOutlineThickness(1);
        point.setOutlineColor(sf::Color(150, 100, 0, 255));
        window.draw(point);
    }
}

void MenuManager::DrawTargetIcon(sf::RenderWindow& window, sf::Vector2f pos, float size) {
    // Outer circle
    sf::CircleShape outerTarget(size / 2);
    outerTarget.setPosition(pos);
    outerTarget.setFillColor(sf::Color::Transparent);
    outerTarget.setOutlineThickness(2);
    outerTarget.setOutlineColor(sf::Color(200, 50, 50, 255));
    window.draw(outerTarget);

    // Inner circle
    sf::CircleShape innerTarget(size / 4);
    innerTarget.setPosition(pos.x + size / 4, pos.y + size / 4);
    innerTarget.setFillColor(sf::Color(200, 50, 50, 180));
    window.draw(innerTarget);

    // Crosshairs
    sf::RectangleShape hLine, vLine;
    hLine.setSize(sf::Vector2f(size, 2));
    hLine.setPosition(pos.x, pos.y + size / 2 - 1);
    hLine.setFillColor(sf::Color(200, 50, 50, 255));
    window.draw(hLine);

    vLine.setSize(sf::Vector2f(2, size));
    vLine.setPosition(pos.x + size / 2 - 1, pos.y);
    vLine.setFillColor(sf::Color(200, 50, 50, 255));
    window.draw(vLine);
}

void MenuManager::DrawMagicalEffects(sf::RenderWindow& window, const Button& button,
    sf::Vector2f pos, sf::Vector2f size) {
    static sf::Clock magicClock;
    float time = magicClock.getElapsedTime().asSeconds();

    // Choose effect color based on button type
    sf::Color effectColor = button.isDeleteButton ? MEDIEVAL_FIRE_GLOW : MEDIEVAL_MAGIC_GLOW;

    // Pulsing glow effect
    float glowIntensity = (sin(time * 3.0f) + 1.0f) * 0.5f;
    sf::Uint8 alpha = (sf::Uint8)(glowIntensity * 120 + 60);

    // Multiple glow layers for depth
    for (int i = 3; i >= 1; i--) {
        sf::RectangleShape glow;
        glow.setSize(sf::Vector2f(size.x + i * 8, size.y + i * 8));
        glow.setPosition(pos.x - i * 4, pos.y - i * 4);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2);
        glow.setOutlineColor(sf::Color(effectColor.r, effectColor.g, effectColor.b, alpha / i));
        window.draw(glow);
    }

    // Sparkle particles around button
    DrawSparkleParticles(window, pos, size, effectColor, time);
}

void MenuManager::DrawSparkleParticles(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size,
    sf::Color color, float time) {
    static std::vector<sf::Vector2f> sparkleOffsets;

    // Initialize sparkle positions if empty
    if (sparkleOffsets.empty()) {
        for (int i = 0; i < 8; i++) {
            sparkleOffsets.push_back(sf::Vector2f(
                (rand() % (int)size.x) - size.x * 0.1f,
                (rand() % (int)size.y) - size.y * 0.1f
            ));
        }
    }

    for (size_t i = 0; i < sparkleOffsets.size(); i++) {
        float sparkleTime = time * 2.0f + i * 0.3f;
        float alpha = (sin(sparkleTime) + 1.0f) * 0.5f * 255.0f;

        sf::CircleShape sparkle(2.0f, 4); // 4-pointed star
        sparkle.setPosition(pos.x + sparkleOffsets[i].x, pos.y + sparkleOffsets[i].y);
        sparkle.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
        sparkle.setOutlineThickness(1);
        sparkle.setOutlineColor(sf::Color(color.r, color.g, color.b, (sf::Uint8)(alpha * 0.8f)));
        window.draw(sparkle);
    }
}

void MenuManager::DrawMedievalButton(sf::RenderWindow& window, const Button& button) {
    sf::Vector2f pos = button.shape.getPosition();
    sf::Vector2f size = button.shape.getSize();

    // 1. Draw deep shadow for 3D depth
    DrawButtonShadow(window, pos, size);

    // 2. Draw button base according to style
    DrawButtonBase(window, button, pos, size);

    // 3. Draw decorative frame based on button type
    DrawDecorativeFrame(window, button, pos, size);

    // 4. Draw texture overlay
    DrawTextureOverlay(window, button, pos, size);

    // 5. Draw rivets/studs
    DrawButtonRivets(window, button, pos, size);

    // 6. Draw icon if present
    DrawButtonIcon(window, button, pos, size);

    // 7. Draw magical effects based on state
    if (button.state == ButtonState::Hovered) {
        DrawMagicalEffects(window, button, pos, size);
    }

    // 8. Draw text with enhanced styling
    DrawButtonText(window, button);

    // 9. Draw decorative gem
    if (button.state == ButtonState::Hovered) {
        DrawDecorativeGem(window, button, pos, size);
    }
}

void MenuManager::DrawMedievalCorners(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, ButtonState state) {
    float cornerSize = 12.0f;
    sf::Color cornerColor = (state == ButtonState::Hovered) ? MEDIEVAL_GOLD : MEDIEVAL_STONE_LIGHT;

    // Top-left corner decoration
    sf::CircleShape cornerDecor(cornerSize / 2, 8);
    cornerDecor.setFillColor(cornerColor);
    cornerDecor.setPosition(pos.x - cornerSize / 4, pos.y - cornerSize / 4);
    window.draw(cornerDecor);

    // Top-right corner
    cornerDecor.setPosition(pos.x + size.x - cornerSize * 3 / 4, pos.y - cornerSize / 4);
    window.draw(cornerDecor);

    // Bottom-left corner
    cornerDecor.setPosition(pos.x - cornerSize / 4, pos.y + size.y - cornerSize * 3 / 4);
    window.draw(cornerDecor);

    // Bottom-right corner
    cornerDecor.setPosition(pos.x + size.x - cornerSize * 3 / 4, pos.y + size.y - cornerSize * 3 / 4);
    window.draw(cornerDecor);
}

void MenuManager::DrawBeveledEdge(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, ButtonState state) {
    float bevelWidth = 4.0f;
    sf::Color lightEdge = (state == ButtonState::Pressed) ? MEDIEVAL_STONE_DARK : MEDIEVAL_STONE_LIGHT;
    sf::Color darkEdge = (state == ButtonState::Pressed) ? MEDIEVAL_STONE_LIGHT : MEDIEVAL_STONE_DARK;

    // Top light edge
    sf::RectangleShape topEdge;
    topEdge.setSize(sf::Vector2f(size.x, bevelWidth));
    topEdge.setPosition(pos);
    topEdge.setFillColor(lightEdge);
    window.draw(topEdge);

    // Left light edge
    sf::RectangleShape leftEdge;
    leftEdge.setSize(sf::Vector2f(bevelWidth, size.y));
    leftEdge.setPosition(pos);
    leftEdge.setFillColor(lightEdge);
    window.draw(leftEdge);

    // Bottom dark edge
    sf::RectangleShape bottomEdge;
    bottomEdge.setSize(sf::Vector2f(size.x, bevelWidth));
    bottomEdge.setPosition(pos.x, pos.y + size.y - bevelWidth);
    bottomEdge.setFillColor(darkEdge);
    window.draw(bottomEdge);

    // Right dark edge
    sf::RectangleShape rightEdge;
    rightEdge.setSize(sf::Vector2f(bevelWidth, size.y));
    rightEdge.setPosition(pos.x + size.x - bevelWidth, pos.y);
    rightEdge.setFillColor(darkEdge);
    window.draw(rightEdge);
}

void MenuManager::DrawMetalStuds(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size) {
    float studRadius = 3.0f;
    sf::CircleShape stud(studRadius);
    stud.setFillColor(MEDIEVAL_IRON);
    stud.setOutlineThickness(1);
    stud.setOutlineColor(MEDIEVAL_GOLD);

    // Four corner studs
    float margin = 15.0f;

    // Top-left
    stud.setPosition(pos.x + margin - studRadius, pos.y + margin - studRadius);
    window.draw(stud);

    // Top-right
    stud.setPosition(pos.x + size.x - margin - studRadius, pos.y + margin - studRadius);
    window.draw(stud);

    // Bottom-left
    stud.setPosition(pos.x + margin - studRadius, pos.y + size.y - margin - studRadius);
    window.draw(stud);

    // Bottom-right
    stud.setPosition(pos.x + size.x - margin - studRadius, pos.y + size.y - margin - studRadius);
    window.draw(stud);
}

void MenuManager::DrawMagicalGlow(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size) {
    // Outer magical glow
    sf::RectangleShape glow;
    glow.setSize(sf::Vector2f(size.x + 16, size.y + 16));
    glow.setPosition(pos.x - 8, pos.y - 8);
    glow.setFillColor(sf::Color::Transparent);
    glow.setOutlineThickness(6);
    glow.setOutlineColor(sf::Color(255, 170, 77, 60)); // Warm magical glow
    window.draw(glow);

    // Inner glow
    glow.setSize(sf::Vector2f(size.x + 8, size.y + 8));
    glow.setPosition(pos.x - 4, pos.y - 4);
    glow.setOutlineThickness(3);
    glow.setOutlineColor(sf::Color(255, 200, 100, 80));
    window.draw(glow);
}

void MenuManager::DrawSparkleEffect(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size) {
    static sf::Clock sparkleClock;
    static std::vector<sf::Vector2f> sparklePositions;

    // Initialize sparkle positions if empty
    if (sparklePositions.empty()) {
        for (int i = 0; i < 6; ++i) {
            sparklePositions.push_back(sf::Vector2f(
                pos.x + (rand() % (int)size.x),
                pos.y + (rand() % (int)size.y)
            ));
        }
    }

    // Animate sparkles
    float time = sparkleClock.getElapsedTime().asSeconds();
    for (size_t i = 0; i < sparklePositions.size(); ++i) {
        float alpha = (sin(time * 3.0f + i * 0.5f) + 1.0f) * 0.5f * 255.0f;

        sf::CircleShape sparkle(2.0f, 4); // Star shape
        sparkle.setPosition(sparklePositions[i]);
        sparkle.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
        window.draw(sparkle);
    }

    // Update sparkle positions periodically
    if (time > 1.0f) {
        sparkleClock.restart();
        sparklePositions.clear();
    }
}

void MenuManager::DrawMedievalScrollIndicators(sf::RenderWindow& window) {
    float scrollbarWidth = 15;
    float dropdownWidth = 300;
    sf::Vector2f dropdownPos = sf::Vector2f(
        (m_windowSize.x - dropdownWidth) / 2,
        m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT + 5
    );
    float scrollbarX = dropdownPos.x + dropdownWidth - scrollbarWidth;

    // Medieval scroll arrows with decorative design
    if (m_resolutionScrollOffset > 0) {
        // Up arrow with medieval styling
        sf::CircleShape upArrowBg(12, 3);
        upArrowBg.setPosition(scrollbarX, dropdownPos.y - 25);
        upArrowBg.setFillColor(MEDIEVAL_GOLD);
        upArrowBg.setOutlineThickness(2);
        upArrowBg.setOutlineColor(MEDIEVAL_IRON);
        window.draw(upArrowBg);

        sf::Text upArrow;
        upArrow.setFont(m_font);
        upArrow.setString("▲");
        upArrow.setCharacterSize(12);
        upArrow.setFillColor(TEXT_COLOR);
        upArrow.setPosition(scrollbarX + 6, dropdownPos.y - 20);
        window.draw(upArrow);
    }

    if (m_resolutionScrollOffset < m_maxResolutionScroll) {
        // Down arrow with medieval styling
        float dropdownHeight = 5 * BUTTON_HEIGHT;
        sf::CircleShape downArrowBg(12, 3);
        downArrowBg.setPosition(scrollbarX, dropdownPos.y + dropdownHeight + 10);
        downArrowBg.setFillColor(MEDIEVAL_GOLD);
        downArrowBg.setOutlineThickness(2);
        downArrowBg.setOutlineColor(MEDIEVAL_IRON);
        window.draw(downArrowBg);

        sf::Text downArrow;
        downArrow.setFont(m_font);
        downArrow.setString("▼");
        downArrow.setCharacterSize(12);
        downArrow.setFillColor(TEXT_COLOR);
        downArrow.setPosition(scrollbarX + 6, dropdownPos.y + dropdownHeight + 15);
        window.draw(downArrow);
    }
}


void MenuManager::Draw(sf::RenderWindow& window) {
    if (m_backgroundTexture.getSize().x > 0) {
        window.draw(m_backgroundSprite);
    }

    window.draw(m_backgroundOverlay);

    DrawBackgroundParticles(window);

    window.draw(m_titleShadow);
    window.draw(m_titleText);

    if (m_currentState == MenuState::Settings) {
        DrawTabButtons(window);
    }

    DrawButtons(window);
    DrawSliders(window);

    if (m_resolutionDropdownOpen && m_currentState == MenuState::Settings &&
        m_currentSettingsTab == SettingsTab::Configuration) {
        DrawResolutionDropdown(window);
    }

    if (m_waitingForNameInput) {
        window.draw(m_inputBoxGlow);
        window.draw(m_inputBox);
        window.draw(m_inputPromptText);
        window.draw(m_inputText);

        DrawInputCursor(window);
    }

    if (m_showWarning) {
        DrawWarningWithBackground(window);
    }

    if (m_gamePaused && m_currentState == MenuState::GamePlay) {
        DrawPauseMenu(window);
    }
}

void MenuManager::DrawBackgroundParticles(sf::RenderWindow& window) {
    static std::vector<sf::CircleShape> particles;
    static sf::Clock particleClock;

    if (particles.empty()) {
        for (int i = 0; i < 20; ++i) {
            sf::CircleShape particle(2);
            particle.setPosition(
                static_cast<float>(rand() % static_cast<int>(m_windowSize.x)),
                static_cast<float>(rand() % static_cast<int>(m_windowSize.y))
            );
            particle.setFillColor(sf::Color(120, 120, 255, 50));
            particles.push_back(particle);
        }
    }

    float deltaTime = particleClock.restart().asSeconds();
    for (auto& particle : particles) {
        sf::Vector2f pos = particle.getPosition();
        pos.y -= 20 * deltaTime; 
        pos.x += sin(pos.y * 0.01f) * 10 * deltaTime; 

        if (pos.y < -10) {
            pos.y = m_windowSize.y + 10;
            pos.x = static_cast<float>(rand() % static_cast<int>(m_windowSize.x));
        }

        particle.setPosition(pos);
        window.draw(particle);
    }
}

void MenuManager::DrawInputCursor(sf::RenderWindow& window) {
    static sf::Clock cursorClock;
    static bool cursorVisible = true;

    if (cursorClock.getElapsedTime().asSeconds() > 0.5f) {
        cursorVisible = !cursorVisible;
        cursorClock.restart();
    }

    if (cursorVisible) {
        sf::RectangleShape cursor;
        cursor.setSize(sf::Vector2f(2, 30));
        cursor.setFillColor(sf::Color::Black);

        sf::FloatRect textBounds = m_inputText.getGlobalBounds();
        cursor.setPosition(
            textBounds.left + textBounds.width + 2,
            m_inputText.getPosition().y + 5
        );

        window.draw(cursor);
    }
}

void MenuManager::DrawWarningWithBackground(sf::RenderWindow& window) {
    sf::FloatRect warningBounds = m_warningText.getGlobalBounds();

    sf::RectangleShape warningBg;
    warningBg.setSize(sf::Vector2f(warningBounds.width + 40, warningBounds.height + 20));
    warningBg.setPosition(warningBounds.left - 20, warningBounds.top - 10);
    warningBg.setFillColor(sf::Color(20, 20, 20, 200));
    warningBg.setOutlineThickness(2);
    warningBg.setOutlineColor(sf::Color(255, 100, 100, 255));

    window.draw(warningBg);
    window.draw(m_warningText);
}

void MenuManager::DrawResolutionDropdown(sf::RenderWindow& window) {
    const int MAX_VISIBLE = 5;
    float dropdownWidth = 300;
    float dropdownHeight = MAX_VISIBLE * BUTTON_HEIGHT;

    sf::Vector2f dropdownPos = sf::Vector2f(
        (m_windowSize.x - dropdownWidth) / 2,
        m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT + 5
    );

    sf::RectangleShape dropdownBg;
    dropdownBg.setSize(sf::Vector2f(dropdownWidth, dropdownHeight));
    dropdownBg.setPosition(dropdownPos);
    dropdownBg.setFillColor(sf::Color(40, 40, 40, 240));
    dropdownBg.setOutlineThickness(2);
    dropdownBg.setOutlineColor(sf::Color::White);
    window.draw(dropdownBg);

    int totalItems = m_availableResolutions.size();
    int maxScroll = std::max(0, totalItems - MAX_VISIBLE);
    int scrollIndex = std::min(maxScroll, (int)(m_resolutionScrollOffset / BUTTON_HEIGHT));

    if (totalItems > MAX_VISIBLE) {
        DrawScrollbar(window, dropdownPos, dropdownWidth, dropdownHeight, scrollIndex, maxScroll);
    }

    for (int i = 0; i < MAX_VISIBLE && (scrollIndex + i) < totalItems; ++i) {
        int resIndex = scrollIndex + i;
        sf::Vector2f itemPos = sf::Vector2f(dropdownPos.x, dropdownPos.y + i * BUTTON_HEIGHT);

        sf::RectangleShape itemBg;
        itemBg.setSize(sf::Vector2f(dropdownWidth, BUTTON_HEIGHT));
        itemBg.setPosition(itemPos);

        if (resIndex == m_selectedResolutionIndex) {
            itemBg.setFillColor(sf::Color(80, 80, 120, 200));
        }
        else {
            itemBg.setFillColor(sf::Color(60, 60, 60, 150));
        }

        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
        if (itemBg.getGlobalBounds().contains(mousePos)) {
            itemBg.setFillColor(sf::Color(100, 100, 100, 200));
        }

        window.draw(itemBg);

        // Item text
        sf::Text itemText;
        itemText.setFont(m_font);
        itemText.setString(std::to_string(m_availableResolutions[resIndex].x) + " x " +
            std::to_string(m_availableResolutions[resIndex].y));
        itemText.setCharacterSize(20);
        itemText.setFillColor(TEXT_COLOR);

        // Center text in item
        sf::FloatRect textBounds = itemText.getLocalBounds();
        itemText.setPosition(
            itemPos.x + (dropdownWidth - textBounds.width) / 2,
            itemPos.y + (BUTTON_HEIGHT - textBounds.height) / 2 - textBounds.top
        );

        window.draw(itemText);
    }

    if (totalItems > MAX_VISIBLE) {
        DrawScrollbar(window, dropdownPos, dropdownWidth, dropdownHeight, scrollIndex, maxScroll);
    }
}

void MenuManager::DrawScrollbar(sf::RenderWindow& window, sf::Vector2f dropdownPos,
    float dropdownWidth, float dropdownHeight, int scrollIndex, int maxScroll) {
    float scrollbarWidth = 15;
    float scrollbarX = dropdownPos.x + dropdownWidth - scrollbarWidth;

    // Scrollbar track
    sf::RectangleShape scrollTrack;
    scrollTrack.setSize(sf::Vector2f(scrollbarWidth, dropdownHeight));
    scrollTrack.setPosition(scrollbarX, dropdownPos.y);
    scrollTrack.setFillColor(sf::Color(30, 30, 30, 200));
    scrollTrack.setOutlineThickness(1);
    scrollTrack.setOutlineColor(sf::Color(60, 60, 60, 255));
    window.draw(scrollTrack);

    // Scrollbar handle
    if (maxScroll > 0) {
        float handleHeight = (dropdownHeight * 5) / m_availableResolutions.size();
        handleHeight = std::max(20.0f, handleHeight); 

        float handleY = dropdownPos.y + (scrollIndex * (dropdownHeight - handleHeight)) / maxScroll;

        sf::RectangleShape scrollHandle;
        scrollHandle.setSize(sf::Vector2f(scrollbarWidth - 4, handleHeight));
        scrollHandle.setPosition(scrollbarX + 2, handleY);
        scrollHandle.setFillColor(sf::Color(120, 120, 255, 200));
        scrollHandle.setOutlineThickness(1);
        scrollHandle.setOutlineColor(sf::Color::White);
        window.draw(scrollHandle);
    }

    // Scroll arrows
    sf::Text upArrow, downArrow;
    upArrow.setFont(m_font);
    downArrow.setFont(m_font);
    upArrow.setString("▲");
    downArrow.setString("▼");
    upArrow.setCharacterSize(12);
    downArrow.setCharacterSize(12);
    upArrow.setFillColor(scrollIndex > 0 ? sf::Color::White : sf::Color(100, 100, 100, 255));
    downArrow.setFillColor(scrollIndex < maxScroll ? sf::Color::White : sf::Color(100, 100, 100, 255));

    upArrow.setPosition(scrollbarX + 2, dropdownPos.y - 20);
    downArrow.setPosition(scrollbarX + 2, dropdownPos.y + dropdownHeight + 5);

    window.draw(upArrow);
    window.draw(downArrow);
}

void MenuManager::DrawPauseMenu(sf::RenderWindow& window) {
    sf::Vector2f centerPos = sf::Vector2f(
        (m_windowSize.x - m_pauseBackground.getSize().x) / 2,
        (m_windowSize.y - m_pauseBackground.getSize().y) / 2
    );
    m_pauseBackground.setPosition(centerPos);

    window.draw(m_pauseBackground);

    for (const auto& button : m_pauseButtons) {
        if (button.isVisible) {
            window.draw(button.shape);
            window.draw(button.text);
        }
    }
}

void MenuManager::HandleInput(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

            // Prioritize sliders
            for (auto& slider : m_sliders) {
                if (slider.isVisible && IsMouseOverSlider(slider, mousePos)) {
                    slider.isDragging = true;
                    UpdateSliderValue(slider, mousePos);
                    return;
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                if (m_currentState == MenuState::Settings && m_resolutionDropdownOpen) {
                    float scrollDelta = event.mouseWheelScroll.delta * RESOLUTION_SCROLL_SPEED;
                    m_resolutionScrollOffset -= scrollDelta;
                    m_resolutionScrollOffset = std::max(0.0f, std::min(m_resolutionScrollOffset, m_maxResolutionScroll));
                    UpdateVisibleResolutions();
                }
            }

            if (m_currentState == MenuState::Settings) {
                for (auto& tabButton : m_tabButtons) {
                    sf::FloatRect textBounds = tabButton.text.getGlobalBounds();
                    if (textBounds.contains(mousePos)) {
                        if (tabButton.callback) {
                            tabButton.callback();
                        }
                        return;
                    }
                }
            }

            if (m_gamePaused && m_currentState == MenuState::GamePlay) {
                for (auto& button : m_pauseButtons) {
                    if (button.isVisible && IsMouseOverButton(button, mousePos)) {
                        button.state = ButtonState::Pressed;
                        if (button.callback) {
                            button.callback(window);
                        }
                        break;
                    }
                }
            }
            else {
                if (m_resolutionDropdownOpen) {
                    HandleResolutionDropdownClick(mousePos, window);
                }
                else {
                    for (auto& button : m_buttons) {
                        if (button.isVisible && IsMouseOverButton(button, mousePos)) {
                            button.state = ButtonState::Pressed;
                            if (button.callback) {
                                button.callback(window);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // Handle mouse wheel scrolling for resolution dropdown
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (m_resolutionDropdownOpen && m_currentState == MenuState::Settings &&
            m_currentSettingsTab == SettingsTab::Configuration) {

            sf::Vector2f dropdownPos = sf::Vector2f(
                (m_windowSize.x - 300) / 2,
                m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT + 5
            );
            sf::FloatRect dropdownArea(dropdownPos.x, dropdownPos.y, 300, 5 * BUTTON_HEIGHT);

            if (dropdownArea.contains(sf::Vector2f(sf::Mouse::getPosition(window)))) {
                float scrollDelta = -event.mouseWheelScroll.delta * RESOLUTION_SCROLL_SPEED;
                m_resolutionScrollOffset += scrollDelta;

                // Clamp scroll offset
                int maxScroll = std::max(0, (int)m_availableResolutions.size() - 5);
                m_resolutionScrollOffset = std::max(0.0f, 
                    std::min(m_resolutionScrollOffset, static_cast<float>(maxScroll * BUTTON_HEIGHT)));
                UpdateVisibleResolutions();
            }
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            for (auto& slider : m_sliders) {
                slider.isDragging = false;
            }
        }
    }

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
        for (auto& slider : m_sliders) {
            if (slider.isDragging) {
                UpdateSliderValue(slider, mousePos);
            }
        }
    }

    if (m_waitingForNameInput && event.type == sf::Event::TextEntered) {
        HandleTextInput(event.text.unicode);
    }

    if (event.type == sf::Event::KeyPressed) {
        if (m_waitingForNameInput && event.key.code == sf::Keyboard::Enter) {
            if (!m_inputBuffer.empty()) {
                CreateNewProfile(m_inputBuffer);
                SetWaitingForInput(false);
                ClearInputText();
                SetMenuState(MenuState::MainMenu);
            }
            else {
                ShowWarningMessage("Please enter a name!");
            }
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            if (m_waitingForNameInput) {
                SetWaitingForInput(false);
                ClearInputText();
                SetMenuState(MenuState::ProfileMenu);
            }
            else {
                if (m_resolutionDropdownOpen) {
                    m_resolutionDropdownOpen = false;
                }  
                else {
                    switch (m_currentState) {
                    case MenuState::ProfileMenu:
                        std::cout << "Exit game requested" << std::endl;
                        break;
                    case MenuState::CreateProfile:
                        SetMenuState(MenuState::ProfileMenu);
                        break;
                    case MenuState::ChooseProfile:
                        SetMenuState(MenuState::ProfileMenu);
                        break;
                    case MenuState::MainMenu:
                        SetMenuState(MenuState::ProfileMenu);
                        break;
                    case MenuState::PlayMenu:
                        SetMenuState(MenuState::MainMenu);
                        break;
                    case MenuState::Settings:
                        if (m_previousState == MenuState::GamePlay) {
                            m_currentState = MenuState::GamePlay; // Đặt lại trạng thái thành GamePlay
                            m_gamePaused = true; // Đảm bảo trạng thái paused
                            m_buttons.clear(); // Xóa giao diện Settings
                            m_sliders.clear();
                            m_resolutionDropdownButtons.clear();
                            CreatePauseMenu(); // Tạo lại PauseMenu
                            std::cout << "Returning to Pause Menu from Settings" << std::endl;
                        }
                        else {
                            SetMenuState(MenuState::MainMenu);
                        }
                        break;
                    case MenuState::GamePlay:
                        TogglePauseMenu();
                        break;
                    }
                }
            }
        }
    }
}

void MenuManager::TogglePauseMenu() {
    m_gamePaused = !m_gamePaused;

    if (m_gamePaused) {
        CreatePauseMenu();
        m_currentState = MenuState::GamePlay;
        std::cout << "Game paused" << std::endl;
    }
    else {
        m_currentState = MenuState::GamePlay; // Đảm bảo trạng thái là GamePlay
        m_pauseButtons.clear();
        m_buttons.clear(); // Xóa mọi nút menu khác
        m_sliders.clear();
        m_resolutionDropdownButtons.clear();
        std::cout << "Game resumed" << std::endl;
    }
}

void MenuManager::CreatePauseMenu() {
    m_pauseButtons.clear();

    sf::Vector2f centerPos = sf::Vector2f(
        (m_windowSize.x - m_pauseBackground.getSize().x) / 2,
        (m_windowSize.y - m_pauseBackground.getSize().y) / 2
    );

    float buttonWidth = 200;
    float buttonHeight = 50;
    float buttonSpacing = 70;
    float startY = centerPos.y + 40;

    CreatePauseButton("Continue",
        sf::Vector2f(centerPos.x + (m_pauseBackground.getSize().x - buttonWidth) / 2, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        [this](sf::RenderWindow&) {
            TogglePauseMenu();
        });

    CreatePauseButton("Settings",
        sf::Vector2f(centerPos.x + (m_pauseBackground.getSize().x - buttonWidth) / 2, startY + buttonSpacing),
        sf::Vector2f(buttonWidth, buttonHeight),
        [this](sf::RenderWindow&) {
            m_previousState = MenuState::GamePlay;
            SetMenuState(MenuState::Settings);
        });

    CreatePauseButton("Exit",
        sf::Vector2f(centerPos.x + (m_pauseBackground.getSize().x - buttonWidth) / 2, startY + buttonSpacing * 2),
        sf::Vector2f(buttonWidth, buttonHeight),
        [this](sf::RenderWindow&) {
            m_gamePaused = false;
            m_pauseButtons.clear();
            SetMenuState(MenuState::ProfileMenu);
            std::cout << "Exiting to Profile Menu" << std::endl;
        });
}

void MenuManager::CreatePauseButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
    std::function<void(sf::RenderWindow&)> callback) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(BUTTON_NORMAL_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.isDeleteButton = false;

    button.text.setFont(m_font);
    button.text.setString(text);

    int fontSize = 20;
    if (text.length() > 20) {
        fontSize = 16;
    }
    else if (text.length() > 15) {
        fontSize = 18;
    }
    button.text.setCharacterSize(fontSize);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);

    button.callback = callback;
    button.state = ButtonState::Normal;
    button.isVisible = true;

    m_pauseButtons.push_back(button);
}

void MenuManager::SetMenuState(MenuState newState) {
    m_previousState = m_currentState;
    m_currentState = newState;

    m_buttons.clear();
    m_sliders.clear();
    m_resolutionDropdownButtons.clear();
    m_resolutionDropdownOpen = false;

    m_titleText.setString("");
    m_titleShadow.setString("");

    if (newState != MenuState::Settings) {
        m_tabButtons.clear();
    }

    if (!(newState == MenuState::Settings && m_previousState == MenuState::GamePlay)) {
        m_gamePaused = false;
        m_pauseButtons.clear();
    }

    switch (newState) {
    case MenuState::ProfileMenu:
        CreateProfileMenu();
        break;
    case MenuState::CreateProfile:
        CreateNewProfileMenu();
        break;
    case MenuState::ChooseProfile:
        CreateChooseProfileMenu();
        break;
    case MenuState::MainMenu:
        CreateMainMenu();
        break;
    case MenuState::PlayMenu:
        CreatePlayMenu();
        break;
    case MenuState::Settings:
        m_currentSettingsTab = SettingsTab::Music;
        CreateSettingsMenu();
        break;
    case MenuState::GamePlay:
        m_titleText.setString("");
        m_titleShadow.setString("");
        break;
    }
}

void MenuManager::UpdateWindowSize(sf::RenderWindow& window) {
    sf::Vector2f oldWindowSize = m_windowSize;
    m_windowSize = sf::Vector2f(window.getSize());

    std::cout << "Updating window size to: " << m_windowSize.x << "x" << m_windowSize.y << std::endl;
    if (oldWindowSize != m_windowSize) {
        RecreateCurrentMenuUI();
    }

    if (m_currentState == MenuState::Settings) {
        CreatePauseMenu();
    }
}

void MenuManager::HandleResolutionDropdownClick(sf::Vector2f mousePos, sf::RenderWindow& window) {
    const int MAX_VISIBLE = 5;
    float dropdownWidth = 300;
    sf::Vector2f dropdownPos = sf::Vector2f(
        (m_windowSize.x - dropdownWidth) / 2,
        m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT + 5
    );

    sf::FloatRect dropdownArea(dropdownPos.x, dropdownPos.y, dropdownWidth, MAX_VISIBLE * BUTTON_HEIGHT);

    if (dropdownArea.contains(mousePos)) {
        // Calculate which item was clicked
        int clickedIndex = (int)((mousePos.y - dropdownPos.y) / BUTTON_HEIGHT);
        int scrollIndex = (int)(m_resolutionScrollOffset / BUTTON_HEIGHT);
        int actualIndex = scrollIndex + clickedIndex;

        if (actualIndex >= 0 && actualIndex < m_availableResolutions.size()) {
            // Select the resolution
            m_selectedResolutionIndex = actualIndex;
            m_settings.resolution = m_availableResolutions[actualIndex];

            // Update button text
            std::string newResText = std::to_string(m_settings.resolution.x) + " x " +
                std::to_string(m_settings.resolution.y);
            m_resolutionButton.text.setString(newResText);
            CenterText(m_resolutionButton.text, m_resolutionButton.shape);

            // Apply resolution and close dropdown
            ApplyResolution(window);
            m_resolutionDropdownOpen = false;
            m_resolutionScrollOffset = 0.0f;

            std::cout << "Resolution changed to: " << m_settings.resolution.x << "x"
                << m_settings.resolution.y << std::endl;
        }
    }
    else {
        // Click outside dropdown, close it
        m_resolutionDropdownOpen = false;
        m_resolutionScrollOffset = 0.0f;
    }
}

sf::Vector2f MenuManager::ScalePosition(sf::Vector2f originalPos, sf::Vector2f oldSize, sf::Vector2f newSize) {
    if (oldSize.x == 0 || oldSize.y == 0) return originalPos;

    float scaleX = newSize.x / oldSize.x;
    float scaleY = newSize.y / oldSize.y;

    return sf::Vector2f(originalPos.x * scaleX, originalPos.y * scaleY);
}

void MenuManager::CreateSettingsMenu() {
    m_gamePaused = false;
    m_titleText.setString("Settings");
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 60);

    // Update title shadow
    m_titleShadow.setString("Settings");
    m_titleShadow.setPosition(
        m_titleText.getPosition().x + 3,
        m_titleText.getPosition().y + 3
    );

    // Tạo tab buttons với vị trí tối ưu
    m_tabButtons.clear();
    float tabY = 140;
    float tabSpacing = 250;
    float startX = (m_windowSize.x - tabSpacing) / 2;

    CreateTabButton("Music", sf::Vector2f(startX, tabY), SettingsTab::Music,
        [this]() { SetActiveTab(SettingsTab::Music); });

    CreateTabButton("Configuration", sf::Vector2f(startX + tabSpacing, tabY), SettingsTab::Configuration,
        [this]() { SetActiveTab(SettingsTab::Configuration); });

    // Set default active tab
    SetActiveTab(m_currentSettingsTab);

    // Tạo nội dung dựa trên tab hiện tại
    CreateSettingsContent();

    // Nút Close ở vị trí tối ưu
    CreateButton("Close",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, m_windowSize.y - 200),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow& window) {
            SaveSettingsToFile();
            if (m_previousState == MenuState::GamePlay) {
                m_currentState = MenuState::GamePlay;
                m_gamePaused = true;
                m_buttons.clear();
                m_sliders.clear();
                m_tabButtons.clear();
                m_resolutionDropdownButtons.clear();
                CreatePauseMenu();
                std::cout << "Returning to Pause Menu from Settings" << std::endl;
            }
            else {
                SetMenuState(MenuState::MainMenu);
            }
        });
}

void MenuManager::UpdateResolutionScroll(float deltaTime) {
    int totalItems = m_availableResolutions.size();
    int maxScroll = std::max(0, totalItems - MAX_VISIBLE_RESOLUTIONS);
    m_maxResolutionScroll = maxScroll * BUTTON_HEIGHT;

    if (m_resolutionDropdownOpen) {
        m_resolutionScrollOffset = std::max(0.0f, std::min(m_resolutionScrollOffset, m_maxResolutionScroll));
    }
}


void MenuManager::UpdateVisibleResolutions() {
    m_resolutionDropdownButtons.clear();

    if (!m_resolutionDropdownOpen) {
        return;
    }

    float dropdownY = m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT;
    int startIndex = (int)(m_resolutionScrollOffset / BUTTON_HEIGHT);
    int endIndex = std::min(startIndex + MAX_VISIBLE_RESOLUTIONS, (int)m_availableResolutions.size());

    for (int i = startIndex; i < endIndex; ++i) {
        std::string resText = std::to_string(m_availableResolutions[i].x) + " x " + std::to_string(m_availableResolutions[i].y);
        float yPos = dropdownY + (i - startIndex) * BUTTON_HEIGHT;

        CreateButton(resText,
            sf::Vector2f((m_windowSize.x - 300) / 2, yPos),
            sf::Vector2f(300, BUTTON_HEIGHT),
            [this, i](sf::RenderWindow& window) {
                m_selectedResolutionIndex = i;
                m_settings.resolution = m_availableResolutions[i];

                std::string newResText = std::to_string(m_settings.resolution.x) + " x " + std::to_string(m_settings.resolution.y);
                m_resolutionButton.text.setString(newResText);
                CenterText(m_resolutionButton.text, m_resolutionButton.shape);

                ApplyResolution(window);
                m_resolutionDropdownOpen = false;
                m_resolutionScrollOffset = 0.0f;

                std::cout << "Resolution changed to: " << m_settings.resolution.x << "x" << m_settings.resolution.y << std::endl;
            },
            true);
    }

    if (m_availableResolutions.size() > MAX_VISIBLE_RESOLUTIONS) {
        m_maxResolutionScroll = (m_availableResolutions.size() - MAX_VISIBLE_RESOLUTIONS) * BUTTON_HEIGHT;
    }
    else {
        m_maxResolutionScroll = 0.0f;
    }
}

void MenuManager::CreateSettingsContent() {
    // Xóa nội dung cũ (trừ tab buttons và close button)
    m_sliders.clear();

    // Xóa buttons trừ close button (giữ lại button cuối cùng là Close)
    if (!m_buttons.empty()) {
        Button closeButton = m_buttons.back();
        m_buttons.clear();
        m_buttons.push_back(closeButton);
    }
    m_resolutionDropdownButtons.clear();

    float contentStartY = 220;

    if (m_currentSettingsTab == SettingsTab::Music) {
        // Tạo Music content
        float sliderY = contentStartY;

        CreateSlider("Music",
            sf::Vector2f((m_windowSize.x - 400) / 2, sliderY),
            m_settings.musicVolume,
            [this](float value) {
                m_settings.musicVolume = value;
                SoundManager::getInstance().SetMusicVolume(value);
                if (m_musicVolumeCallback) {
                    m_musicVolumeCallback(value);
                }
            });

        sliderY += 80;
        CreateSlider("Sound effect",
            sf::Vector2f((m_windowSize.x - 400) / 2, sliderY),
            m_settings.sfxVolume,
            [this](float value) {
                m_settings.sfxVolume = value;
                SoundManager::getInstance().SetSoundVolume(value);
                if (m_sfxVolumeCallback) {
                    m_sfxVolumeCallback(value);
                }
            });
    }

    else if (m_currentSettingsTab == SettingsTab::Configuration) {
        // Tạo Configuration content
        float buttonY = contentStartY + 60;

        std::string resolutionText = std::to_string(m_settings.resolution.x) + " x " + std::to_string(m_settings.resolution.y);
        m_resolutionButton = Button();
        m_resolutionButton.shape.setPosition(sf::Vector2f((m_windowSize.x - 300) / 2, contentStartY));
        m_resolutionButton.shape.setSize(sf::Vector2f(300, BUTTON_HEIGHT));
        m_resolutionButton.shape.setFillColor(BUTTON_NORMAL_COLOR);
        m_resolutionButton.shape.setOutlineThickness(2);
        m_resolutionButton.shape.setOutlineColor(sf::Color::White);
        m_resolutionButton.isDeleteButton = false;

        m_resolutionButton.text.setFont(m_font);
        m_resolutionButton.text.setString(resolutionText);
        m_resolutionButton.text.setCharacterSize(24);
        m_resolutionButton.text.setFillColor(TEXT_COLOR);
        CenterText(m_resolutionButton.text, m_resolutionButton.shape);

        m_resolutionButton.callback = [this](sf::RenderWindow& window) {
            m_resolutionDropdownOpen = !m_resolutionDropdownOpen;
            if (m_resolutionDropdownOpen) {
                m_resolutionScrollOffset = 0.0f;
                if (m_availableResolutions.size() > MAX_VISIBLE_RESOLUTIONS) {
                    m_maxResolutionScroll = (m_availableResolutions.size() - MAX_VISIBLE_RESOLUTIONS) * BUTTON_HEIGHT;
                }
                else {
                    m_maxResolutionScroll = 0.0f;
                }
            }
            else {
                m_resolutionScrollOffset = 0.0f;
                m_resolutionDropdownButtons.clear(); // Xóa nút khi đóng
            }

            // Tạo nút dựa trên phạm vi cuộn
            float dropdownY = m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT;
            int startIndex = (int)(m_resolutionScrollOffset / BUTTON_HEIGHT);
            int endIndex = std::min(startIndex + MAX_VISIBLE_RESOLUTIONS, (int)m_availableResolutions.size());
            for (size_t i = startIndex; i < endIndex; ++i) {
                std::string resText = std::to_string(m_availableResolutions[i].x) + " x " + std::to_string(m_availableResolutions[i].y);
                CreateButton(resText,
                    sf::Vector2f((m_windowSize.x - 300) / 2, dropdownY + (i - startIndex) * BUTTON_HEIGHT),
                    sf::Vector2f(300, BUTTON_HEIGHT),
                    [this, i](sf::RenderWindow& window) {
                        sf::Vector2f oldPosition = m_resolutionButton.shape.getPosition();

                        m_selectedResolutionIndex = i;
                        m_settings.resolution = m_availableResolutions[i];

                        std::string newResText = std::to_string(m_settings.resolution.x) + " x " + std::to_string(m_settings.resolution.y);

                        if (!m_buttons.empty()) {
                            m_buttons[0].text.setString(newResText);
                            CenterText(m_buttons[0].text, m_buttons[0].shape);
                        }

                        m_resolutionButton.text.setString(newResText);
                        CenterText(m_resolutionButton.text, m_resolutionButton.shape);

                        ApplyResolution(window);

                        m_resolutionDropdownOpen = false;
                        UpdateWindowSize(window);

                        std::cout << "Resolution changed to: " << m_settings.resolution.x << "x" << m_settings.resolution.y << std::endl;
                    },
                    true);
            }
            };

        m_resolutionButton.state = ButtonState::Normal;
        m_resolutionButton.isVisible = true;

        m_buttons.insert(m_buttons.begin(), m_resolutionButton);

        sf::Text resolutionLabel;
        resolutionLabel.setFont(m_font);
        resolutionLabel.setString("Resolution:");
        resolutionLabel.setCharacterSize(20);
        resolutionLabel.setFillColor(TEXT_COLOR);
        sf::FloatRect labelBounds = resolutionLabel.getLocalBounds();
        resolutionLabel.setPosition(
            (m_windowSize.x - labelBounds.width) / 2,
            buttonY - 35
        );
    }
}

void MenuManager::CreateTabButton(const std::string& text, sf::Vector2f position,
    SettingsTab tab, std::function<void()> callback) {
    TabButton tabButton;

    tabButton.text.setFont(m_font);
    tabButton.text.setString(text);
    tabButton.text.setCharacterSize(24);
    tabButton.text.setPosition(position);
    tabButton.callback = callback;
    tabButton.isActive = (tab == m_currentSettingsTab);

    // Tạo underline
    sf::FloatRect textBounds = tabButton.text.getLocalBounds();
    tabButton.underline.setSize(sf::Vector2f(textBounds.width, 3));
    tabButton.underline.setPosition(position.x, position.y + textBounds.height + 5);
    tabButton.underline.setFillColor(TAB_UNDERLINE_COLOR);

    // Set màu dựa trên trạng thái active
    if (tabButton.isActive) {
        tabButton.text.setFillColor(TAB_ACTIVE_COLOR);
        tabButton.underline.setFillColor(TAB_UNDERLINE_COLOR);
    }
    else {
        tabButton.text.setFillColor(TAB_INACTIVE_COLOR);
        tabButton.underline.setFillColor(sf::Color::Transparent);
    }

    m_tabButtons.push_back(tabButton);
}

void MenuManager::SetActiveTab(SettingsTab tab) {
    m_currentSettingsTab = tab;

    // Update tab buttons
    for (auto& tabButton : m_tabButtons) {
        tabButton.isActive = false;
        tabButton.text.setFillColor(TAB_INACTIVE_COLOR);
        tabButton.underline.setFillColor(sf::Color::Transparent);
    }

    // Set active tab
    if (tab == SettingsTab::Music && m_tabButtons.size() > 0) {
        m_tabButtons[0].isActive = true;
        m_tabButtons[0].text.setFillColor(TAB_ACTIVE_COLOR);
        m_tabButtons[0].underline.setFillColor(TAB_UNDERLINE_COLOR);
    }
    else if (tab == SettingsTab::Configuration && m_tabButtons.size() > 1) {
        m_tabButtons[1].isActive = true;
        m_tabButtons[1].text.setFillColor(TAB_ACTIVE_COLOR);
        m_tabButtons[1].underline.setFillColor(TAB_UNDERLINE_COLOR);
    }

    // Recreate content
    CreateSettingsContent();
}

void MenuManager::UpdateTabButtons(sf::RenderWindow& window) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    for (auto& tabButton : m_tabButtons) {
        sf::FloatRect textBounds = tabButton.text.getGlobalBounds();
        if (textBounds.contains(mousePos)) {
            if (!tabButton.isActive) {
                tabButton.text.setFillColor(sf::Color(200, 200, 200, 255));
            }
        }
        else {
            if (!tabButton.isActive) {
                tabButton.text.setFillColor(TAB_INACTIVE_COLOR);
            }
        }
    }
}

void MenuManager::DrawTabButtons(sf::RenderWindow& window) {
    for (const auto& tabButton : m_tabButtons) {
        window.draw(tabButton.text);
        window.draw(tabButton.underline);
    }
}

void MenuManager::CreateSlider(const std::string& label, sf::Vector2f position, float value,
    std::function<void(float)> callback) {
    Slider slider;

    slider.label.setFont(m_font);
    slider.label.setString(label + ": " + std::to_string((int)value));
    slider.label.setCharacterSize(22);
    slider.label.setFillColor(TEXT_COLOR);
    slider.label.setPosition(position.x, position.y - 35);

    // Track với kích thước phù hợp
    slider.track.setSize(sf::Vector2f(350, 12));
    slider.track.setPosition(position.x, position.y);
    slider.track.setFillColor(SLIDER_TRACK_COLOR);
    slider.track.setOutlineThickness(2);
    slider.track.setOutlineColor(sf::Color(100, 100, 100, 255));

    // Handle với thiết kế đẹp hơn
    slider.handle.setSize(sf::Vector2f(24, 24));
    float handleX = position.x + (value / 100.0f) * (350 - 24);
    slider.handle.setPosition(handleX, position.y - 6);
    slider.handle.setFillColor(SLIDER_HANDLE_COLOR);
    slider.handle.setOutlineThickness(2);
    slider.handle.setOutlineColor(sf::Color::White);

    slider.value = value;
    slider.callback = callback;
    slider.isVisible = true;
    slider.isDragging = false;

    m_sliders.push_back(slider);
}

void MenuManager::UpdateSliders(sf::RenderWindow& window) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    for (auto& slider : m_sliders) {
        if (!slider.isVisible) continue;

        if (IsMouseOverSlider(slider, mousePos) || slider.isDragging) {
            slider.handle.setFillColor(SLIDER_HANDLE_HOVER_COLOR);
        }
        else {
            slider.handle.setFillColor(SLIDER_HANDLE_COLOR);
        }
    }
}

void MenuManager::DrawSliders(sf::RenderWindow& window) {
    for (const auto& slider : m_sliders) {
        if (slider.isVisible) {
            window.draw(slider.label);

            sf::RectangleShape trackBorder = slider.track;
            trackBorder.setFillColor(sf::Color::Transparent);
            trackBorder.setOutlineThickness(2);
            trackBorder.setOutlineColor(MEDIEVAL_GOLD);
            window.draw(trackBorder);
            window.draw(slider.track);

            sf::RectangleShape handleShadow = slider.handle;
            handleShadow.setPosition(slider.handle.getPosition().x + 2, slider.handle.getPosition().y + 2);
            handleShadow.setFillColor(MEDIEVAL_SHADOW);
            handleShadow.setOutlineThickness(0);
            window.draw(handleShadow);

            window.draw(slider.handle);

            sf::CircleShape gem(4);
            sf::Vector2f handleCenter = sf::Vector2f(
                slider.handle.getPosition().x + slider.handle.getSize().x / 2 - 4,
                slider.handle.getPosition().y + slider.handle.getSize().y / 2 - 4
            );
            gem.setPosition(handleCenter);
            gem.setFillColor(sf::Color(100, 200, 255, 200));
            gem.setOutlineThickness(1);
            gem.setOutlineColor(MEDIEVAL_GOLD);
            window.draw(gem);
        }
    }
}

bool MenuManager::IsMouseOverSlider(const Slider& slider, sf::Vector2f mousePos) {
    return slider.handle.getGlobalBounds().contains(mousePos) ||
        slider.track.getGlobalBounds().contains(mousePos);
}

void MenuManager::UpdateSliderValue(Slider& slider, sf::Vector2f mousePos) {
    sf::FloatRect trackBounds = slider.track.getGlobalBounds();

    float relativeX = mousePos.x - trackBounds.left;
    relativeX = std::max(0.0f, std::min(relativeX, trackBounds.width - 20));

    float newValue = (relativeX / (trackBounds.width - 20)) * 100.0f;
    newValue = std::max(0.0f, std::min(newValue, 100.0f));

    slider.value = newValue;

    float handleX = trackBounds.left + (newValue / 100.0f) * (trackBounds.width - 20);
    slider.handle.setPosition(handleX, slider.handle.getPosition().y);

    std::string labelText = slider.label.getString();
    size_t colonPos = labelText.find(':');
    if (colonPos != std::string::npos) {
        labelText = labelText.substr(0, colonPos + 2) + std::to_string((int)newValue);
        slider.label.setString(labelText);
    }

    if (slider.callback) {
        slider.callback(newValue);
    }
}

void MenuManager::CreateProfileMenu() {
    m_titleShadow.setString("");

    m_titleText.setString("Profile Menu");
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 100);

    m_titleShadow.setString("Profile Menu");
    m_titleShadow.setPosition(
        m_titleText.getPosition().x + 3,
        m_titleText.getPosition().y + 3
    );

    float startY = m_windowSize.y / 2 - 100;

    CreateButton("New Profile",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) {
            if (m_profiles.size() >= MAX_PROFILES) {
                ShowWarningMessage("Maximum number of profiles reached!");
            }
            else {
                SetMenuState(MenuState::CreateProfile);
            }
        });

    CreateButton("Existing Profile",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) { SetMenuState(MenuState::ChooseProfile); });

    CreateButton("Play as Guest",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + BUTTON_SPACING * 2),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) {
            m_currentProfile = nullptr;
            SetMenuState(MenuState::MainMenu);
        });

    CreateButton("Exit",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + BUTTON_SPACING * 3),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow& window) {
            if (m_exitCallback) {
                m_exitCallback(window);
            }
        });
}



void MenuManager::CreateNewProfileMenu() {
    m_titleText.setString("Create New Profile");
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 120);

    m_titleShadow.setString("Create New Profile");
    m_titleShadow.setPosition(m_titleText.getPosition().x + 3, m_titleText.getPosition().y + 3);

    sf::FloatRect promptBounds = m_inputPromptText.getLocalBounds();
    m_inputPromptText.setPosition(
        (m_windowSize.x - promptBounds.width) / 2,
        m_windowSize.y / 2 - 120
    );

    m_inputBoxGlow.setPosition(
        (m_windowSize.x - m_inputBoxGlow.getSize().x) / 2,
        m_windowSize.y / 2 - 55
    );

    m_inputBox.setPosition(
        (m_windowSize.x - m_inputBox.getSize().x) / 2,
        m_windowSize.y / 2 - 50
    );

    m_inputText.setPosition(
        m_inputBox.getPosition().x + 15,
        m_inputBox.getPosition().y + 15
    );

    SetWaitingForInput(true);

    CreateEnhancedButton("Back",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, m_windowSize.y / 2 + 50),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) {
            SetWaitingForInput(false);
            ClearInputText();
            SetMenuState(MenuState::ProfileMenu);
        });
}

void MenuManager::CreateChooseProfileMenu() {
    m_titleText.setString("Choose Profile");
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 100);

    float startY = 200;
    int buttonIndex = 0;

    for (size_t i = 0; i < m_profiles.size(); ++i) {
        std::string profileInfo = m_profiles[i].name + " - Lv." + std::to_string(m_profiles[i].level);
        float profileButtonWidth = BUTTON_WIDTH - 80;
        CreateButton(profileInfo,
            sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + buttonIndex * BUTTON_SPACING),
            sf::Vector2f(profileButtonWidth, BUTTON_HEIGHT),
            [this, i](sf::RenderWindow&) {
                SelectProfile(i);
                SetMenuState(MenuState::MainMenu);
            });

        CreateDeleteButton("X",
            sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2 + profileButtonWidth + 10, startY + buttonIndex * BUTTON_SPACING),
            sf::Vector2f(60, BUTTON_HEIGHT),
            [this, i](sf::RenderWindow&) {
                DeleteProfile(i);
            });

        buttonIndex++;
    }

    if (m_profiles.size() < MAX_PROFILES) {
        CreateButton("+ New Profile",
            sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + buttonIndex * BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this](sf::RenderWindow&) { SetMenuState(MenuState::CreateProfile); });
        buttonIndex++;
    }

    CreateButton("Back",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + buttonIndex * BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) { SetMenuState(MenuState::ProfileMenu); });
}

void MenuManager::CreateMainMenu() {
    std::string title = m_currentProfile ? "Welcome, " + m_currentProfile->name + "!" : "Welcome, Guest!";
    m_titleText.setString(title);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 100);

    float startY = m_windowSize.y / 2 - 50;

    CreateButton("Play",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) { SetMenuState(MenuState::PlayMenu); });

    CreateButton("Settings",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) { SetMenuState(MenuState::Settings); });

    CreateButton("Back",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + BUTTON_SPACING * 2),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) { SetMenuState(MenuState::ProfileMenu); });
}

void MenuManager::CreatePlayMenu() {
    m_titleText.setString("Select Level");
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2, 100);

    float startY = 200;
    int totalLevels = 4;
    for (int i = 1; i <= totalLevels; ++i) {
        std::string levelText = "Level " + std::to_string(i);
        CreateButton(levelText,
            sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + (i - 1) * BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this, i](sf::RenderWindow&) {
                if (m_startGameCallback) {
                    m_startGameCallback(i);
                }
            });
    }

    CreateButton("Back",
        sf::Vector2f((m_windowSize.x - BUTTON_WIDTH) / 2, startY + totalLevels * BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this](sf::RenderWindow&) {
            if (m_previousState == MenuState::GamePlay) {
                m_currentState = MenuState::GamePlay;
                m_gamePaused = true;
                m_buttons.clear();
                m_sliders.clear();
                m_resolutionDropdownButtons.clear();
                CreatePauseMenu();
                std::cout << "Returning to Pause Menu from PlayMenu" << std::endl;
            }
            else {
                SetMenuState(MenuState::MainMenu);
            }
        });
}

void MenuManager::CreateButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
    std::function<void(sf::RenderWindow&)> callback, bool isDropdownButton) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(BUTTON_NORMAL_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.isDeleteButton = false;

    button.text.setFont(m_font);
    button.text.setString(text);

    int fontSize = 24;
    if (text.length() > 20) {
        fontSize = 18;
    }
    else if (text.length() > 15) {
        fontSize = 20;
    }
    button.text.setCharacterSize(fontSize);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);

    button.callback = callback;
    button.state = ButtonState::Normal;
    button.isVisible = true;

    if (isDropdownButton) {
        m_resolutionDropdownButtons.push_back(button);
    }
    else {
        m_buttons.push_back(button);
    }
}

void MenuManager::CreateDeleteButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
    std::function<void(sf::RenderWindow&)> callback) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(DELETE_BUTTON_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.isDeleteButton = true;

    button.text.setFont(m_font);
    button.text.setString(text);
    button.text.setCharacterSize(20);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);

    button.callback = callback;
    button.state = ButtonState::Normal;
    button.isVisible = true;

    m_buttons.push_back(button);
}

void MenuManager::UpdateButtons(sf::RenderWindow& window) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    for (auto& button : m_buttons) {
        if (!button.isVisible) continue;

        if (IsMouseOverButton(button, mousePos)) {
            if (button.state != ButtonState::Pressed) {
                button.state = ButtonState::Hovered;

                if (button.isDeleteButton) {
                    button.shape.setFillColor(DELETE_BUTTON_HOVER_COLOR);
                }
                else {
                    button.shape.setFillColor(BUTTON_HOVER_GRADIENT_START);
                }
            }
        }
        else {
            button.state = ButtonState::Normal;
            if (button.isDeleteButton) {
                button.shape.setFillColor(DELETE_BUTTON_COLOR);
            }
            else {
                button.shape.setFillColor(BUTTON_NORMAL_COLOR);
            }
        }

        if (button.state == ButtonState::Pressed) {
            button.state = ButtonState::Normal;
        }
    }

    // Update pause buttons with medieval styling
    if (m_gamePaused && m_currentState == MenuState::GamePlay) {
        for (auto& button : m_pauseButtons) {
            if (!button.isVisible) continue;

            if (IsMouseOverButton(button, mousePos)) {
                if (button.state != ButtonState::Pressed) {
                    button.state = ButtonState::Hovered;
                    button.shape.setFillColor(BUTTON_HOVER_GRADIENT_START);
                }
            }
            else {
                button.state = ButtonState::Normal;
                button.shape.setFillColor(BUTTON_NORMAL_COLOR);
            }

            if (button.state == ButtonState::Pressed) {
                button.state = ButtonState::Normal;
                button.shape.setFillColor(BUTTON_NORMAL_COLOR);
            }
        }
    }

    if (m_resolutionDropdownOpen) {
        for (auto& button : m_resolutionDropdownButtons) {
            if (!button.isVisible) continue;

            if (IsMouseOverButton(button, mousePos)) {
                if (button.state != ButtonState::Pressed) {
                    button.state = ButtonState::Hovered;
                    button.shape.setFillColor(BUTTON_HOVER_GRADIENT_START);
                }
            }
            else {
                button.state = ButtonState::Normal;
                button.shape.setFillColor(BUTTON_NORMAL_COLOR);
            }

            if (button.state == ButtonState::Pressed) {
                button.state = ButtonState::Normal;
                button.shape.setFillColor(BUTTON_NORMAL_COLOR);
            }
        }
    }
}

void MenuManager::DrawButtons(sf::RenderWindow& window) {
    for (const auto& button : m_buttons) {
        if (button.isVisible) {
            DrawMedievalButton(window, button);
            sf::RectangleShape buttonShadow = button.shape;
            buttonShadow.setPosition(
                button.shape.getPosition().x + 4,
                button.shape.getPosition().y + 4
            );
            buttonShadow.setFillColor(sf::Color(0, 0, 0, 80));
            buttonShadow.setOutlineThickness(0);
            window.draw(buttonShadow);

            window.draw(button.shape);

            window.draw(button.textShadow);
            window.draw(button.text);
        }
    }

    if (m_resolutionDropdownOpen && m_maxResolutionScroll > 0) {
        DrawMedievalScrollIndicators(window);
        sf::RectangleShape scrollIndicator;
        scrollIndicator.setSize(sf::Vector2f(10, 20));
        scrollIndicator.setFillColor(sf::Color(150, 150, 150, 200));

        float dropdownY = m_resolutionButton.shape.getPosition().y + BUTTON_HEIGHT;
        float dropdownHeight = MAX_VISIBLE_RESOLUTIONS * BUTTON_HEIGHT;

        // Top scroll indicator (nếu không ở đầu)
        if (m_resolutionScrollOffset > 0) {
            scrollIndicator.setPosition((m_windowSize.x - 300) / 2 + 310, dropdownY - 15);
            // Vẽ mũi tên lên
            sf::Text upArrow;
            upArrow.setFont(m_font);
            upArrow.setString("▲");
            upArrow.setCharacterSize(12);
            upArrow.setFillColor(sf::Color::White);
            upArrow.setPosition(scrollIndicator.getPosition().x + 2, scrollIndicator.getPosition().y);
            window.draw(upArrow);
        }

        // Bottom scroll indicator (nếu không ở cuối)
        if (m_resolutionScrollOffset < m_maxResolutionScroll) {
            scrollIndicator.setPosition((m_windowSize.x - 300) / 2 + 310, dropdownY + dropdownHeight);
            // Vẽ mũi tên xuống
            sf::Text downArrow;
            downArrow.setFont(m_font);
            downArrow.setString("▼");
            downArrow.setCharacterSize(12);
            downArrow.setFillColor(sf::Color::White);
            downArrow.setPosition(scrollIndicator.getPosition().x + 2, scrollIndicator.getPosition().y);
            window.draw(downArrow);
        }
    }   
}

bool MenuManager::IsMouseOverButton(const Button& button, sf::Vector2f mousePos) {
    return button.shape.getGlobalBounds().contains(mousePos);
}

void MenuManager::CenterText(sf::Text& text, const sf::RectangleShape& shape) {
    sf::FloatRect textBounds = text.getLocalBounds();
    sf::FloatRect shapeBounds = shape.getGlobalBounds();

    text.setPosition(
        shapeBounds.left + (shapeBounds.width - textBounds.width) / 2,
        shapeBounds.top + (shapeBounds.height - textBounds.height) / 2 - textBounds.top
    );
}

void MenuManager::CreateNewProfile(const std::string& name) {
    for (const auto& profile : m_profiles) {
        if (profile.name == name) {
            ShowWarningMessage("Profile name already exists!");
            return;
        }
    }

    if (m_profiles.size() >= MAX_PROFILES) {
        ShowWarningMessage("Maximum number of profiles reached!");
        return;
    }

    PlayerProfile newProfile(name);
    m_profiles.push_back(newProfile);
    m_currentProfile = &m_profiles.back();

    SaveProfilesToFile();
    std::cout << "Created new profile: " << name << std::endl;
}

void MenuManager::SelectProfile(int index) {
    if (index >= 0 && index < m_profiles.size()) {
        m_currentProfile = &m_profiles[index];
        std::cout << "Selected profile: " << m_currentProfile->name << std::endl;
    }
}

void MenuManager::DeleteProfile(int index) {
    if (index >= 0 && index < m_profiles.size()) {
        std::string profileName = m_profiles[index].name;

        if (m_currentProfile == &m_profiles[index]) {
            m_currentProfile = nullptr;
        }

        m_profiles.erase(m_profiles.begin() + index);
        SaveProfilesToFile();

        std::cout << "Deleted profile: " << profileName << std::endl;
        ShowWarningMessage("Profile deleted: " + profileName);
        SetMenuState(MenuState::ChooseProfile);
    }
}

void MenuManager::HandleTextInput(sf::Uint32 unicode) {
    if (unicode == 8) {
        if (!m_inputBuffer.empty()) {
            m_inputBuffer.pop_back();
        }
    }
    else if (unicode >= 32 && unicode <= 126) {
        if (m_inputBuffer.length() < 20) {
            m_inputBuffer += static_cast<char>(unicode);
        }
    }

    m_inputText.setString(m_inputBuffer);
}

void MenuManager::ClearInputText() {
    m_inputBuffer.clear();
    m_inputText.setString("");
}

void MenuManager::ShowWarningMessage(const std::string& message) {
    m_warningText.setString(message);
    sf::FloatRect textBounds = m_warningText.getLocalBounds();
    m_warningText.setPosition((m_windowSize.x - textBounds.width) / 2, m_windowSize.y - 150);
    m_showWarning = true;
    m_warningTimer = 3.0f;
}

void MenuManager::ReturnToMenu() {
    SetMenuState(MenuState::PlayMenu);
}

bool MenuManager::IsGamePaused() const {
    return m_gamePaused;
}

void MenuManager::RecreateCurrentMenuUI() {
    // Lưu lại trạng thái hiện tại
    MenuState currentState = m_currentState;
    SettingsTab currentTab = m_currentSettingsTab;
    bool wasDropdownOpen = m_resolutionDropdownOpen;

    // Đóng dropdown nếu đang mở
    m_resolutionDropdownOpen = false;

    // Xóa tất cả UI elements
    m_buttons.clear();
    m_sliders.clear();
    m_tabButtons.clear();
    m_resolutionDropdownButtons.clear();

    // Clear title texts to prevent ghost text
    m_titleText.setString("");
    m_titleShadow.setString("");

    // Tạo lại UI dựa trên menu hiện tại
    switch (currentState) {
    case MenuState::ProfileMenu:
        CreateProfileMenu();
        break;
    case MenuState::CreateProfile:
        CreateNewProfileMenu();
        break;
    case MenuState::ChooseProfile:
        CreateChooseProfileMenu();
        break;
    case MenuState::MainMenu:
        CreateMainMenu();
        break;
    case MenuState::PlayMenu:
        CreatePlayMenu();
        break;
    case MenuState::Settings:
        m_currentSettingsTab = currentTab;
        CreateSettingsMenu();
        break;
    case MenuState::GamePlay:
        if (m_gamePaused) {
            CreatePauseMenu();
        }
        else {
            m_titleText.setString("");
            m_titleShadow.setString("");
        }
        break;
    default:
        break;
    }

    if (m_titleText.getString() != "") {
        sf::FloatRect titleBounds = m_titleText.getLocalBounds();
        m_titleText.setPosition((m_windowSize.x - titleBounds.width) / 2,
            m_titleText.getPosition().y);

        m_titleShadow.setPosition(
            m_titleText.getPosition().x + 3,
            m_titleText.getPosition().y + 3
        );
    }
}


void MenuManager::ApplyResolution(sf::RenderWindow& window) {
    std::cout << "Attempting to apply resolution: " << m_settings.resolution.x << "x" << m_settings.resolution.y
        << " (Fullscreen: " << m_settings.fullscreen << ")" << std::endl;

    // Validate resolution
    sf::VideoMode newMode(m_settings.resolution.x, m_settings.resolution.y);
    if (!newMode.isValid()) {
        std::cerr << "Invalid resolution: " << m_settings.resolution.x << "x" << m_settings.resolution.y << std::endl;
        ShowWarningMessage("Invalid resolution! Reverting to default.");
        m_settings.resolution = m_availableResolutions[0];
        m_selectedResolutionIndex = 0;
        newMode = sf::VideoMode(m_settings.resolution.x, m_settings.resolution.y);
    }

    // Recreate the window
    window.create(newMode, "SFML window", m_settings.fullscreen ? sf::Style::Fullscreen : sf::Style::Default);
    window.setVerticalSyncEnabled(true);

    // Cập nhật window size
    m_windowSize = sf::Vector2f(window.getSize());
    std::cout << "Window recreated with size: " << m_windowSize.x << "x" << m_windowSize.y << std::endl;

    RecreateCurrentMenuUI();


    if (m_resolutionChangeCallback) {
        m_resolutionChangeCallback(m_settings.resolution);
    }
}

void MenuManager::SaveSettingsToFile() {
    try {
        json root;
        root["musicVolume"] = m_settings.musicVolume;
        root["sfxVolume"] = m_settings.sfxVolume;
        root["resolution"]["width"] = m_settings.resolution.x;
        root["resolution"]["height"] = m_settings.resolution.y;
        root["fullscreen"] = m_settings.fullscreen;

        std::ofstream file(SETTINGS_FILE_PATH);
        if (file.is_open()) {
            file << root.dump(4);
            file.close();
            std::cout << "Settings saved to " << SETTINGS_FILE_PATH << std::endl;
        }
        else {
            std::cerr << "Could not open settings file for writing: " << SETTINGS_FILE_PATH << std::endl;
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON error while saving settings: " << e.what() << std::endl;
    }
}

void MenuManager::LoadSettingsFromFile() {
    std::ifstream file(SETTINGS_FILE_PATH);
    if (!file.is_open()) {
        std::cout << "No existing settings file found. Using default settings." << std::endl;
        return;
    }

    try {
        json root;
        file >> root;

        m_settings.musicVolume = root.value("musicVolume", 50.0f);
        m_settings.sfxVolume = root.value("sfxVolume", 50.0f);
        if (root.contains("resolution") && root["resolution"].is_object()) {
            m_settings.resolution.x = root["resolution"].value("width", 1920u);
            m_settings.resolution.y = root["resolution"].value("height", 1080u);
        }
        m_settings.fullscreen = root.value("fullscreen", false);

        SoundManager::getInstance().SetMusicVolume(m_settings.musicVolume);
        SoundManager::getInstance().SetSoundVolume(m_settings.sfxVolume);

        std::cout << "Loaded settings from " << SETTINGS_FILE_PATH << std::endl;
    }
    catch (const json::exception& e) {
        std::cerr << "JSON error while loading settings: " << e.what() << std::endl;
    }

    file.close();
}

void MenuManager::SaveProfilesToFile() {
    try {
        json root;
        json profilesArray = json::array();

        for (const auto& profile : m_profiles) {
            json profileJson;
            profileJson["name"] = profile.name;
            profileJson["level"] = profile.level;
            profileJson["experience"] = profile.experience;
            profileJson["highScore"] = profile.highScore;
            profileJson["savedLevel"] = profile.savedLevel;
            profileJson["savedDifficulty"] = profile.savedDifficulty;
            profileJson["savedGold"] = profile.savedGold;

            json towersArray = json::array();
            for (const auto& tower : profile.savedTowers) {
                json towerJson;
                towerJson["x"] = tower.x;
                towerJson["y"] = tower.y;
                towerJson["type"] = tower.type;
                towerJson["level"] = tower.level;
                towersArray.push_back(towerJson);
            }
            profileJson["savedTowers"] = towersArray;

            json mapArray = json::array();
            for (const auto& row : profile.savedMapLayout) {
                json rowArray = json::array();
                for (int cell : row) {
                    rowArray.push_back(cell);
                }
                mapArray.push_back(rowArray);
            }
            profileJson["savedMapLayout"] = mapArray;

            json pathArray = json::array();
            for (const auto& point : profile.savedEnemyPath) {
                json pointJson;
                pointJson["x"] = point.x;
                pointJson["y"] = point.y;
                pathArray.push_back(pointJson);
            }
            profileJson["savedEnemyPath"] = pathArray;

            profilesArray.push_back(profileJson);
        }

        root["profiles"] = profilesArray;

        std::ofstream file(PROFILES_FILE_PATH);
        if (file.is_open()) {
            file << root.dump(4);
            file.close();
            std::cout << "Profiles saved to " << PROFILES_FILE_PATH << std::endl;
        }
        else {
            std::cerr << "Could not open file for writing: " << PROFILES_FILE_PATH << std::endl;
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON error while saving profiles: " << e.what() << std::endl;
    }
}

void MenuManager::LoadProfilesFromFile() {
    std::ifstream file(PROFILES_FILE_PATH);
    if (!file.is_open()) {
        std::cout << "No existing profiles file found. Starting with empty profile list." << std::endl;
        return;
    }

    try {
        json root;
        file >> root;

        m_profiles.clear();

        if (root.contains("profiles") && root["profiles"].is_array()) {
            for (const auto& profileJson : root["profiles"]) {
                PlayerProfile profile;
                profile.name = profileJson.value("name", "");
                profile.level = profileJson.value("level", 1);
                profile.experience = profileJson.value("experience", 0);
                profile.highScore = profileJson.value("highScore", 0);
                profile.savedLevel = profileJson.value("savedLevel", 1);
                profile.savedDifficulty = profileJson.value("savedDifficulty", 1.0f);
                profile.savedGold = profileJson.value("savedGold", 10);

                if (profileJson.contains("savedTowers") && profileJson["savedTowers"].is_array()) {
                    for (const auto& towerJson : profileJson["savedTowers"]) {
                        TowerData tower;
                        tower.x = towerJson.value("x", 0);
                        tower.y = towerJson.value("y", 0);
                        tower.type = towerJson.value("type", 0);
                        tower.level = towerJson.value("level", 1);
                        profile.savedTowers.push_back(tower);
                    }
                }

                if (profileJson.contains("savedMapLayout") && profileJson["savedMapLayout"].is_array()) {
                    for (const auto& rowJson : profileJson["savedMapLayout"]) {
                        if (rowJson.is_array()) {
                            std::vector<int> row;
                            for (const auto& cellJson : rowJson) {
                                row.push_back(cellJson.get<int>());
                            }
                            profile.savedMapLayout.push_back(row);
                        }
                    }
                }

                if (profileJson.contains("savedEnemyPath") && profileJson["savedEnemyPath"].is_array()) {
                    for (const auto& pointJson : profileJson["savedEnemyPath"]) {
                        PathPoint point;
                        point.x = pointJson.value("x", 0);
                        point.y = pointJson.value("y", 0);
                        profile.savedEnemyPath.push_back(point);
                    }
                }

                if (!profile.name.empty()) {
                    m_profiles.push_back(profile);
                }
            }
            std::cout << "Loaded " << m_profiles.size() << " profiles from " << PROFILES_FILE_PATH << std::endl;
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON error while loading profiles: " << e.what() << std::endl;
    }

    file.close();
}

void MenuManager::SetStartGameCallback(std::function<void(int)> callback) {
    m_startGameCallback = callback;
}