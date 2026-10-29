#ifdef _WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement                  = 0x00000001;
    __declspec(dllexport) int           AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "files/FileProvider.hpp"
#include "game/Game.hpp"
#include "game/context/Context.hpp"
#include "game/loop/MainLoop.hpp"
#include "game/window/MainWindow.hpp"
#include "game/window/WindowData.hpp"


int main(int argc, char* argv[])
{
    FileProvider provider(argv[0]);

    Game       game;
    MainWindow window(game.windowData);

    if (!window.open("Platformer 2D", 1200, 920))
        return 1;

    Context context;

    if (!context.load(game))
        return false;

    MainLoop loop(window);
    loop(game);

    context.save(game);

    return 0;
}