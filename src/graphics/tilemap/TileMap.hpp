#pragma once

#include <memory>
#include <vector>

#include "graphics/tilemap/culling/TileChunk.hpp"
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

private:
    struct TilePlane
    {
        uint32_t texture;
        uint32_t VAO; // Vertex Array Object
        uint32_t VBO; // Vertex Buffer Object
        uint32_t EBO; // Element Buffer Object
        uint32_t indexCount;

        struct Chunk
        {
            const void* indices;
            uint32_t    count;
        };

        std::vector<Chunk> chunks;
    };

    bool loadTextures() noexcept;
    void createBufferData(TilePlane& plane, void* bufferData) noexcept;
    void loadGraphics() noexcept;
    void cleanup() noexcept;
    void draw(const TilePlane& plane, const ivec4s& viewport) noexcept;

    const class TmxLoader* m_loader{nullptr};
    TileChunk              m_chunk;
    TilePlane              m_background{};
    TilePlane              m_foreground{};
    bool                   m_isLoaded{false};
};