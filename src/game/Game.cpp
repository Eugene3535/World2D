#include <SFML/Window/Keyboard.hpp>

#include "game/Game.hpp"



Game::Game() noexcept = default;
Game::~Game() = default;


bool Game::initialize() noexcept
{
    window.create(sf::VideoMode({800, 600}), "Platformer 2D");
    window.setVerticalSyncEnabled(true);
    view = sf::View(sf::FloatRect({ 0.f, 0.f }, { 800.f, 600.f }));

	return true;
}


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