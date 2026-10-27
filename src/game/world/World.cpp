#include <cassert>

#include "files/FileProvider.hpp"
#include "game/world/World.hpp"


static World* gInstance;


World::World() noexcept
{
    assert(gInstance == nullptr);
    gInstance = this;
}


World::~World()
{
}


bool World::open() noexcept
{
    return false;
}


World* World::getInstance() noexcept
{
    return gInstance;
}