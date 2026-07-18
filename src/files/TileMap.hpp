#pragma once

#include <string>
#include <vector>
#include <list>
#include <span>
#include <memory>
#include <filesystem>

#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Image.hpp>


class TileMap final
{
public:
	struct Object
	{
		struct Property
		{
			std::string name;
			std::string type;
			std::string value;
		};

		std::string  name;
		std::string  type;
		sf::Vector2i position;
		sf::Vector2i size;

		std::vector<Property> properties;
	};

	struct Plane
	{
		std::string                name;
		std::unique_ptr<sf::Image> image;
		std::vector<sf::Vertex>    vertices;
	};

	TileMap() noexcept;
	~TileMap();

	bool loadFromFile(const std::filesystem::path& filepath) noexcept;

	const std::list<Plane>& getPlanes()   const noexcept;
	std::span<const Object> getObjects()  const noexcept;
	const sf::Vector2i&     getMapSize()  const noexcept;
	const sf::Vector2i&     getTileSize() const noexcept;

private:
    bool loadTilePlanes(const void* rootNode) noexcept;
    bool loadObjects(const void* rootNode)    noexcept;

	std::list<Plane>    m_planes;
    std::vector<Object> m_objects;
    sf::Vector2i        m_mapSize;
    sf::Vector2i        m_tileSize;
};