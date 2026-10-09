#pragma once

#include <cstdint>
#include <memory>

#include "graphics/transform/Transform2D.hpp"


class TileMap:
    public Transform2D
{
public:
    TileMap(const class TmxLoader& loader, class GlResourceManager& resources) noexcept;
    ~TileMap();

    bool create() noexcept;
    void draw() const noexcept;

    const class TmxLoader& getLoader() const noexcept;

private:
    struct BufferData
    {
        std::vector<float> vertices;
        std::vector<uint32_t> indices;
        const void* tileLayer;
    };

    bool loadTextures() noexcept;
    void createBufferData(BufferData& data) noexcept;
    void loadGraphics() noexcept;
    void cleanup() noexcept;

    const class TmxLoader*   m_loader;
    class GlResourceManager* m_glResources;

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
    bool m_isLoaded;
};