#ifdef _WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "files/TileMap.hpp"

int main()
{
    TileMap tm;
    tm.loadFromFile("res/levels/Level-1.tmx");

	return 0;
}

