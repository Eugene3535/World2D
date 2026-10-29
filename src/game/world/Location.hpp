#pragma once

#include <list>

#include "files/TmxLoader.hpp"
#include "graphics/tilemap/TileMap.hpp"


// Перенести все загрузки ресурсов в AssetManager, чтоб по 20 раз не загружать текстуры


class Location final
{
public:
    Location(class GlResourceManager& resources) noexcept;
    ~Location();

private:
    class GlResourceManager* m_glResources;

    TmxLoader m_loader;
    TileMap   m_tilemap;

    std::list<Location> m_locations;
};