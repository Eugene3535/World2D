#pragma once

#include <string>
#include <vector>
#include <list>
#include <span>
#include <memory>
#include <filesystem>

#include <cglm/struct/vec2.h>

#include "files/StbImage.hpp"


class TileMap final
{
public:
	struct PackedTile 
	{
		enum Type: uint8_t
		{
			Empty,
			Solid,
			Quest
		};

		uint32_t type : 8;
		uint32_t left : 8;
		uint32_t right : 8;
		uint32_t quest : 8;
	};

	struct Object
	{
		struct Property
		{
			std::string name;
			std::string type;
			std::string value;
		};

		std::string name;
		std::string type;
		ivec2s position;
		ivec2s size;

		std::vector<Property> properties;
	};

	struct Plane
	{
		std::string           name;
		StbImage              image;
		std::vector<float>    vertices;
		std::vector<uint32_t> indices;
	};

	TileMap() noexcept;
	~TileMap();

	bool loadFromFile(const std::filesystem::path& filepath) noexcept;

	const std::list<Plane>&     getPlanes()   const noexcept;
	std::span<const Object>     getObjects()  const noexcept;
	std::span<const PackedTile> getTileMask() const noexcept;
	ivec2s                      getMapSize()  const noexcept;
	ivec2s                      getTileSize() const noexcept;

private:
    bool loadTilePlanes(const void* rootNode) noexcept;
    bool loadObjects(const void* rootNode)    noexcept;

	std::list<Plane>        m_planes;
    std::vector<Object>     m_objects;
	std::vector<PackedTile> m_tileMask;
    ivec2s                  m_mapSize;
    ivec2s                  m_tileSize;
};