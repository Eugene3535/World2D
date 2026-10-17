#pragma once

#include <array>

#include <cglm/struct/ivec2.h>


class CullingSystem
{
public:
    struct RenderData
    {
        const int32_t*     counts;
        const void* const* indices;
        int32_t            drawcount;
    };

    CullingSystem(const class TmxLoader& loader) noexcept;
    virtual ~CullingSystem();

protected:
    void updateCulling(const ivec4s& viewport) noexcept;

    RenderData getRenderData() const noexcept;

private:
    const class TmxLoader* m_loader;

    ivec4s  m_viewport;
    int32_t m_lines;

    std::array<int32_t, 64>     m_counts;
    std::array<const void*, 64> m_offsets;
};