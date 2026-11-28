#pragma once

#include <span>
#include <string>
#include <vector>

#include "graphics/transform/Transform2D.hpp"
#include <files/TmxData.hpp>


class TileMap : public Transform2D
{
public:
    TileMap(const std::string& locationName) noexcept;
    ~TileMap();

    bool open() noexcept;
    bool save() noexcept;
    void drawBackground(const ivec4s& viewport) noexcept;
    void drawForeground(const ivec4s& viewport) noexcept;

    ivec2s               getMapSize() const noexcept;
    ivec2s               getTileSize() const noexcept;
    std::span<TmxObject> getObjects() noexcept;

private:
    bool openFromFile(const class BinaryLoader* loader) noexcept;
    bool saveToFile(const class BinaryLoader* saver) noexcept;

    struct TilePlane
    {
        std::string textureName;
        uint32_t    texture;
        uint32_t    VAO; // Vertex Array Object
        uint32_t    VBO; // Vertex Buffer Object
        uint32_t    EBO; // Element Buffer Object

        struct Chunk
        {
            const void* indices; // offset in bytes
            uint32_t    count;   // number of indices
            uint32_t    width;   // in tiles
            uint32_t    height;  // in tiles
        };

        std::vector<Chunk> chunks;
        std::vector<Chunk> visibleChunks;
        uint32_t           width;
        uint32_t           height;
    };

    bool loadTextures(const class TmxLoader& loader) noexcept;
    void createBufferData(TilePlane& plane, void* bufferData) noexcept;
    void loadGraphics(const class TmxLoader& loader) noexcept;
    void cleanup() noexcept;
    void prepareChunks(TilePlane& plane, const ivec4s& extents) noexcept;
    void draw(const TilePlane& plane) const noexcept;

    std::string            m_locationName;
    TilePlane              m_background{};
    TilePlane              m_foreground{};
    std::vector<TmxObject> m_objects;
    ivec2s                 m_mapSize{0, 0};
    ivec2s                 m_tileSize{0, 0};
    bool                   m_isLoaded{false};
};