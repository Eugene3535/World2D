#include <SFML/Graphics/RenderWindow.hpp>

#include "game/main_loop/MainLoop.hpp"
#include "game/Game.hpp"


int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Platformer 2D");
    window.setVerticalSyncEnabled(true);

    MainLoop loop(window);
    Game game;

    if (!game.initialize())
        return 1;

    loop(game);

	return 0;
}

