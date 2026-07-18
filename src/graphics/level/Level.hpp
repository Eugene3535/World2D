#pragma once

#include <memory>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/Texture.hpp>

class Level:
    public sf::Drawable
{
public:
    Level(const class TileMap& tilemap) noexcept;
    ~Level();

    bool load() noexcept;

    const class TileMap& getTileMap() const noexcept;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept override;

    bool loadTextures() noexcept;
    bool loadGraphics() noexcept;

    const class TileMap& m_tilemap;

    struct TilePlane
    {
        sf::VertexBuffer vertexBuffer;
        std::shared_ptr<sf::Texture> texture;
        bool isEnabled;
    };

    TilePlane m_background;
    // Sprite plane as middleground
    TilePlane m_foreground;
};