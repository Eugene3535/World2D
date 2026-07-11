#pragma once

#include <cstdint>
#include <memory>

#include "graphics/transform/Transform2D.hpp"


class Level:
    public Transform2D
{
public:
    Level() noexcept;
    ~Level();

    bool load(const class TileMap& tilemap) noexcept;
    void draw() const noexcept;

private:
    bool loadTextures(const class TileMap& tilemap) noexcept;
    void loadGraphics(const class TileMap& tilemap) noexcept;
    void cleanup() noexcept;

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
};