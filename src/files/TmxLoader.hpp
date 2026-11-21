#pragma once

#include <list>
#include <span>

#include "files/TmxData.hpp"


class TmxLoader final
{
public:
    struct Tileset
    {
        std::string filename;
        uint32_t    width;
        uint32_t    height;
        uint32_t    columns;
        uint32_t    rows;
        uint32_t    tileCount;
        uint32_t    firstGID;
    };

    struct Layer
    {
        std::string           name;
        const Tileset*        tileset;
        std::vector<uint32_t> tiles;
    };

    TmxLoader() noexcept;
    ~TmxLoader();

    bool loadFromFile(const std::string& filename) noexcept;

    std::string_view           getName() const noexcept;
    const std::list<Layer>&    getLayers() const noexcept;
    std::span<const TmxObject> getObjects() const noexcept;
    ivec2s                     getMapSize() const noexcept;
    ivec2s                     getTileSize() const noexcept;

private:
    bool                  loadTilesets(const void* rootNode) noexcept;
    bool                  loadTileLayers(const void* rootNode) noexcept;
    bool                  loadObjects(const void* rootNode) noexcept;
    std::vector<uint32_t> parseCSV(const void* node) noexcept;

    std::string            m_name;
    std::list<Layer>       m_layers;
    std::list<Tileset>     m_tilesets;
    std::vector<TmxObject> m_objects;

    ivec2s m_mapSize;
    ivec2s m_tileSize;
};