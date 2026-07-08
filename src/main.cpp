#ifdef _WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "game/Game.hpp"


int main()
{
    Game game;

    if (!game.initialize())
        return 1;

    int returnCode = game.play();

	return returnCode;
}

