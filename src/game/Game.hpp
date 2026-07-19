#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "common/action/ActionManager.hpp"


class Game final
{
public:
    enum State
    {
        TitleScreen,
        Stage
    };

    Game() noexcept;
    ~Game();

    bool initialize() noexcept;
    void update(const sf::Time deltaTime) noexcept;

    sf::RenderWindow window;
    sf::View view;
};