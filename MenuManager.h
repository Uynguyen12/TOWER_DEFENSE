#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> // Added for sound effects
#include <vector>
#include <string>
#include "Entity.h"
#include <functional>
#include "UIManager.h"

class MenuManager {
public:
    enum class MenuState {
        ProfileMenu,
        CreateProfile,
        ChooseProfile,
        MainMenu,
        PlayMenu,
        GamePlay,
        DifficultyMenu,
        PauseMenu,
        GameOver,
        GameWon,
        Settings,
        MusicSettings,
        ConfigSettings,
    };

    enum class ButtonState {
        Normal,
        Hovered,
        Pressed
    };

    enum class Difficulty {
        Easy,
        Medium,
        Hard,
        Extremely
    };

    enum class SettingsTab {
        Music,
        Configuration
    };

    struct TabButton {
        sf::Text text;
        sf::RectangleShape underline;
        bool isActive;
        std::function<void()> callback;

        TabButton() : isActive(false) {}
    };

    enum class ButtonStyle {
        Shield,      // Khiên
        Scroll,      // Cuộn giấy
        WoodPlank,   // Bảng gỗ
        Stone        // Đá cổ
    };

    // Button icon enum
    enum class ButtonIcon {
        None,
        Shield,      // 🏰 New Profile
        Sword,       // 🛡️ Play as Guest
        Gear,        // ⚙️ Settings
        Scroll,      // 📜 Back/Exit
        Crown,       // 👑 Main Menu
        Target,      // 🎯 Play / Continue
        Restart      // 🕰️ Restart
    };

    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        sf::Text textShadow;
        std::function<void(sf::RenderWindow&)> callback;
        ButtonState state;
        ButtonStyle style;
        ButtonIcon icon;
        sf::CircleShape decorativeGem;
        std::vector<sf::CircleShape> rivets;
        sf::Sprite iconSprite; // Added for flat medieval icon
        bool isVisible;
        bool isDeleteButton;
        float iconGlowIntensity; // For hover glow effect

        Button() : state(ButtonState::Normal), isVisible(true), isDeleteButton(false),
            style(ButtonStyle::Shield), icon(ButtonIcon::None), iconGlowIntensity(0.0f) {
        }
    };

    struct Slider {
        sf::RectangleShape track;
        sf::RectangleShape handle;
        sf::Text label;
        float value; // 0.0f to 100.0f
        float minValue;
        float maxValue;
        bool isDragging;
        bool isVisible;
        std::function<void(float)> callback;

        Slider() : value(50.0f), minValue(0.0f), maxValue(100.0f), isDragging(false), isVisible(true) {}
    };

    // Thêm struct cho cài đặt
    struct GameSettings {
        float musicVolume = 50.0f;
        float sfxVolume = 50.0f;
        sf::Vector2u resolution = { 1920, 1080 };
        bool fullscreen = false;
    };

    struct TowerData {
        int x, y;
        int type;
        int level;
    };

    struct PathPoint {
        int x, y;
    };

    struct PlayerProfile {
        std::string name;
        int map = 1;
        Difficulty difficulty = Difficulty::Easy;
        bool won = false;
        int savedLevel = 1;
        int savedGold = 10;
        int currentLevel = 1;

        // Game state data for each map and difficulty combination
        struct GameStateData {
            std::vector<TowerData> savedTowers;
            std::vector<std::vector<int>> savedMapLayout;
            std::vector<PathPoint> savedEnemyPath;
            int playerHealth = 100;
            float timeInPlayMode = 0.0f;
            std::vector<int> towerCounts = { 0, 0, 0, 0 };
            std::vector<int> spawnedEnemies = { 0, 0, 0, 0 };
            std::vector<int> killedEnemies = { 0, 0, 0, 0 };
            int savedGoldForState = 10;
        };

        // Save states for each map (1-4) and each difficulty (0-3)
        GameStateData gameStates[4][4]; // [mapIndex][difficultyIndex]

        struct CompletionData {
            bool easy = false;
            bool medium = false;
            bool hard = false;
            bool extremely = false;
        };
        std::vector<CompletionData> mapCompletions = std::vector<CompletionData>(4);

        PlayerProfile() = default;
        PlayerProfile(const std::string& playerName) : name(playerName) {
            mapCompletions.resize(4);
            // Initialize all game states
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    gameStates[i][j] = GameStateData();
                }
            }
        }

        // Get game state for specific map and difficulty
        GameStateData& GetGameState(int mapNumber, Difficulty diff) {
            int mapIndex = mapNumber - 1; // Convert to 0-based index
            int diffIndex = static_cast<int>(diff);
            return gameStates[mapIndex][diffIndex];
        }

        const GameStateData& GetGameState(int mapNumber, Difficulty diff) const {
            int mapIndex = mapNumber - 1;
            int diffIndex = static_cast<int>(diff);
            return gameStates[mapIndex][diffIndex];
        }

        // Check if there's saved data for specific map and difficulty
        bool HasSavedData(int mapNumber, Difficulty diff) const {
            const GameStateData& state = GetGameState(mapNumber, diff);
            return (state.timeInPlayMode > 0.0f || !state.savedTowers.empty());
        }

        bool HasCompletedMap(int mapNumber, Difficulty diff) const {
            if (mapNumber < 1 || mapNumber > 4) return false;
            const CompletionData& completion = mapCompletions[mapNumber - 1];
            switch (diff) {
            case Difficulty::Easy: return completion.easy;
            case Difficulty::Medium: return completion.medium;
            case Difficulty::Hard: return completion.hard;
            case Difficulty::Extremely: return completion.extremely;
            default: return false;
            }
        }

        void SetMapCompleted(int mapNumber, Difficulty diff) {
            if (mapNumber < 1 || mapNumber > 4) return;
            CompletionData& completion = mapCompletions[mapNumber - 1];
            switch (diff) {
            case Difficulty::Easy: completion.easy = true; break;
            case Difficulty::Medium: completion.medium = true; break;
            case Difficulty::Hard: completion.hard = true; break;
            case Difficulty::Extremely: completion.extremely = true; break;
            }
            won = true;
        }

        int GetTotalCompletions() const {
            int total = 0;
            for (const auto& completion : mapCompletions) {
                if (completion.easy) total++;
                if (completion.medium) total++;
                if (completion.hard) total++;
                if (completion.extremely) total++;
            }
            return total;
        }
    };

public:
    MenuManager();
    ~MenuManager();
    
    void SetCurrentMapAndDifficulty(int map, Difficulty difficulty);

    void Initialize(sf::RenderWindow& window);
    void Update(sf::RenderWindow& window, float deltaTime);
    void Draw(sf::RenderWindow& window);
    void HandleInput(sf::Event& event, sf::RenderWindow& window);
    void ReturnToMenu();
    void DrawPauseMenu(sf::RenderWindow& window);
    void TogglePauseMenu();
    void CreatePauseMenu();

    void DrawGameOverMenu(sf::RenderWindow& window);
    void ShowGameOverMenu();
    void CreateGameOverMenu();
    
    void DrawGameWonMenu(sf::RenderWindow& window);
    void ShowGameWonMenu();
    void CreateGameWonMenu();

    bool IsGamePaused() const;
    bool IsGameOver() const;
    bool IsGameWon() const;
    
    // Menu state management
    void SetMenuState(MenuState newState);
    MenuState GetMenuState() const { return m_currentState; }
    bool IsInGamePlay() const { return m_currentState == MenuState::GamePlay; }

    // Profile management
    void CreateNewProfile(const std::string& name);
    void SelectProfile(int index);
    void DeleteProfile(int index);
    bool IsGuestProfile();
    void DeleteGuestProfile();
    const PlayerProfile* GetCurrentProfile() const { return m_currentProfile; }
    const std::vector<PlayerProfile>& GetProfiles() const { return m_profiles; }

    // Profile save/load functions
    void SaveProfilesToFile();
    void LoadProfilesFromFile();
    bool HasCompletedMap(int mapNumber, Difficulty difficulty) const;
    void SetMapCompleted(int mapNumber, Difficulty difficulty) const;
    int GetTotalCompletions() const;
    void ApplyResolution(sf::RenderWindow& window);

    // Settings management
    const GameSettings& GetSettings() const { return m_settings; }
    void SaveSettingsToFile();
    void LoadSettingsFromFile();

    // Input handling for profile creation
    void SetWaitingForInput(bool waiting);
    bool IsWaitingForInput() const { return m_waitingForNameInput; }
    void HandleTextInput(sf::Uint32 unicode);
    void ClearInputText();
    void SetExitCallback(std::function<void(sf::RenderWindow&)> callback) { m_exitCallback = callback; }
    void SetStartGameCallback(std::function<void(int, Difficulty)> callback);
    void SetResolutionChangeCallback(std::function<void(sf::Vector2u)> callback) { m_resolutionChangeCallback = callback; }

    //Save game
    void SetSaveGameCallback(std::function<void()> callback) { m_saveGameCallback = callback; }
    void SetClearGameDataCallback(std::function<void()> callback) { m_clearGameDataCallback = callback; }

    // Audio callbacks
    void SetMusicVolumeCallback(std::function<void(float)> callback) { m_musicVolumeCallback = callback; }
    void SetSFXVolumeCallback(std::function<void(float)> callback) { m_sfxVolumeCallback = callback; }
    void SetBackgroundMusicVolumeCallback(std::function<void(float)> callback) { m_backgroundMusicVolumeCallback = callback; }

private:
    // Menu creation functions
    void CreateProfileMenu();
    void CreateChooseProfileMenu();
    void CreateNewProfileMenu();
    void CreateMainMenu();
    void CreatePlayMenu();
    void CreateDifficultyMenu();
    void CreateSettingsMenu();
    void CreateSettingsContent();
    void CreateTabButton(const std::string& text, sf::Vector2f position,
        SettingsTab tab, std::function<void()> callback);
    void UpdateTabButtons(sf::RenderWindow& window);
    void DrawTabButtons(sf::RenderWindow& window);
    void SetActiveTab(SettingsTab tab);

    void CreateGradientBackground();
    void CreateEnhancedButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback, bool isDeleteButton = false);
    void CreateMedievalButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback,
        ButtonStyle style = ButtonStyle::WoodPlank,
        ButtonIcon icon = ButtonIcon::None, bool isDeleteButton = false);
    void CenterTextWithShadow(sf::Text& text, sf::Text& shadow, const sf::RectangleShape& shape);
    void DrawBackgroundParticles(sf::RenderWindow& window);
    void DrawInputCursor(sf::RenderWindow& window);
    void DrawWarningWithBackground(sf::RenderWindow& window);

    // Button management
    void CreateButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback, bool isDropdownButton = false);
    void CreateDeleteButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback);
    void CreatePauseButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback,
        ButtonStyle style = ButtonStyle::WoodPlank,
        ButtonIcon icon = ButtonIcon::None, bool isDeleteButton = false);

    void CreateGameOverButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback,
        ButtonStyle style = ButtonStyle::WoodPlank,
        ButtonIcon icon = ButtonIcon::None, bool isDeleteButton = false);
    void CreateGameWonButton(const std::string& text, sf::Vector2f position, sf::Vector2f size,
        std::function<void(sf::RenderWindow&)> callback,
        ButtonStyle style = ButtonStyle::WoodPlank,
        ButtonIcon icon = ButtonIcon::None, bool isDeleteButton = false);

    void UpdateButtons(sf::RenderWindow& window);
    void DrawButtons(sf::RenderWindow& window);
    bool IsMouseOverButton(const Button& button, sf::Vector2f mousePos);

    // Helper methods for resolution dropdown
    void UpdateResolutionScroll(float deltaTime);
    void UpdateVisibleResolutions();

    // Slider management
    void CreateSlider(const std::string& label, sf::Vector2f position, float value,
        std::function<void(float)> callback);
    void UpdateSliders(sf::RenderWindow& window);
    void DrawSliders(sf::RenderWindow& window);
    void DrawResolutionDropdown(sf::RenderWindow& window);
    void DrawScrollbar(sf::RenderWindow& window, sf::Vector2f dropdownPos,
        float dropdownWidth, float dropdownHeight, int scrollIndex, int maxScroll);
    bool IsMouseOverSlider(const Slider& slider, sf::Vector2f mousePos);
    void UpdateSliderValue(Slider& slider, sf::Vector2f mousePos);
    void HandleResolutionDropdownClick(sf::Vector2f mousePos, sf::RenderWindow& window);

    // Helper functions
    void CenterText(sf::Text& text, const sf::RectangleShape& shape);
    void LoadResources();
    void ShowWarningMessage(const std::string& message);
    void UpdateWindowSize(sf::RenderWindow& window);
    void RecreateCurrentMenuUI();

    // Medieval button drawing functions
    void DrawMedievalButton(sf::RenderWindow& window, const Button& button);
    void DrawMedievalScrollIndicators(sf::RenderWindow& window);
    void DrawButtonShadow(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size);
    void DrawButtonBase(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawShieldShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color baseColor, ButtonState state);
    void DrawScrollShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color baseColor, ButtonState state);
    void DrawWoodPlankShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color baseColor, ButtonState state);
    void DrawStoneShape(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color baseColor, ButtonState state);
    void DrawDecorativeFrame(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawCornerOrnaments(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color color);
    void DrawButtonRivets(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawTextureOverlay(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawButtonText(sf::RenderWindow& window, const Button& button);
    void DrawDecorativeGem(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);

    // Button icons
    void DrawButtonIcon(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawShieldIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawSwordIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawGearIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawScrollIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawCrownIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawTargetIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);
    void DrawIconGlow(sf::RenderWindow& window, sf::Vector2f iconPos, float iconSize, sf::Color glowColor);
    void DrawRestartIcon(sf::RenderWindow& window, sf::Vector2f pos, float size);

    // Medieval effects
    void DrawMagicalEffects(sf::RenderWindow& window, const Button& button, sf::Vector2f pos, sf::Vector2f size);
    void DrawSparkleParticles(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, sf::Color color, float time);
    void DrawMedievalCorners(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, ButtonState state);
    void DrawBeveledEdge(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, ButtonState state);
    void DrawMetalStuds(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size);
    void DrawMagicalGlow(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size);
    void DrawSparkleEffect(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size);

    // Pause menu effects
    void DrawMysticalBackground(sf::RenderWindow& window);
    void DrawEnhancedMenuShadow(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawScrollParchmentBackground(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawGothicBorderWithRunes(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawRunePatterns(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize, float time);
    void DrawAnimatedCornerDecorations(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawEnhancedCoatOfArms(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize, float decorSize, float time);
    void DrawMagicalTitle(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawEnhancedDecorativeBorder(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawEnhancedMagicalParticles(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawFloatingRunes(sf::RenderWindow& window, sf::Vector2f centerPos, sf::Vector2f backgroundSize);
    void DrawEnhancedMedievalButton(sf::RenderWindow& window, const Button& button);

private:
    MenuState m_currentState;
    MenuState m_previousState;

    // Resources
    sf::Font m_font;
    sf::Texture m_backgroundTexture;
    sf::Sprite m_backgroundSprite;
    sf::Sprite m_castleBackgroundSprite;
    sf::Texture m_parchmentTexture; // Added for scroll background
    sf::Texture m_castleBackgroundTexture; // Added for castle background
    sf::Texture m_gothicPatternTexture; // Added for gothic border patterns
    sf::Texture m_dragonHeadTexture;
    sf::Texture m_warriorTexture;
    sf::Texture m_shieldSwordTexture;
    sf::Texture m_iconShieldTexture; // Added for flat medieval icons
    sf::Texture m_iconSwordTexture;
    sf::Texture m_iconGearTexture;
    sf::Texture m_iconScrollTexture;
    sf::Texture m_iconCrownTexture;
    sf::Texture m_iconTargetTexture;
    sf::Texture m_iconRestartTexture;
   

    // Current level for difficulty selection
    Difficulty m_currentDifficulty = Difficulty::Easy;

    // UI Elements
    std::vector<Button> m_buttons;
    std::vector<Button> m_pauseButtons;
    std::vector<Button> m_gameOverButtons;
    std::vector<Button> m_gameWonButtons;
    std::vector<Slider> m_sliders;
    sf::Text m_titleText;
    sf::Text m_warningText;
    sf::Text m_inputPromptText;
    sf::Text m_inputText;
    sf::RectangleShape m_inputBox;
    sf::RectangleShape m_pauseBackground;
    sf::Vector2f ScalePosition(sf::Vector2f originalPos, sf::Vector2f oldSize, sf::Vector2f newSize);
    sf::Text m_titleShadow;
    sf::RectangleShape m_inputBoxGlow;
    sf::RectangleShape m_backgroundOverlay;
    sf::RectangleShape m_gameOverBackground;
    sf::RectangleShape m_gameWonBackground;

    // Animation elements
    sf::Clock m_pauseGlowClock;
    std::vector<sf::CircleShape> m_magicalParticles;
    std::vector<sf::CircleShape> m_smokeParticles; // Added for smoke effect
    std::vector<sf::Vector2f> m_smokeVelocities;
    sf::Clock m_smokeClock;
    float m_dragonAnimationPhase; // For dragon animation
    float m_knightSwordAnimationPhase; // For knight sword animation

    // Profile management
    std::vector<PlayerProfile> m_profiles;
    PlayerProfile* m_currentProfile;
    std::string m_inputBuffer;
    bool m_waitingForNameInput;
    bool m_showWarning;
    float m_warningTimer;
    bool m_gamePaused;
    bool m_gameOver;
    bool m_gameWon;
    int m_currentMap;
    bool m_ambientSoundsPlaying;
    bool m_gameOverSoundPlaying;
    bool m_gameWonSoundPlaying;

    float m_resolutionScrollOffset;
    float m_maxResolutionScroll;

    //Guest profile
    bool m_guestProfile;

    // Decorative corner elements
    std::vector<sf::Sprite> m_dragonSprites;
    std::vector<sf::Sprite> m_warriorSprites;
    std::vector<sf::Sprite> m_shieldSwordSprites;

    // Gothic border decorations
    std::vector<sf::RectangleShape> m_gothicBorders;
    std::vector<sf::CircleShape> m_cornerRivets;

    // Settings
    GameSettings m_settings;
    SettingsTab m_currentSettingsTab;
    std::vector<TabButton> m_tabButtons;
    static const sf::Color TAB_ACTIVE_COLOR;
    static const sf::Color TAB_INACTIVE_COLOR;
    static const sf::Color TAB_UNDERLINE_COLOR;

    // Callback functions
    std::function<void(sf::RenderWindow&)> m_exitCallback;
    std::function<void(int, Difficulty)> m_startGameCallback;
    std::function<void(sf::Vector2u)> m_resolutionChangeCallback;
    std::function<void(float)> m_musicVolumeCallback;
    std::function<void(float)> m_backgroundMusicVolumeCallback;
    std::function<void(float)> m_sfxVolumeCallback;

    //Save game
    std::function<void()> m_saveGameCallback;
    std::function<void()> m_clearGameDataCallback;

    // UI State
    sf::Vector2f m_windowSize;

    // Static constants
    static const sf::Color BUTTON_NORMAL_COLOR;
    static const sf::Color BUTTON_HOVER_COLOR;
    static const sf::Color BUTTON_PRESSED_COLOR;
    static const sf::Color TEXT_COLOR;
    static const sf::Color DELETE_BUTTON_COLOR;
    static const sf::Color DELETE_BUTTON_HOVER_COLOR;
    static const sf::Color SLIDER_TRACK_COLOR;
    static const sf::Color SLIDER_HANDLE_COLOR;
    static const sf::Color SLIDER_HANDLE_HOVER_COLOR;
    static const sf::Color BACKGROUND_OVERLAY_COLOR;
    static const sf::Color BUTTON_GRADIENT_START;
    static const sf::Color BUTTON_GRADIENT_END;
    static const sf::Color BUTTON_HOVER_GRADIENT_START;
    static const sf::Color BUTTON_HOVER_GRADIENT_END;
    static const sf::Color INPUT_BOX_BORDER_COLOR;
    static const sf::Color INPUT_BOX_FOCUS_COLOR;

    static const sf::Color MEDIEVAL_WOOD_DARK;
    static const sf::Color MEDIEVAL_WOOD_LIGHT;
    static const sf::Color MEDIEVAL_STONE_BASE;
    static const sf::Color MEDIEVAL_GOLD_FRAME;
    static const sf::Color MEDIEVAL_SILVER_FRAME;
    static const sf::Color MEDIEVAL_FIRE_GLOW;
    static const sf::Color MEDIEVAL_MAGIC_GLOW;
    static const sf::Color MEDIEVAL_TEXT_GOLD;
    static const sf::Color MEDIEVAL_RIVET_COLOR;

    static const int BUTTON_HEIGHT = 60;
    static const int BUTTON_WIDTH = 300;
    static const int BUTTON_SPACING = 80;
    static const int MAX_PROFILES;
    static const int MAX_VISIBLE_RESOLUTIONS;
    static const float RESOLUTION_SCROLL_SPEED;

    // Profile file path
    static const std::string PROFILES_FILE_PATH;
    static const std::string SETTINGS_FILE_PATH;

    std::vector<sf::Vector2u> m_availableResolutions;
    int m_selectedResolutionIndex;
    Button m_resolutionButton;
    std::vector<Button> m_resolutionDropdownButtons;
    bool m_resolutionDropdownOpen;

    
};