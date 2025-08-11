#ifndef TOWERSELECTIONPANEL_H
#define TOWERSELECTIONPANEL_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "MenuManager.h"

class TowerSelectionPanel {
public:
    TowerSelectionPanel();
    ~TowerSelectionPanel();

    // Initialize the panel
    bool Initialize(sf::RenderWindow& window);

    //Set start game
    void SetCurrentMap(int map);
    void UpdateInterfaceForMap(int mapNumber);

    //Get window sizez
    void SetWindowSize(const sf::Vector2u& windowSize);

    // Update panel (for hover effects and interactions)
    void Update(float deltaTime, const sf::Vector2f& mousePos, int playerGold, const std::vector<int>& towerCosts);

    // Draw the panel
    void Draw(sf::RenderWindow& window) const;

    // Check if a tower slot is clicked
    int GetClickedTowerIndex(const sf::Vector2f& mousePos) const;

    // Get tower cost by index
    int GetTowerCost(int index) const;

    // Set visibility
    void SetVisible(bool visible) { m_IsVisible = visible; }
    bool IsVisible() const { return m_IsVisible; }

private:
    struct TowerSlot {
        sf::RectangleShape background;
        sf::RectangleShape border;
        sf::Sprite towerSprite;
        sf::Text label;
        sf::Text costText;
        sf::Sprite goldIcon;
        bool isHovered;
        int cost;
    };

    //window size
    sf::Vector2f m_windowSize;
    
    //Start game
    int m_currentMap;

    // Helper functions
    void InitializeTextures();
    void InitializeSlots(sf::RenderWindow& window, const std::vector<int>& towerCosts);
    void UpdateHoverEffects(float deltaTime, const sf::Vector2f& mousePos, int playerGold);
    void UpdateVisibility(float deltaTime, const sf::Vector2f& mousePos);
    void UpdatePositions();
    bool IsMouseOverTrigger(const sf::Vector2f& mousePos) const;

    // UI Elements - Always visible trigger tab
    sf::RectangleShape m_TriggerTab;        // Tab chứa "Buy Tower" và "<<"
    sf::Text m_TitleText;                   // "Buy Tower" - luôn hiển thị
    sf::Text m_TriggerArrow;                // Dấu "<<" 
    sf::Font m_Font;

    // Panel elements - slide in/out
    sf::RectangleShape m_PanelBackground;
    sf::RectangleShape m_PanelBorder;

    // Tower slots
    std::vector<TowerSlot> m_TowerSlots;
    std::vector<sf::Texture> m_TowerTextures;

    // Panel properties
    sf::Vector2f m_TriggerTabPosition;      // Vị trí tab cố định
    sf::Vector2f m_TriggerTabSize;          // Kích thước tab
    sf::Vector2f m_PanelPosition;           // Vị trí panel có thể slide
    sf::Vector2f m_PanelSize;
    sf::Vector2f m_WindowSize;
    float m_SlotSpacing;

    // Animation properties
    float m_HoverScale;
    float m_HoverAnimationSpeed;

    // Visibility and animation
    bool m_IsVisible;                       // Panel có hiển thị không
    float m_SlideOffset;                    // Độ lệch slide
    float m_SlideSpeed;
    float m_HideTimer;
    float m_HideDelay;
    bool m_TriggerHovered;                  // Trigger tab có được hover không

    float m_towerWidht;
    float m_towerHeight;

};

#endif // TOWERSELECTIONPANEL_H