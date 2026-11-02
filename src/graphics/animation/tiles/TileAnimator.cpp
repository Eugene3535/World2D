#include <array>
#include <span>

#include <glad/glad.h>

#include "graphics/animation/tiles/TileAnimator.hpp"


#define TILE_FLOAT_COUNT   16
#define TILE_COUNT_RESERVE 256


TileAnimator::TileAnimator(uint32_t texture) noexcept :
    m_texture(texture)
{
    glGetTextureLevelParameteriv(m_texture, 0, GL_TEXTURE_WIDTH, &m_textureSize.x);
    glGetTextureLevelParameteriv(m_texture, 0, GL_TEXTURE_HEIGHT, &m_textureSize.y);
    m_vertices.reserve(TILE_FLOAT_COUNT * TILE_COUNT_RESERVE);
}


TileAnimator::~TileAnimator() = default;


void TileAnimator::createLinearAnimaton(uint32_t id, const TileAnimator::Tile& tile, uint32_t duration) noexcept
{
    if (m_isLocked)
        return;

    if (m_animations.find(id) != m_animations.end())
        return;

    auto& animation = m_animations[id];
    animation.begin = m_currentTile;
    animation.end   = m_currentTile + duration;
    m_currentTile += duration;

    std::vector<float> vertices(static_cast<size_t>(duration * TILE_FLOAT_COUNT));
    const vec2s        ratio = {1.f / m_textureSize.x, 1.f / m_textureSize.y};

    auto culculate_tile_bounds = [&vertices, &tile, ratio](uint32_t stride) -> void
    {
        auto* quad = vertices.data() + stride * TILE_FLOAT_COUNT;

        //      positions
        quad[0] = static_cast<float>(tile.position.x + tile.size.x * stride);
        quad[1] = static_cast<float>(tile.position.y);

        quad[4] = static_cast<float>(tile.position.x + tile.size.x * stride + tile.size.x);
        quad[5] = static_cast<float>(tile.position.y);

        quad[8] = static_cast<float>(tile.position.x + tile.size.x * stride + tile.size.x);
        quad[9] = static_cast<float>(tile.position.y + tile.size.y);

        quad[12] = static_cast<float>(tile.position.x + tile.size.x * stride);
        quad[13] = static_cast<float>(tile.position.y + tile.size.y);

        //      UV
        const float left   = tile.texCoords.x * ratio.x;
        const float top    = tile.texCoords.y * ratio.y;
        const float right  = (tile.texCoords.x + tile.size.x) * ratio.x;
        const float bottom = (tile.texCoords.y + tile.size.y) * ratio.y;

        quad[2] = left;
        quad[3] = top;

        quad[6] = right;
        quad[7] = top;

        quad[10] = right;
        quad[11] = bottom;

        quad[14] = left;
        quad[15] = bottom;
    };

    for (int i = 0; i < duration; ++i)
    {
        culculate_tile_bounds(i);
    }

    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
}


void TileAnimator::createGridAnimaton(uint32_t id, const TileAnimator::Tile& tile, uint32_t columns, uint32_t rows) noexcept
{
    if (m_isLocked)
        return;

    if (m_animations.find(id) != m_animations.end())
        return;

    auto& animation = m_animations[id];
    animation.begin = m_currentTile;
    animation.end   = m_currentTile + columns * rows;
    m_currentTile += columns * rows;

    std::vector<float> vertices(static_cast<size_t>(columns * rows * TILE_FLOAT_COUNT));
    const vec2s        ratio = {1.f / m_textureSize.x, 1.f / m_textureSize.y};

    auto culculate_tile_bounds = [&vertices, &tile, ratio](uint32_t row, uint32_t column) -> void
    {
        auto* quad = vertices.data() + row * column * TILE_FLOAT_COUNT;

        //      positions
        quad[0] = static_cast<float>(tile.position.x + tile.size.x * column);
        quad[1] = static_cast<float>(tile.position.y + tile.size.y * row);

        quad[4] = static_cast<float>(tile.position.x + tile.size.x * column + tile.size.x);
        quad[5] = static_cast<float>(tile.position.y + tile.size.y * row);

        quad[8] = static_cast<float>(tile.position.x + tile.size.x * column + tile.size.x);
        quad[9] = static_cast<float>(tile.position.y + tile.size.y * row + tile.size.y);

        quad[12] = static_cast<float>(tile.position.x + tile.size.x * column);
        quad[13] = static_cast<float>(tile.position.y + tile.size.y * row + tile.size.y);

        //      UV
        const float left   = (tile.texCoords.x + tile.size.x * column) * ratio.x;
        const float top    = (tile.texCoords.y + tile.size.y * row) * ratio.y;
        const float right  = (tile.texCoords.x + tile.size.x * column + tile.size.x) * ratio.x;
        const float bottom = (tile.texCoords.y + tile.size.y * row + tile.size.y) * ratio.y;

        quad[2] = left;
        quad[3] = top;

        quad[6] = right;
        quad[7] = top;

        quad[10] = right;
        quad[11] = bottom;

        quad[14] = left;
        quad[15] = bottom;
    };

    for (uint32_t y = 0; y < rows; ++y)
    {
        for (uint32_t x = 0; x < columns; ++x)
        {
            culculate_tile_bounds(y, x);
        }
    }

    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
}


std::optional<TileAnimator::TileAnimation2D> TileAnimator::getAnimation(uint32_t id) const noexcept
{
    if (auto it = m_animations.find(id); it != m_animations.end())
        return it->second;

    return std::nullopt;
}


void TileAnimator::pushVerticesOnGPU(const uint32_t vertexBuffer) noexcept
{
    if (m_vertices.empty())
        return;

    glNamedBufferStorage(vertexBuffer,
                         std::span<float>(m_vertices).size_bytes(),
                         static_cast<const void*>(m_vertices.data()),
                         0);

    std::vector<float>().swap(m_vertices);
    m_isLocked = true;
}