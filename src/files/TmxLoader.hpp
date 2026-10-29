#pragma once

#include <list>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include <cglm/struct/vec2.h>


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

    struct Object
    {
        struct Property
        {
            std::string                                 name;
            std::variant<bool, int, float, std::string> value;
        };

        std::string name;
        std::string type;
        ivec2s      position;
        ivec2s      size;

        std::vector<Property> properties;
    };


    TmxLoader() noexcept;
    ~TmxLoader();

    bool loadFromFile(const std::string& filename) noexcept;

    const std::list<Layer>& getLayers() const noexcept;
    std::span<const Object> getObjects() const noexcept;
    ivec2s                  getMapSize() const noexcept;
    ivec2s                  getTileSize() const noexcept;

private:
    bool                  loadTilesets(const void* rootNode) noexcept;
    bool                  loadTileLayers(const void* rootNode) noexcept;
    bool                  loadObjects(const void* rootNode) noexcept;
    std::vector<uint32_t> parseCSV(const void* node) noexcept;

    std::list<Layer>    m_layers;
    std::list<Tileset>  m_tilesets;
    std::vector<Object> m_objects;

    ivec2s m_mapSize;
    ivec2s m_tileSize;
};