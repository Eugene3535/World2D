#pragma once

#include <list>

#include "files/TmxLoader.hpp"
#include "graphics/tilemap/TileMap.hpp"


class Location final
{
public:
    Location() noexcept;
    ~Location();

private:
    TmxLoader m_loader;
    TileMap   m_tilemap;

    std::list<Location> m_locations;
};