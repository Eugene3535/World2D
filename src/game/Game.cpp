#include "files/TileMap.hpp"
#include "graphics/level/Level.hpp"
#include "game/Game.hpp"



Game::Game() noexcept = default;
Game::~Game() = default;


bool Game::initialize() noexcept
{
    m_window.create(sf::VideoMode({800, 600}), "Platformer 2D");
    m_window.setVerticalSyncEnabled(true);

	return true;
}


int Game::play() noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level(tilemap);

    if (!level.load())
        return 1;

    sf::View view({{ 0.f, 0.f }, { 800.f, 600.f }});

    while (m_window.isOpen())
	{
        while (const std::optional event = m_window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                m_window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    m_window.close();
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                view.setSize(sf::Vector2f(resized->size));
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            view.move({0, -10});

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            view.move({-10, 0});

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            view.move({0, 10});

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            view.move({10, 0});

        m_window.setView(view);
        m_window.clear();
        m_window.draw(level);
		m_window.display();
	}

	return 0;
}