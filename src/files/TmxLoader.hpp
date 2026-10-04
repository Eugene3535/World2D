#pragma once

#include <string>
#include <vector>
#include <list>
#include <span>
#include <memory>
#include <variant>

#include <cglm/struct/vec2.h>

#include "files/StbImage.hpp"


class TmxLoader final
{
public:
	struct Tileset
	{
		std::string name;
		StbImage image;
		int32_t columns;
		int32_t rows;
		int32_t tileCount;
		int32_t firstGID;
	};

	struct Layer
	{
		std::string           name;
		StbImage*             image;
		std::vector<float>    vertices;
		std::vector<uint32_t> indices;
	};

	struct Object
	{
		struct Property
		{
			std::string name;
			std::variant<bool, int, float, std::string> value;
		};

		std::string name;
		std::string type;
		ivec2s position;
		ivec2s size;

		std::vector<Property> properties;
	};


	TmxLoader() noexcept;
	~TmxLoader();

	bool loadFromFile(const std::string& filename) noexcept;

	const std::list<Layer>& getLayers()   const noexcept;
	std::span<const Object> getObjects()  const noexcept;
	const ivec2s&           getMapSize()  const noexcept;
	const ivec2s&           getTileSize() const noexcept;

private:
	bool loadTilesets(const void* rootNode)         noexcept;
    bool loadTileLayers(const void* rootNode)       noexcept;
    bool loadObjects(const void* rootNode)          noexcept;
	std::vector<int32_t> parseCSV(const void* node) noexcept;

	std::list<Layer>    m_layers;
	std::list<Tileset>  m_tilesets;
    std::vector<Object> m_objects;

    ivec2s m_mapSize;
    ivec2s m_tileSize;
};