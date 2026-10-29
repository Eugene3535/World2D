
#include "game/context/Context.hpp"
#include "game/Game.hpp"


bool Context::load(Game& game) noexcept
{
    if (!game.glResources.compilePrograms())
        return false;

    return true;
}


void Context::save(Game& game) noexcept
{
    game.glResources.reset();
}