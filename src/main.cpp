#include <SFML/Graphics/RenderWindow.hpp>

#include "app/context/Context.hpp"
#include "app/loop/MainLoop.hpp"
#include "app/game/Game.hpp"


int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Platformer 2D");
    window.setVerticalSyncEnabled(true);

    Context context(window);
    Game game;

    if (!context.load(game))
        return -1;

    MainLoop loop(window);
    loop(game);

    context.save(game);

	return 0;
}

