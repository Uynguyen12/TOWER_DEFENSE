#include "MenuManager.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Static color definitions
const sf::Color MenuManager::BUTTON_NORMAL_COLOR = sf::Color(70, 70, 70, 200);
const sf::Color MenuManager::BUTTON_HOVER_COLOR = sf::Color(100, 100, 100, 200);
const sf::Color MenuManager::BUTTON_PRESSED_COLOR = sf::Color(50, 50, 50, 200);
const sf::Color MenuManager::TEXT_COLOR = sf::Color::White;
const sf::Color MenuManager::DELETE_BUTTON_COLOR = sf::Color(150, 50, 50, 200);
const sf::Color MenuManager::DELETE_BUTTON_HOVER_COLOR = sf::Color(200, 70, 70, 200);
const std::string MenuManager::PROFILES_FILE_PATH = "profiles.json";
const int MenuManager::MAX_PROFILES = 5;

MenuManager::MenuManager()
    : m_currentState(MenuState::ProfileMenu)
    , m_previousState(MenuState::ProfileMenu)
    , m_currentProfile(nullptr)
    , m_waitingForNameInput(false)
    , m_showWarning(false)
    , m_warningTimer(0.0f)
    , m_gamePaused(false)
    , m_selectedLevel(1)
{
}

MenuManager::~MenuManager() {
    SaveProfilesToFile();
}

void MenuManager::Initialize(sf::RenderWindow& window) {
    m_windowSize = sf::Vector2f(window.getSize());
    LoadResources();
    LoadProfilesFromFile();
    SetMenuState(MenuState::ProfileMenu);
}

void MenuManager::LoadResources() {
    if (!m_font.loadFromFile("Fonts/Kreon-Medium.ttf")) {
        std::cerr << "Warning: Could not load font. Using default font." << std::endl;
    }

    m_titleText.setFont(m_font);
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(TEXT_COLOR);

    m_warningText.setFont(m_font);
    m_warningText.setCharacterSize(24);
    m_warningText.setFillColor(sf::Color::Red);

    m_inputPromptText.setFont(m_font);
    m_inputPromptText.setCharacterSize(32);
    m_inputPromptText.setFillColor(TEXT_COLOR);
    m_inputPromptText.setString("Enter your name:");

    m_inputText.setFont(m_font);
    m_inputText.setCharacterSize(24);
    m_inputText.setFillColor(sf::Color::Black);

    m_inputBox.setSize(sf::Vector2f(400, 50));
    m_inputBox.setFillColor(sf::Color::White);
    m_inputBox.setOutlineThickness(2);
    m_inputBox.setOutlineColor(sf::Color::Black);

    m_pauseBackground.setSize(sf::Vector2f(400, 300));
    m_pauseBackground.setFillColor(sf::Color(0, 0, 0, 180));
    m_pauseBackground.setOutlineThickness(3);
    m_pauseBackground.setOutlineColor(sf::Color::White);
}

void MenuManager::Update(sf::RenderWindow& window, float deltaTime) {
    UpdateButtons(window);

    if (m_showWarning) {
        m_warningTimer -= deltaTime;
        if (m_warningTimer <= 0.0f) {
            m_showWarning = false;
        }
    }
}

void MenuManager::Draw(sf::RenderWindow& window) {
    if (m_backgroundTexture.getSize().x > 0) {
        window.draw(m_backgroundSprite);
    }

    window.draw(m_titleText);
    DrawButtons(window);

    if (m_waitingForNameInput) {
        window.draw(m_inputPromptText);
        window.draw(m_inputBox);
        window.draw(m_inputText);
    }

    if (m_showWarning) {
        window.draw(m_warningText);
    }

    if (m_gamePaused && m_currentState == MenuState::GamePlay) {
        DrawPauseMenu(window);
    }
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

            if (m_gamePaused && m_currentState == MenuState::GamePlay) {
                for (auto& button : m_pauseButtons) {
                    if (button.isVisible && IsMouseOverButton(button, mousePos)) {
                        button.state = ButtonState::Pressed;
                        if (button.callback) {
                            button.callback();
                        }
                        break;
                    }
                }
            }
            else {
                for (auto& button : m_buttons) {
                    if (button.isVisible && IsMouseOverButton(button, mousePos)) {
                        button.state = ButtonState::Pressed;
                        if (button.callback) {
                            button.callback();
                        }
                        break;
                    }
                }
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
                switch (m_currentState) {
                case MenuState::ProfileMenu:
                    if (m_exitCallback) {
                        m_exitCallback();
                    }
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
                case MenuState::DifficultyMenu:
                    SetMenuState(MenuState::PlayMenu);
                    break;
                case MenuState::GamePlay:
                    TogglePauseMenu();
                    break;
                case MenuState::Settings:
                    SetMenuState(MenuState::MainMenu);
                    break;
                }
            }
        }
    }
}

void MenuManager::TogglePauseMenu() {
    m_gamePaused = !m_gamePaused;

    if (m_gamePaused) {
        CreatePauseMenu();
        std::cout << "Game paused" << std::endl;
    }
    else {
        m_pauseButtons.clear();
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
        [this]() {
            TogglePauseMenu();
        });

    CreatePauseButton("Settings",
        sf::Vector2f(centerPos.x + (m_pauseBackground.getSize().x - buttonWidth) / 2, startY + buttonSpacing),
        sf::Vector2f(buttonWidth, buttonHeight),
        [this]() {
            ShowWarningMessage("Settings menu not implemented yet!");
        });

    CreatePauseButton("Exit to Menu",
        sf::Vector2f(centerPos.x + (m_pauseBackground.getSize().x - buttonWidth) / 2, startY + buttonSpacing * 2),
        sf::Vector2f(buttonWidth, buttonHeight),
        [this]() {
            m_gamePaused = false;
            m_pauseButtons.clear();
            SetMenuState(MenuState::MainMenu);
        });
}

void MenuManager::CreateProfileMenu() {
    m_buttons.clear();
    m_titleText.setString("Profile Menu");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - BUTTON_WIDTH / 2;
    float startY = m_windowSize.y / 2 - BUTTON_HEIGHT;

    CreateButton("Create Profile",
        sf::Vector2f(centerX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::CreateProfile);
        });

    CreateButton("Choose Profile",
        sf::Vector2f(centerX, startY + BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::ChooseProfile);
        });

    CreateButton("Exit",
        sf::Vector2f(centerX, startY + BUTTON_SPACING * 2),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            if (m_exitCallback) {
                m_exitCallback();
            }
        });
}

void MenuManager::CreateChooseProfileMenu() {
    m_buttons.clear();
    m_titleText.setString("Choose Profile");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - BUTTON_WIDTH / 2;
    float startY = m_windowSize.y / 2 - BUTTON_HEIGHT * m_profiles.size() / 2;

    for (size_t i = 0; i < m_profiles.size(); ++i) {
        CreateButton(m_profiles[i].name,
            sf::Vector2f(centerX, startY + i * BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this, i]() {
                SelectProfile(i);
                SetMenuState(MenuState::MainMenu);
            });

        CreateDeleteButton("Delete",
            sf::Vector2f(centerX + BUTTON_WIDTH + 20, startY + i * BUTTON_SPACING),
            sf::Vector2f(100, BUTTON_HEIGHT),
            [this, i]() {
                DeleteProfile(i);
            });
    }

    CreateButton("Back",
        sf::Vector2f(centerX, startY + m_profiles.size() * BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::ProfileMenu);
        });
}

void MenuManager::CreateNewProfileMenu() {
    m_buttons.clear();
    m_titleText.setString("Create New Profile");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - 400 / 2;
    float centerY = m_windowSize.y / 2 - 50 / 2;

    m_inputBox.setPosition(sf::Vector2f(centerX, centerY));
    m_inputPromptText.setPosition(sf::Vector2f(centerX, centerY - 50));
    m_inputText.setPosition(sf::Vector2f(centerX + 10, centerY + 10));

    SetWaitingForInput(true);
}

void MenuManager::CreateMainMenu() {
    m_buttons.clear();
    m_titleText.setString("Main Menu");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - BUTTON_WIDTH / 2;
    float startY = m_windowSize.y / 2 - BUTTON_HEIGHT;

    CreateButton("Play",
        sf::Vector2f(centerX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::PlayMenu);
        });

    CreateButton("Settings",
        sf::Vector2f(centerX, startY + BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            ShowWarningMessage("Settings menu not implemented yet!");
        });

    CreateButton("Exit",
        sf::Vector2f(centerX, startY + BUTTON_SPACING * 2),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::ProfileMenu);
        });
}

void MenuManager::CreatePlayMenu() {
    m_buttons.clear();
    m_titleText.setString("Select Level");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - BUTTON_WIDTH / 2;
    float startY = m_windowSize.y / 2 - BUTTON_HEIGHT * 2;

	const int numsOfLevels = 4;
    for (int i = 1; i <= numsOfLevels; ++i) {
        CreateButton("Level " + std::to_string(i),
            sf::Vector2f(centerX, startY + (i - 1) * BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this, i]() {
                m_selectedLevel = i;
                SetMenuState(MenuState::DifficultyMenu);
            });
    }

    CreateButton("Back",
        sf::Vector2f(centerX, startY + numsOfLevels * BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::MainMenu);
        });
}

void MenuManager::CreateDifficultyMenu() {
    m_buttons.clear();
    m_titleText.setString("Select Difficulty");
    CenterText(m_titleText, sf::RectangleShape(sf::Vector2f(m_windowSize.x, 100)));

    float centerX = m_windowSize.x / 2 - BUTTON_WIDTH / 2;
    float startY = m_windowSize.y / 2 - BUTTON_HEIGHT * 2;

    CreateButton("Easy",
        sf::Vector2f(centerX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            if (m_startGameCallback) {
                m_startGameCallback(m_selectedLevel, Difficulty::Easy);
            }
            SetMenuState(MenuState::GamePlay);
        });

    CreateButton("Medium",
        sf::Vector2f(centerX, startY + BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            if (m_startGameCallback) {
                m_startGameCallback(m_selectedLevel, Difficulty::Medium);
            }
            SetMenuState(MenuState::GamePlay);
        });

    CreateButton("Hard",
        sf::Vector2f(centerX, startY + BUTTON_SPACING * 2),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            if (m_startGameCallback) {
                m_startGameCallback(m_selectedLevel, Difficulty::Hard);
            }
            SetMenuState(MenuState::GamePlay);
        });

    CreateButton("Very Hard",
        sf::Vector2f(centerX, startY + BUTTON_SPACING * 3),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            if (m_startGameCallback) {
                m_startGameCallback(m_selectedLevel, Difficulty::VeryHard);
            }
            SetMenuState(MenuState::GamePlay);
        });

    CreateButton("Back",
        sf::Vector2f(centerX, startY + BUTTON_SPACING * 4),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() {
            SetMenuState(MenuState::PlayMenu);
        });
}

void MenuManager::CreateButton(const std::string& text, sf::Vector2f position, sf::Vector2f size, std::function<void()> callback) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(BUTTON_NORMAL_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.text.setFont(m_font);
    button.text.setString(text);
    button.text.setCharacterSize(24);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);
    button.callback = callback;
    button.isVisible = true;
    button.isDeleteButton = false;
    m_buttons.push_back(button);
}

void MenuManager::CreateDeleteButton(const std::string& text, sf::Vector2f position, sf::Vector2f size, std::function<void()> callback) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(DELETE_BUTTON_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.text.setFont(m_font);
    button.text.setString(text);
    button.text.setCharacterSize(24);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);
    button.callback = callback;
    button.isVisible = true;
    button.isDeleteButton = true;
    m_buttons.push_back(button);
}

void MenuManager::CreatePauseButton(const std::string& text, sf::Vector2f position, sf::Vector2f size, std::function<void()> callback) {
    Button button;
    button.shape.setPosition(position);
    button.shape.setSize(size);
    button.shape.setFillColor(BUTTON_NORMAL_COLOR);
    button.shape.setOutlineThickness(2);
    button.shape.setOutlineColor(sf::Color::White);
    button.text.setFont(m_font);
    button.text.setString(text);
    button.text.setCharacterSize(24);
    button.text.setFillColor(TEXT_COLOR);
    CenterText(button.text, button.shape);
    button.callback = callback;
    button.isVisible = true;
    button.isDeleteButton = false;
    m_pauseButtons.push_back(button);
}

void MenuManager::UpdateButtons(sf::RenderWindow& window) {
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    for (auto& button : m_buttons) {
        if (!button.isVisible) continue;

        if (IsMouseOverButton(button, mousePos)) {
            button.state = sf::Mouse::isButtonPressed(sf::Mouse::Left) ? ButtonState::Pressed : ButtonState::Hovered;
            button.shape.setFillColor(button.isDeleteButton ?
                (button.state == ButtonState::Pressed ? BUTTON_PRESSED_COLOR : DELETE_BUTTON_HOVER_COLOR) :
                (button.state == ButtonState::Pressed ? BUTTON_PRESSED_COLOR : BUTTON_HOVER_COLOR));
        }
        else {
            button.state = ButtonState::Normal;
            button.shape.setFillColor(button.isDeleteButton ? DELETE_BUTTON_COLOR : BUTTON_NORMAL_COLOR);
        }
    }

    for (auto& button : m_pauseButtons) {
        if (!button.isVisible) continue;

        if (IsMouseOverButton(button, mousePos)) {
            button.state = sf::Mouse::isButtonPressed(sf::Mouse::Left) ? ButtonState::Pressed : ButtonState::Hovered;
            button.shape.setFillColor(button.state == ButtonState::Pressed ? BUTTON_PRESSED_COLOR : BUTTON_HOVER_COLOR);
        }
        else {
            button.state = ButtonState::Normal;
            button.shape.setFillColor(BUTTON_NORMAL_COLOR);
        }
    }
}

void MenuManager::DrawButtons(sf::RenderWindow& window) {
    for (const auto& button : m_buttons) {
        if (button.isVisible) {
            window.draw(button.shape);
            window.draw(button.text);
        }
    }
}

bool MenuManager::IsMouseOverButton(const Button& button, sf::Vector2f mousePos) {
    return button.shape.getGlobalBounds().contains(mousePos);
}

void MenuManager::CenterText(sf::Text& text, const sf::RectangleShape& shape) {
    sf::FloatRect textBounds = text.getLocalBounds();
    sf::Vector2f shapePos = shape.getPosition();
    sf::Vector2f shapeSize = shape.getSize();
    text.setPosition(
        shapePos.x + (shapeSize.x - textBounds.width) / 2,
        shapePos.y + (shapeSize.y - textBounds.height) / 2 - textBounds.top
    );
}

void MenuManager::SetMenuState(MenuState newState) {
    m_previousState = m_currentState;
    m_currentState = newState;
    m_buttons.clear();
    m_showWarning = false;

    switch (m_currentState) {
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
    case MenuState::DifficultyMenu:
        CreateDifficultyMenu();
        break;
    case MenuState::GamePlay:
        m_gamePaused = false;
        m_pauseButtons.clear();
        break;
    case MenuState::Settings:
        // Not implemented yet
        break;
    }
}

void MenuManager::CreateNewProfile(const std::string& name) {
    if (m_profiles.size() >= MAX_PROFILES) {
        ShowWarningMessage("Maximum number of profiles reached!");
        return;
    }

    for (const auto& profile : m_profiles) {
        if (profile.name == name) {
            ShowWarningMessage("Profile name already exists!");
            return;
        }
    }

    m_profiles.emplace_back(name);
    m_currentProfile = &m_profiles.back();
    SaveProfilesToFile();
}

void MenuManager::SelectProfile(int index) {
    if (index >= 0 && index < m_profiles.size()) {
        m_currentProfile = &m_profiles[index];
    }
}

void MenuManager::DeleteProfile(int index) {
    if (index >= 0 && index < m_profiles.size()) {
        if (&m_profiles[index] == m_currentProfile) {
            m_currentProfile = nullptr;
        }
        m_profiles.erase(m_profiles.begin() + index);
        SaveProfilesToFile();
        CreateChooseProfileMenu();
    }
}

void MenuManager::SaveProfilesToFile() {
    json j;
    for (const auto& profile : m_profiles) {
        json j_profile;
        j_profile["name"] = profile.name;
        j_profile["level"] = profile.level;
        j_profile["experience"] = profile.experience;
        j_profile["highScore"] = profile.highScore;
        j_profile["savedLevel"] = profile.savedLevel;
        j_profile["savedDifficulty"] = profile.savedDifficulty;
        j_profile["savedGold"] = profile.savedGold;

        json j_towers = json::array();
        for (const auto& tower : profile.savedTowers) {
            j_towers.push_back({ {"x", tower.x}, {"y", tower.y}, {"type", tower.type}, {"level", tower.level} });
        }
        j_profile["savedTowers"] = j_towers;

        json j_map = json::array();
        for (const auto& row : profile.savedMapLayout) {
            j_map.push_back(row);
        }
        j_profile["savedMapLayout"] = j_map;

        json j_path = json::array();
        for (const auto& point : profile.savedEnemyPath) {
            j_path.push_back({ {"x", point.x}, {"y", point.y} });
        }
        j_profile["savedEnemyPath"] = j_path;

        j["profiles"].push_back(j_profile);
    }

    std::ofstream file(PROFILES_FILE_PATH);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
    else {
        std::cerr << "Failed to save profiles to " << PROFILES_FILE_PATH << std::endl;
    }
}

void MenuManager::LoadProfilesFromFile() {
    std::ifstream file(PROFILES_FILE_PATH);
    if (!file.is_open()) {
        std::cerr << "No profile file found, starting with empty profiles." << std::endl;
        return;
    }

    json j;
    try {
        file >> j;
    }
    catch (const json::exception& e) {
        std::cerr << "Error parsing profiles file: " << e.what() << std::endl;
        file.close();
        return;
    }
    file.close();

    m_profiles.clear();
    for (const auto& j_profile : j["profiles"]) {
        PlayerProfile profile;
        profile.name = j_profile.value("name", "");
        profile.level = j_profile.value("level", 1);
        profile.experience = j_profile.value("experience", 0);
        profile.highScore = j_profile.value("highScore", 0);
        profile.savedLevel = j_profile.value("savedLevel", 1);
        profile.savedDifficulty = j_profile.value("savedDifficulty", 1.0f);
        profile.savedGold = j_profile.value("savedGold", 10);

        if (j_profile.contains("savedTowers")) {
            for (const auto& j_tower : j_profile["savedTowers"]) {
                TowerData tower;
                tower.x = j_tower.value("x", 0);
                tower.y = j_tower.value("y", 0);
                tower.type = j_tower.value("type", 1);
                tower.level = j_tower.value("level", 1);
                profile.savedTowers.push_back(tower);
            }
        }

        if (j_profile.contains("savedMapLayout")) {
            for (const auto& j_row : j_profile["savedMapLayout"]) {
                std::vector<int> row;
                for (const auto& value : j_row) {
                    row.push_back(value.get<int>());
                }
                profile.savedMapLayout.push_back(row);
            }
        }

        if (j_profile.contains("savedEnemyPath")) {
            for (const auto& j_point : j_profile["savedEnemyPath"]) {
                PathPoint point;
                point.x = j_point.value("x", 0);
                point.y = j_point.value("y", 0);
                profile.savedEnemyPath.push_back(point);
            }
        }

        m_profiles.push_back(profile);
    }
}

void MenuManager::HandleTextInput(sf::Uint32 unicode) {
    if (unicode == 8 && !m_inputBuffer.empty()) { // Backspace
        m_inputBuffer.pop_back();
    }
    else if (unicode >= 32 && unicode <= 126 && m_inputBuffer.size() < 20) { // Printable ASCII
        m_inputBuffer += static_cast<char>(unicode);
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
    m_warningText.setPosition(
        (m_windowSize.x - textBounds.width) / 2,
        m_windowSize.y / 2 + 100
    );
    m_showWarning = true;
    m_warningTimer = 2.0f; // Display for 2 seconds
}

bool MenuManager::IsGamePaused() const {
    return m_gamePaused;
}

void MenuManager::SetStartGameCallback(std::function<void(int, Difficulty)> callback) {
    m_startGameCallback = callback;
}

void MenuManager::ReturnToMenu() {
    m_gamePaused = false;
    m_pauseButtons.clear();
    SetMenuState(MenuState::MainMenu);
}