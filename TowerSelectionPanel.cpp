#include "TowerSelectionPanel.h"
#include <stdexcept>

TowerSelectionPanel::TowerSelectionPanel()
    : m_TriggerTabPosition(0.0f, 0.0f)
    , m_TriggerTabSize(160.0f, 50.0f)       // Tab nhỏ chỉ chứa title và arrow
    , m_PanelPosition(50.0f, 100.0f)
    , m_PanelSize(280.0f, 400.0f)           // Panel chứa towers
    , m_SlotSpacing(15.0f)
    , m_HoverScale(1.1f)
    , m_HoverAnimationSpeed(5.0f)
    , m_IsVisible(false)                    // Panel bắt đầu ẩn
    , m_SlideOffset(280.0f)                 // Bắt đầu ở ngoài màn hình
    , m_SlideSpeed(12.0f)                   // Tốc độ slide nhanh hơn
    , m_HideTimer(0.0f)
    , m_HideDelay(0.5f)                     // 2 giây sau khi không hover
    , m_TriggerHovered(false)
    , m_WindowSize(0.0f, 0.0f)
{
}

TowerSelectionPanel::~TowerSelectionPanel() {
}

bool TowerSelectionPanel::Initialize(sf::RenderWindow& window) {
    // Load font
    if (!m_Font.loadFromFile("Fonts/MedievalSharp-Regular.ttf")) {
        throw std::runtime_error("Failed to load font from 'Fonts/MedievalSharp-Regular.ttf'");
    }

    // Initialize textures
    InitializeTextures();

    m_windowSize = sf::Vector2f(window.getSize());

    // Initialize tower slots
    std::vector<int> defaultCosts = { 5, 10, 15, 20 };
    InitializeSlots(window, defaultCosts);

    return true;
}

void TowerSelectionPanel::UpdateInterfaceForMap(int mapNumber) {
    m_currentMap = mapNumber;

    // Set window size

    m_towerWidht = 280.0f;
    m_towerHeight = 450.0f;

    // Setup trigger tab size to match tower slots (85px height like tower slots)
    m_TriggerTabSize = sf::Vector2f(m_towerWidht, 70.0f); // Same width as panel minus spacing, same height as tower slots

    // Switch case để điều chỉnh giao diện theo map
    switch (mapNumber) {
    case 1:
        // Map 1 - Position ở góc phải giữa
        m_TriggerTabPosition.x = m_WindowSize.x - m_TriggerTabSize.x - 15.0f;
        m_TriggerTabPosition.y = m_WindowSize.y / 3.0f;
        break;

    case 2:
        // Map 2 - Position ở góc phải trên
        m_TriggerTabPosition.x = m_WindowSize.x - m_TriggerTabSize.x;
        m_TriggerTabPosition.y = 50.f;
        break;

    case 3:
        // Map 3 - Position ở dưới
        m_TriggerTabPosition.x = m_WindowSize.x - m_TriggerTabSize.x;
        m_TriggerTabPosition.y = m_WindowSize.y / 2.8f;
        break;

    case 4:
        // Map 4 - Position ở góc phải trên
        m_TriggerTabPosition.x = m_WindowSize.x - m_TriggerTabSize.x - 15.0f;
        m_TriggerTabPosition.y = 50.f;
        break;

    default:
        // Default position
        m_TriggerTabPosition.x = m_WindowSize.x - m_TriggerTabSize.x - 15.0f;
        m_TriggerTabPosition.y = m_WindowSize.y / 3.5f;
        break;
    }


    // Setup trigger tab position (always visible, right edge)


    // Setup trigger tab background
    m_TriggerTab.setSize(m_TriggerTabSize);
    m_TriggerTab.setFillColor(sf::Color(60, 45, 30, 240));
    m_TriggerTab.setPosition(m_TriggerTabPosition);

    // Add border to trigger tab
    m_TriggerTab.setOutlineColor(sf::Color(180, 140, 100));
    m_TriggerTab.setOutlineThickness(2.0f);

    // Setup title text (centered in trigger tab)
    m_TitleText.setFont(m_Font);
    m_TitleText.setString("Buy Tower");
    m_TitleText.setCharacterSize(18);
    m_TitleText.setFillColor(sf::Color(255, 220, 150));
    m_TitleText.setStyle(sf::Text::Bold);

    // Center title in trigger tab
    sf::FloatRect titleBounds = m_TitleText.getLocalBounds();
    m_TitleText.setPosition(
        m_TriggerTabPosition.x + (m_TriggerTabSize.x - titleBounds.width) / 2,
        m_TriggerTabPosition.y + 15.0f
    );

    // Setup trigger arrow (centered below title)
    m_TriggerArrow.setFont(m_Font);
    m_TriggerArrow.setString("<<");
    m_TriggerArrow.setCharacterSize(20);
    m_TriggerArrow.setFillColor(sf::Color(255, 215, 0)); // Gold color
    m_TriggerArrow.setStyle(sf::Text::Bold);

    sf::FloatRect arrowBounds = m_TriggerArrow.getLocalBounds();
    m_TriggerArrow.setPosition(
        m_TriggerTabPosition.x + (m_TriggerTabSize.x - arrowBounds.width) / 2,
        m_TriggerTabPosition.y + 45.0f
    );

    // Setup panel position - will slide to replace trigger tab position
    m_PanelPosition.x = m_TriggerTabPosition.x; // Same X as trigger tab
    m_PanelPosition.y = m_TriggerTabPosition.y + m_TriggerTabSize.y + 5.0f; // Below trigger tab

    // Set up panel background
    m_PanelBackground.setSize(m_PanelSize);
    m_PanelBackground.setFillColor(sf::Color(40, 30, 20, 230));
    m_PanelBackground.setPosition(m_PanelPosition);

    // Set up panel border
    m_PanelBorder.setSize(sf::Vector2f(m_PanelSize.x + 4.0f, m_PanelSize.y + 4.0f));
    m_PanelBorder.setPosition(sf::Vector2f(m_PanelPosition.x - 2.0f, m_PanelPosition.y - 2.0f));
    m_PanelBorder.setFillColor(sf::Color::Transparent);
    m_PanelBorder.setOutlineColor(sf::Color(180, 140, 100));
    m_PanelBorder.setOutlineThickness(3.0f);

    // Update all positions
    UpdatePositions();
}

void TowerSelectionPanel::SetWindowSize(const sf::Vector2u& windowSize) {
    m_WindowSize = static_cast<sf::Vector2f>(windowSize);

    // Update trigger tab position (always at right edge with margin)
    m_TriggerTabPosition.x = windowSize.x - m_TriggerTabSize.x - 15.0f;
    m_TriggerTab.setPosition(m_TriggerTabPosition);

    // Update title and arrow positions (centered in trigger tab)
    sf::FloatRect titleBounds = m_TitleText.getLocalBounds();
    m_TitleText.setPosition(
        m_TriggerTabPosition.x + (m_TriggerTabSize.x - titleBounds.width) / 2,
        m_TriggerTabPosition.y + 15.0f
    );

    sf::FloatRect arrowBounds = m_TriggerArrow.getLocalBounds();
    m_TriggerArrow.setPosition(
        m_TriggerTabPosition.x + (m_TriggerTabSize.x - arrowBounds.width) / 2,
        m_TriggerTabPosition.y + 45.0f
    );

    // Update panel base position
    m_PanelPosition.x = m_TriggerTabPosition.x;
    m_PanelPosition.y = m_TriggerTabPosition.y + m_TriggerTabSize.y + 5.0f;

    // Update positions
    UpdatePositions();
}


void TowerSelectionPanel::InitializeTextures() {

    // Load tower textures
    std::vector<std::string> towerTexturePaths = {
        "image/sprite/Tower1.png",
        "image/sprite/Tower2.png",
        "image/sprite/Tower3.png",
        "image/sprite/Tower4.png"
    };

    for (const auto& path : towerTexturePaths) {
        sf::Texture texture;
        if (!texture.loadFromFile(path)) {
            throw std::runtime_error("Failed to load tower texture: " + path);
        }
        m_TowerTextures.push_back(texture);
    }
}

void TowerSelectionPanel::InitializeSlots(sf::RenderWindow& window, const std::vector<int>& towerCosts) {
    m_TowerSlots.clear();
    std::vector<std::string> labels = { "Tower 1", "Tower 2", "Tower 3", "Tower 4" };

    float slotWidth = m_PanelSize.x - m_SlotSpacing * 2;
    float slotHeight = 85.0f;
    float startY = m_PanelPosition.y + 15.0f; // Start from top of panel (no title inside)

    for (size_t i = 0; i < 4; ++i) {
        TowerSlot slot;
        slot.cost = (i < towerCosts.size()) ? towerCosts[i] : 0;
        slot.isHovered = false;

        float slotY = startY + i * (slotHeight + m_SlotSpacing);

        // Slot background
        slot.background.setSize(sf::Vector2f(slotWidth, slotHeight));
        slot.background.setFillColor(sf::Color(60, 45, 30, 220));
        slot.background.setPosition(m_PanelPosition.x + m_SlotSpacing, slotY);

        // Slot border
        slot.border.setSize(sf::Vector2f(slotWidth + 2.0f, slotHeight + 2.0f));
        slot.border.setPosition(
            m_PanelPosition.x + m_SlotSpacing - 1.0f,
            slotY - 1.0f
        );
        slot.border.setFillColor(sf::Color::Transparent);
        slot.border.setOutlineColor(sf::Color(120, 100, 80));
        slot.border.setOutlineThickness(2.0f);

        // Tower sprite
        slot.towerSprite.setTexture(m_TowerTextures[i]);
        slot.towerSprite.setScale(1.0f, 1.0f);
        sf::FloatRect spriteBounds = slot.towerSprite.getLocalBounds();
        slot.towerSprite.setOrigin(spriteBounds.width / 2, spriteBounds.height / 2);
        slot.towerSprite.setPosition(
            slot.background.getPosition().x + 50.0f,
            slot.background.getPosition().y + slotHeight / 2
        );

        // Label
        slot.label.setFont(m_Font);
        slot.label.setString(labels[i]);
        slot.label.setCharacterSize(18);
        slot.label.setFillColor(sf::Color(255, 240, 200));
        slot.label.setStyle(sf::Text::Bold);
        slot.label.setPosition(
            slot.background.getPosition().x + 110.0f,
            slot.background.getPosition().y + 15.0f
        );

        // Cost text
        slot.costText.setFont(m_Font);
        slot.costText.setString("Gold: " + std::to_string(slot.cost));
        slot.costText.setCharacterSize(14);
        slot.costText.setFillColor(sf::Color(255, 215, 0));
        slot.costText.setStyle(sf::Text::Bold);
        slot.costText.setPosition(
            slot.background.getPosition().x + 110.0f,
            slot.background.getPosition().y + slotHeight - 35.0f
        );

        m_TowerSlots.push_back(slot);
    }
}

void TowerSelectionPanel::Update(float deltaTime, const sf::Vector2f& mousePos, int playerGold, const std::vector<int>& towerCosts) {
    for (size_t i = 0; i < m_TowerSlots.size() && i < towerCosts.size(); ++i) {
        if (m_TowerSlots[i].cost != towerCosts[i]) {
            m_TowerSlots[i].cost = towerCosts[i];
            m_TowerSlots[i].costText.setString("Gold " + std::to_string(towerCosts[i]));
        }
    }

    // Update trigger hover effect
    bool wasHovered = m_TriggerHovered;
    m_TriggerHovered = IsMouseOverTrigger(mousePos);

    if (m_TriggerHovered != wasHovered) {
        if (m_TriggerHovered) {
            // Hover effect for trigger
            m_TriggerTab.setFillColor(sf::Color(80, 60, 40, 250));
            m_TriggerArrow.setFillColor(sf::Color(255, 255, 100)); // Brighter gold
        }
        else {
            // Normal trigger appearance
            m_TriggerTab.setFillColor(sf::Color(60, 45, 30, 240));
            m_TriggerArrow.setFillColor(sf::Color(255, 215, 0));
        }
    }

    UpdateVisibility(deltaTime, mousePos);
    UpdateHoverEffects(deltaTime, mousePos, playerGold);
}

void TowerSelectionPanel::UpdateVisibility(float deltaTime, const sf::Vector2f& mousePos) {
    bool shouldBeVisible = IsMouseOverTrigger(mousePos) ||
        (m_IsVisible && m_SlideOffset < m_PanelSize.x * 0.8f && m_PanelBackground.getGlobalBounds().contains(mousePos));

    if (shouldBeVisible) {
        m_IsVisible = true;
        m_HideTimer = 0.0f;

        // Animate slide in (slide offset goes to 0 to align with trigger tab)
        if (m_SlideOffset > 0.0f) {
            m_SlideOffset -= m_SlideSpeed * deltaTime * 60.0f;
            if (m_SlideOffset < 0.0f) m_SlideOffset = 0.0f;
        }
    }
    else {
        // Start hide timer
        m_HideTimer += deltaTime;

        if (m_HideTimer >= m_HideDelay) {
            // Animate slide out to right
            if (m_SlideOffset < m_PanelSize.x) {
                m_SlideOffset += m_SlideSpeed * deltaTime * 60.0f;
                if (m_SlideOffset >= m_PanelSize.x) {
                    m_SlideOffset = m_PanelSize.x;
                    m_IsVisible = false;
                }
            }
        }
    }

    UpdatePositions();
}



void TowerSelectionPanel::UpdatePositions() {
    float offsetX = m_SlideOffset;

    // Panel slides to the right to hide, but stays aligned with trigger tab when visible
    sf::Vector2f currentPanelPos = sf::Vector2f(
        m_PanelPosition.x + offsetX, // Slides from trigger tab position
        m_PanelPosition.y
    );

    m_PanelBackground.setPosition(currentPanelPos);
    m_PanelBorder.setPosition(sf::Vector2f(currentPanelPos.x - 2.0f, currentPanelPos.y - 2.0f));

    // Update slot positions
    float slotHeight = 85.0f;
    float startY = currentPanelPos.y + 15.0f; // Start from panel top

    for (size_t i = 0; i < m_TowerSlots.size(); ++i) {
        float slotY = startY + i * (slotHeight + m_SlotSpacing);

        m_TowerSlots[i].background.setPosition(currentPanelPos.x + m_SlotSpacing, slotY);
        m_TowerSlots[i].border.setPosition(currentPanelPos.x + m_SlotSpacing - 1.0f, slotY - 1.0f);

        m_TowerSlots[i].towerSprite.setPosition(
            currentPanelPos.x + m_SlotSpacing + 50.0f,
            slotY + slotHeight / 2
        );

        m_TowerSlots[i].label.setPosition(
            currentPanelPos.x + m_SlotSpacing + 110.0f,
            slotY + 15.0f
        );

        m_TowerSlots[i].costText.setPosition(
            currentPanelPos.x + m_SlotSpacing + 110.0f,
            slotY + slotHeight - 35.0f
        );
    }
}

bool TowerSelectionPanel::IsMouseOverTrigger(const sf::Vector2f& mousePos) const {
    return m_TriggerTab.getGlobalBounds().contains(mousePos);
}


void TowerSelectionPanel::UpdateHoverEffects(float deltaTime, const sf::Vector2f& mousePos, int playerGold) {
    if (!m_IsVisible || m_SlideOffset > 50.0f) return; // Don't update hover

    for (auto& slot : m_TowerSlots) {
        bool isMouseOver = slot.background.getGlobalBounds().contains(mousePos);
        bool canAfford = playerGold >= slot.cost;

        if (isMouseOver && canAfford) {
            if (!slot.isHovered) {
                slot.isHovered = true;
                slot.background.setFillColor(sf::Color(100, 70, 30, 200)); // Slightly lighter wood
                slot.border.setOutlineColor(sf::Color(200, 200, 200)); // Glow effect
            }
            // Animate scale up
            float currentScale = slot.towerSprite.getScale().x;
            float targetScale = m_HoverScale * 0.6f;
            if (currentScale < targetScale) {
                float newScale = std::min(targetScale, currentScale + m_HoverAnimationSpeed * deltaTime);
                slot.towerSprite.setScale(newScale, newScale);
            }
        }
        else {
            if (slot.isHovered) {
                slot.isHovered = false;
                slot.background.setFillColor(sf::Color(80, 50, 20, 200)); // Original wood color
                slot.border.setOutlineColor(sf::Color(150, 150, 150)); // Original border
            }
            // Animate scale down
            float currentScale = slot.towerSprite.getScale().x;
            float targetScale = 0.6f;
            if (currentScale > targetScale) {
                float newScale = std::max(targetScale, currentScale - m_HoverAnimationSpeed * deltaTime);
                slot.towerSprite.setScale(newScale, newScale);
            }
            // Dim if can't afford
            if (!canAfford) {
                slot.towerSprite.setColor(sf::Color(150, 150, 150, 150));
                slot.costText.setFillColor(sf::Color(255, 100, 100)); // Light red cost text
                slot.label.setFillColor(sf::Color(200, 200, 200)); // Dim label
            }
            else {
                slot.towerSprite.setColor(sf::Color::White);
                slot.costText.setFillColor(sf::Color::Yellow);
                slot.label.setFillColor(sf::Color::White);
            }
        }
    }
}

void TowerSelectionPanel::Draw(sf::RenderWindow& window) const {

    window.draw(m_TriggerTab);
    window.draw(m_TitleText);
    window.draw(m_TriggerArrow);

    if (m_IsVisible && m_SlideOffset < m_PanelSize.x) {
        window.draw(m_PanelBackground);
        window.draw(m_PanelBorder);

        // Draw tower slots
        for (const auto& slot : m_TowerSlots) {
            window.draw(slot.background);
            window.draw(slot.border);
            window.draw(slot.towerSprite);
            window.draw(slot.label);
            window.draw(slot.costText);
        }
    }
}

int TowerSelectionPanel::GetClickedTowerIndex(const sf::Vector2f& mousePos) const {
    // Don't allow clicking if panel is mostly hidden
    if (!m_IsVisible || m_SlideOffset > 50.0f) return -1;

    for (size_t i = 0; i < m_TowerSlots.size(); ++i) {
        if (m_TowerSlots[i].background.getGlobalBounds().contains(mousePos)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int TowerSelectionPanel::GetTowerCost(int index) const {
    if (index >= 0 && index < static_cast<int>(m_TowerSlots.size())) {
        return m_TowerSlots[index].cost;
    }
    return 0;
}

void TowerSelectionPanel::SetCurrentMap(int map) {
    m_currentMap = map;
}
