#pragma once

#include <SFML/Graphics/RenderWindow.hpp>


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
    int play() noexcept;

private:
    sf::RenderWindow m_window;
};