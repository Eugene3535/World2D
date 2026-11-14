#pragma once

#include <vector>

#include "graphics/transform/Transform2D.hpp"


class TileMap : public Transform2D
{
public:
    TileMap(const class TmxLoader& loader) noexcept;
    ~TileMap();

    bool create() noexcept;
    void drawBackground(const ivec4s& viewport) noexcept;
    void drawForeground(const ivec4s& viewport) noexcept;

    const class TmxLoader& getLoader() const noexcept;

    bool saveToFile(const class BinaryLoader& saver) noexcept;

private:
    struct TilePlane
    {
        uint32_t texture;
        uint32_t VAO; // Vertex Array Object
        uint32_t VBO; // Vertex Buffer Object
        uint32_t EBO; // Element Buffer Object

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

    bool loadTextures() noexcept;
    void createBufferData(TilePlane& plane, void* bufferData) noexcept;
    void loadGraphics() noexcept;
    void cleanup() noexcept;
    void prepareChunks(TilePlane& plane, const ivec4s& extents) noexcept;
    void draw(const TilePlane& plane, const ivec4s& viewport) const noexcept;

    const class TmxLoader* m_loader{nullptr};
    TilePlane              m_background{};
    TilePlane              m_foreground{};
    bool                   m_isLoaded{false};
};