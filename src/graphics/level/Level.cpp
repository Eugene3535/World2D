#include <array>
#include <algorithm>
#include <cstring>
#include <span>

#include <glad/glad.h>

#include "files/TileMap.hpp"
#include "files/Shader.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/vao/VertexBufferLayout.hpp"
#include "graphics/level/Level.hpp"



Level::Level(const TileMap& tilemap) noexcept:
    m_tilemap(tilemap),
    Transform2D(),
    m_background{},
    m_foreground{}
{

}


Level::~Level() 
{
    cleanup();
}


bool Level::load() noexcept
{
    cleanup();

    if (!loadTextures())
        return false;

    loadGraphics();

    return true;
}


void Level::draw() const noexcept
{
    const bool needSecondSampler = ((m_foreground.texture != 0) && (m_foreground.texture != m_background.texture));

	glBindTextureUnit(0, m_background.texture);

    if (needSecondSampler)
        glBindTextureUnit(1, m_foreground.texture);

	glBindVertexArray(m_background.VAO);
	glDrawElements(GL_TRIANGLES, m_background.indexCount, GL_UNSIGNED_INT, nullptr);

    if (m_foreground.VAO)
    {
        glBindVertexArray(m_foreground.VAO);
	    glDrawElements(GL_TRIANGLES, m_foreground.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
	glBindTextureUnit(0, 0);

    if (needSecondSampler)
        glBindTextureUnit(1, 0);
}


bool Level::loadTextures() noexcept
{
    const auto& tilePlanes = m_tilemap.getPlanes();

    auto planeB = std::find_if(tilePlanes.begin(), tilePlanes.end(), [](const TileMap::Plane& plane) -> bool
    {
        return plane.name == "Background";
    });

    auto planeA = std::find_if(tilePlanes.begin(), tilePlanes.end(), [](const TileMap::Plane& plane) -> bool
    {
        return plane.name == "Foreground";
    });

    if (planeB == tilePlanes.end()) // At least one tile layer must exist.
        return false;

    const auto textureCount = std::count_if(tilePlanes.begin(), tilePlanes.end(), [](const TileMap::Plane& plane) -> size_t
    {
        return (plane.image.pixels != nullptr);
    });

    if (textureCount == 0)
        return false;

    std::array<GLuint, 2> textures = { 0, 0 };
    glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(textureCount), textures.data());
    Texture2D background(textures[0]);
    Texture2D foreground(textures[1]);

    if (!background.loadFromImage(planeB->image))
        return false;

    if (textureCount > 1)
    {
        if (!foreground.loadFromImage(planeA->image))
            return false;
    }
    else
    {
        foreground = background;
    }

    m_background.texture = background.getHandle();
    m_foreground.texture = foreground.getHandle();

    return true;
}


void Level::loadGraphics() noexcept
{
    const auto& tilePlanes = m_tilemap.getPlanes();
    const auto tilePlaneCount = tilePlanes.size();

    std::array<GLuint, 4> buffers = {};
    glCreateBuffers(static_cast<GLsizei>(tilePlaneCount << 1), buffers.data());

    m_background.VBO = buffers[0];
    m_background.EBO = buffers[1];
    m_foreground.VBO = buffers[2];
    m_foreground.EBO = buffers[3];

    std::span<const float> vertices = tilePlanes.front().vertices;
    std::span<const uint32_t> indices = tilePlanes.front().indices;
    glNamedBufferStorage(m_background.VBO, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), 0);
    glNamedBufferStorage(m_background.EBO, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), 0);
    m_background.indexCount = indices.size(); 

    if (tilePlaneCount > 1)
    {
        vertices = tilePlanes.back().vertices;
        indices = tilePlanes.back().indices;
        glNamedBufferStorage(m_foreground.VBO, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), 0);
        glNamedBufferStorage(m_foreground.EBO, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), 0);
        m_foreground.indexCount = indices.size(); 
    }

    std::array<GLuint, 2> vertexArrays = {};
    glGenVertexArrays(static_cast<GLsizei>(tilePlaneCount), vertexArrays.data());

    const std::array<VertexBufferLayout::Attribute, 1> attributes = { VertexBufferLayout::Attribute::Float4 };
	VertexBufferLayout layout(attributes);

	layout.createVertexInputState(vertexArrays[0], m_background.VBO);
    glVertexArrayElementBuffer(vertexArrays[0], m_background.EBO);

    if (tilePlaneCount > 1)
    {
        layout.createVertexInputState(vertexArrays[1], m_foreground.VBO);
        glVertexArrayElementBuffer(vertexArrays[1], m_foreground.EBO);
    }

    m_background.VAO = vertexArrays[0];
    m_foreground.VAO = vertexArrays[1];
}


void Level::cleanup() noexcept
{
    const std::array<GLuint, 2> vertexArrays = { m_background.VAO, m_foreground.VAO };
    glDeleteVertexArrays(static_cast<GLsizei>(vertexArrays.size()), vertexArrays.data());

    const std::array<GLuint, 4> buffers = 
    { 
        m_background.VBO, m_background.EBO,
        m_foreground.VBO, m_foreground.EBO 
    };
    glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());

    const std::array<GLuint, 2> textures = { m_background.texture, m_foreground.texture };
    glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());

    memset(&m_background, 0, sizeof(m_background));
    memset(&m_foreground, 0, sizeof(m_foreground));
}


const TileMap& Level::getTileMap() const noexcept
{
    return m_tilemap;
}