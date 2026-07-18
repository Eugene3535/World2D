#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/View.hpp>


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


    sf::View view;
};