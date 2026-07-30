#include <SFML/Window/Keyboard.hpp>

#include "app/game/Game.hpp"


void Game::update(const sf::Time deltaTime) noexcept
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        view.move({0, -10});

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        view.move({-10, 0});

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        view.move({0, 10});

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        view.move({10, 0});
}