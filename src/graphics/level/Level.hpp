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

    struct
    {
		uint32_t texture;
		uint32_t VAO; // Vertex Array Object
		uint32_t VBO; // Vertex Buffer Object
        uint32_t EBO; // Element Buffer Object
		uint32_t indexCount;
    } m_background;

    struct
    {
		uint32_t texture;
		uint32_t VAO; // Vertex Array Object
		uint32_t VBO; // Vertex Buffer Object
        uint32_t EBO; // Element Buffer Object
		uint32_t indexCount;
    } m_foreground;
};