#pragma once

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

    std::vector<std::unique_ptr<Location>> m_locations;
};