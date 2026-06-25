#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <cstdint>
#include <memory>

#include "graphics/transform/Transform2D.hpp"


class Level
{
public:
    Level() noexcept;
    ~Level();

    bool load(const class TileMap& tilemap) noexcept;
    void draw(uint32_t program) const noexcept;

private:
    bool loadTextures(const class TileMap& tilemap) noexcept;
    void loadGraphics(const class TileMap& tilemap) noexcept;
    bool loadShaders() noexcept;
    void cleanup() noexcept;

    Transform2D m_transform;
    std::unique_ptr<class ShaderProgram> m_program;

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

#endif // !LEVEL_HPP