#include <algorithm>
#include <span>

#include <SFML/Graphics/RenderTarget.hpp>

#include "files/tilemap/TileMap.hpp"
#include "graphics/level/Level.hpp"



Level::Level(const TileMap& tilemap) noexcept:
    m_tilemap(tilemap),
    m_background{},
    m_foreground{}
{

}


Level::~Level() = default;


bool Level::load() noexcept
{
    m_background.isEnabled = false;
    m_foreground.isEnabled = false;

    if (!loadTextures())
        return false;

    loadGraphics();

    return true;
}


void Level::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
    states.texture = m_background.texture.get();
    target.draw(m_background.vertexBuffer, states);

    // draw sprites

    if (m_foreground.isEnabled)
    {
        states.texture = m_foreground.texture.get();
        target.draw(m_foreground.vertexBuffer, states);
    }
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
        return (plane.image.operator bool());
    });

    if (textureCount == 0)
        return false;

    m_background.texture = std::make_shared<sf::Texture>();

    if (!m_background.texture->loadFromImage(*planeB->image))
        return false;

    m_background.isEnabled = true;

    if (textureCount > 1)
    {
        m_foreground.texture = std::make_shared<sf::Texture>();

        if (!m_foreground.texture->loadFromImage(*planeA->image))
            return false;
    }
    else
    {
        m_foreground.texture = m_background.texture;
    }

    return true;
}


bool Level::loadGraphics() noexcept
{
    const auto& tilePlanes = m_tilemap.getPlanes();
    const auto tilePlaneCount = tilePlanes.size();

    auto createVertexBuffer = [](std::span<const sf::Vertex> vertices, TilePlane& plane) -> bool
    {
        if (!plane.vertexBuffer.create(vertices.size()))
            return false;

        plane.vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);
        plane.vertexBuffer.setPrimitiveType(sf::PrimitiveType::Triangles);
        plane.isEnabled = plane.vertexBuffer.update(vertices.data());

        return plane.isEnabled;
    };

    std::span<const sf::Vertex> vertices = tilePlanes.front().vertices;

    if (!createVertexBuffer(vertices, m_background))
        return false;

    if (tilePlaneCount > 1)
    {
        vertices = tilePlanes.back().vertices;

        if (!createVertexBuffer(vertices, m_foreground))
            return false;
    }

    return true;
}


const TileMap& Level::getTileMap() const noexcept
{
    return m_tilemap;
}