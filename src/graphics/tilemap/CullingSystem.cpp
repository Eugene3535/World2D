#include "graphics/tilemap/CullingSystem.hpp"
#include "files/TmxLoader.hpp"


CullingSystem::CullingSystem(const TmxLoader& loader) noexcept : m_loader(&loader), m_viewport({0, 0, 0, 0}), m_lines(0)
{
    m_counts.fill(0);
    m_offsets.fill(nullptr);
}


CullingSystem::~CullingSystem() = default;


void CullingSystem::updateCulling(const ivec4s& viewport) noexcept
{
    auto are_viewports_equal = [](ivec4s a, ivec4s b) -> bool
    { return (a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w); };

    if (are_viewports_equal(m_viewport, viewport))
        return;

    m_viewport = viewport;

    const auto mapSize         = m_loader->getMapSize();
    const auto tileSize        = m_loader->getTileSize();
    const auto mapSizeInPixels = glms_ivec2_mul(mapSize, tileSize);

    uint32_t left   = (viewport.x < 1) ? 0 : viewport.x;
    uint32_t top    = (viewport.y < 1) ? 0 : viewport.y;
    uint32_t width  = left + viewport.z + (tileSize.x << 1);
    uint32_t height = top + viewport.w + (tileSize.y << 1);

    if (width >= mapSizeInPixels.x)
        width = mapSizeInPixels.x;

    if (height >= mapSizeInPixels.y)
        height = mapSizeInPixels.y;

    const uint32_t x = left / tileSize.x;
    const uint32_t y = top / tileSize.y;

    const uint32_t tile  = x + y * mapSize.x;
    m_lines              = (height - top) / tileSize.y;
    const int32_t length = (width - left) / tileSize.x;

    m_counts.fill(length * 6);

    for (int32_t i = 0; i < m_lines; ++i)
        m_offsets[i] = (const void*)((tile + i * mapSize.x) * 6 * sizeof(uint32_t));
}


CullingSystem::RenderData CullingSystem::getRenderData() const noexcept
{
    return {m_counts.data(), m_offsets.data(), m_lines};
}