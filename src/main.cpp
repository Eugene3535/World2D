#include "game/main_loop/MainLoop.hpp"
#include "game/Game.hpp"


int main()
{
    Game game;

    // Context context;
    // if (!context.load(game) { ... }

    if (!game.initialize())
        return 1;

    MainLoop loop(game);
    loop();

	return 0;
}

