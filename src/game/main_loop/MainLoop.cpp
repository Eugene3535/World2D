#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "files/TileMap.hpp"
#include "graphics/level/Level.hpp"
#include "game/Game.hpp"
#include "game/main_loop/MainLoop.hpp"



MainLoop::MainLoop(Game& game) noexcept:
    m_game(game)
{

}


MainLoop::~MainLoop() = default;


bool MainLoop::operator()() noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level(tilemap);

    if (!level.load())
        return 1;

    sf::Time timestamp;
    sf::Clock clock;

    while (m_game.window.isOpen())
	{
        const sf::Time elapsed = clock.getElapsedTime();
        const sf::Time deltaTime = elapsed - timestamp;
        timestamp = elapsed;

        while (const auto event = m_game.window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                m_game.window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    m_game.window.close();
            }

            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                m_game.view.setSize(sf::Vector2f(resized->size));
            }
        }

        m_game.update(deltaTime);

        m_game.window.setView(m_game.view);
        m_game.window.clear();
        m_game.window.draw(level);
		m_game.window.display();
	}

    return true;
}