#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "files/TileMap.hpp"
#include "graphics/level/Level.hpp"
#include "game/Game.hpp"
#include "game/main_loop/MainLoop.hpp"



MainLoop::MainLoop(sf::RenderWindow& rw) noexcept:
    m_window(rw)
{

}


MainLoop::~MainLoop() = default;


bool MainLoop::operator()(Game& game) noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level(tilemap);

    if (!level.load())
        return 1;

    sf::Time timestamp;
    sf::Clock clock;

    while (m_window.isOpen())
	{
        const sf::Time elapsed = clock.getElapsedTime();
        const sf::Time deltaTime = elapsed - timestamp;
        timestamp = elapsed;

        while (const auto event = m_window.pollEvent())
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
                game.view.setSize(sf::Vector2f(resized->size));
            }
        }


        game.update(deltaTime);

        m_window.setView(game.view);
        m_window.clear();
        m_window.draw(level);
		m_window.display();
	}

    return true;
}