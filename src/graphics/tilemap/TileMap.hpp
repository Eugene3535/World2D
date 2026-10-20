#pragma once

#include <memory>

#include "graphics/tilemap/CullingSystem.hpp"
#include "graphics/transform/Transform2D.hpp"


class TileMap : public Transform2D
{
public:
    TileMap(const class TmxLoader& loader, class GlResourceManager& resources) noexcept;
    ~TileMap();

    bool create() noexcept;
    void draw(const ivec4s& viewport) noexcept;

    const class TmxLoader& getLoader() const noexcept;

private:
    bool loadTextures() noexcept;
    void createBufferData(void* bufferData) noexcept;
    void loadGraphics() noexcept;
    void cleanup() noexcept;

    const class TmxLoader*   m_loader;
    class GlResourceManager* m_glResources;
    CullingSystem            m_culling;

    struct TilePlane
    {
        uint32_t texture;
        uint32_t VAO; // Vertex Array Object
        uint32_t VBO; // Vertex Buffer Object
        uint32_t EBO; // Element Buffer Object
        uint32_t indexCount;
    };

    TilePlane m_background;
    TilePlane m_foreground;
    bool      m_isLoaded;
};