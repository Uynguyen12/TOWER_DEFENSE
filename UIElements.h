#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "SoundManager.h"

struct Button {
    sf::Text text;
    sf::RectangleShape shape;

    Button(const std::string& label, const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size) {
        text.setString(label);
        text.setFont(font);
        text.setCharacterSize(30);
        text.setFillColor(sf::Color::White);

        shape.setPosition(position);
        shape.setSize(size);
        shape.setFillColor(sf::Color(100, 100, 100));
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(2.f);

        // Center text on the button
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    }

    void draw(sf::RenderTarget& target) const {
        target.draw(shape);
        target.draw(text);
    }

    bool isMouseOver(const sf::Vector2f& mousePos) {
        bool isOver = shape.getGlobalBounds().contains(mousePos);
        if (isOver) {
            shape.setFillColor(sf::Color(150, 150, 150)); // Hover color
        }
        else {
            shape.setFillColor(sf::Color(100, 100, 100)); // Normal color
        }
        return isOver;
    }

    void handleClick() {
        SoundManager::getInstance().playSfx(SoundManager::SfxType::ButtonClick);
        // Optional: Add a visual click effect
        shape.setFillColor(sf::Color(50, 50, 50));
    }
};
