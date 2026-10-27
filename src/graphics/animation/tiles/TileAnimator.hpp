#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <cglm/struct/ivec2.h>


class TileAnimator final
{
public:
    struct TileAnimation2D
    {
        uint32_t begin;
        uint32_t end;
    };

    struct Tile
    {
        ivec2s position;  // world coords
        ivec2s size;      // in pixels
        ivec2s texCoords; // UV within a texture
    };

    TileAnimator(uint32_t texture) noexcept;
    TileAnimator(const TileAnimator&) noexcept            = delete;
    TileAnimator(TileAnimator&&) noexcept                 = delete;
    TileAnimator& operator=(const TileAnimator&) noexcept = delete;
    TileAnimator& operator=(TileAnimator&&) noexcept      = delete;
    ~TileAnimator();

    void createLinearAnimaton(uint32_t id, const Tile& tile, uint32_t duration) noexcept;
    void createGridAnimaton(uint32_t id, const Tile& tile, uint32_t columns, uint32_t rows) noexcept;

    std::optional<TileAnimation2D> getAnimation(uint32_t id) const noexcept;

    void pushVerticesOnGPU(const uint32_t vertexBuffer) noexcept;

private:
    std::unordered_map<uint32_t, TileAnimation2D> m_animations;

    const uint32_t     m_texture;
    uint32_t           m_currentTile = 0;
    ivec2s             m_textureSize;
    std::vector<float> m_vertices;        // Accumulation buffer
    bool               m_isLocked{false}; // Once sent to the GPU, the object instance becomes immutable.
};