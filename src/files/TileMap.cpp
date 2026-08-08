#include <list>
#include <algorithm>
#include <charconv>
#include <cstring>
#include <unordered_map>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"

#include "files/TileMap.hpp"


struct Tileset
{
    StbImage image;
    int32_t columns;
    int32_t rows;
    int32_t tileCount;
    int32_t firstGID;
    std::vector<TileMap::PackedTile> tiles;
};


static bool load_tiles(const void* node, std::vector<TileMap::PackedTile>& tiles) noexcept
{
    const auto tilesetNode = static_cast<const rapidxml::xml_node<char>*>(node); 
    auto tileNode = tilesetNode->first_node("tile");

    uint32_t tileCount = 0;
    std::unordered_map<std::string, int> tileData;

    while (tileNode)
    {
        ++tileCount;

        const auto id = tileNode->first_attribute("id");
        const int index = id ? atoi(id->value()) : 0;

        if (const auto propertiesNode = tileNode->first_node("properties"))
        {
            auto propertyNode = propertiesNode->first_node("property");
            auto& tile = tiles[static_cast<size_t>(index)];

            while (propertyNode)
            {
                const std::string name = { propertyNode->first_attribute("name")->value(), propertyNode->first_attribute("name")->value_size() };
                const int value = atoi(propertyNode->first_attribute("value")->value());
                tileData[name] = value;
                propertyNode = propertyNode->next_sibling("property");
            }

            tile.left  = tileData["Left"];
            tile.quest = tileData["Quest"];
            tile.right = tileData["Right"];
            tile.type  = tileData["Type"];
        }

        tileNode = tileNode->next_sibling("tile");
    }

    return (tileCount > 0);
}


static bool load_tilesets(const void* rootNode, std::list<Tileset>& tilesets) noexcept
{
    const auto mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);
    std::vector<std::filesystem::path> imagePaths;

    for (auto tilesetNode = mapNode->first_node("tileset"); tilesetNode; tilesetNode = tilesetNode->next_sibling("tileset"))
    {
        auto imageNode = tilesetNode->first_node("image");

        if (!imageNode)
            continue;

        auto source = imageNode->first_attribute("source");

        if (!source)
            continue;

        const auto filename = std::filesystem::path(source->value()).filename().string();
        const auto filepath = std::filesystem::path("res/textures") / filename;

        auto alreadyExists = std::find(imagePaths.begin(), imagePaths.end(), filepath);

        if (alreadyExists != imagePaths.end())
            continue; // Uploading for the first time only

        imagePaths.push_back(filepath);

        Tileset& tileset = tilesets.emplace_back();

        if (!tileset.image.loadFromFile(filepath))
            return false;

        auto tileCount = tilesetNode->first_attribute("tilecount");
        auto columns   = tilesetNode->first_attribute("columns");
        auto firstGID  = tilesetNode->first_attribute("firstgid");

        tileset.tileCount = tileCount ? atoi(tileCount->value()) : 0;
        tileset.columns   = columns ? atoi(columns->value()) : 0;
        tileset.rows      = (!tileCount || !columns) ? 0 : tileset.tileCount / tileset.columns;
        tileset.firstGID  = firstGID ? atoi(firstGID->value()) : 0;

        if (tileset.firstGID == 1) // The first tileset, so we use it for collision mask
        {
            tileset.tiles.resize(tileset.tileCount, { .type = 0, .left = 32, .right = 32, .quest = 0 });

            if (!load_tiles(tilesetNode, tileset.tiles))
                return false;
        }
    }

    return (!tilesets.empty());
}


static std::vector<int32_t> parse_csv(const void* node) noexcept
{
    const auto dataNode = static_cast<const rapidxml::xml_node<char>*>(node);
    std::string csv(dataNode->value(), dataNode->value_size());

    const auto commaCount = std::count_if(csv.begin(), csv.end(), [](char c){ return c == ','; });

    std::vector<int> result;
    result.reserve(commaCount + 1);

    const char* begin = csv.data();
    const char* end = begin + csv.size();

    while (begin < end) 
    {
        while (begin < end && *begin == ' ') ++begin;

        int value = 0;
        auto [p, ec] = std::from_chars(begin, end, value);

        if (ec == std::errc()) 
        {
            result.push_back(value);
            begin = p;
        }
        else ++begin;

        while (begin < end && (*begin == ',' || *begin == ' ')) ++begin;
    }

    return result;
}



TileMap::TileMap() noexcept = default;
TileMap::~TileMap() = default;


bool TileMap::loadFromFile(const std::filesystem::path& filepath) noexcept
{
    if (!std::filesystem::exists(filepath))
        return false;

    auto document = std::make_unique<rapidxml::xml_document<char>>();
    rapidxml::file<char> xmlFile(filepath.string().c_str());
    document->parse<0>(xmlFile.data());
    const auto mapNode = document->first_node("map");

    m_planes.clear();

    if (loadTilePlanes(static_cast<const void*>(mapNode)))
        // if (loadObjects(static_cast<const void*>(mapNode)))
            return true;

    return false;
}


const std::list<TileMap::Plane>& TileMap::getPlanes() const noexcept
{
    return m_planes;
}


std::span<const TileMap::Object> TileMap::getObjects() const noexcept
{
    return m_objects;
}


std::span<const TileMap::PackedTile> TileMap::getTileMask() const noexcept
{
    return m_tileMask;
}


ivec2s TileMap::getMapSize() const noexcept
{
    return m_mapSize;
}


ivec2s TileMap::getTileSize() const noexcept
{
    return m_tileSize;
}


bool TileMap::loadTilePlanes(const void* rootNode) noexcept
{
	const auto mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);

    std::list<Tileset> tilesets;

	if (!load_tilesets(rootNode, tilesets))
		return false;

	auto mapW  = mapNode->first_attribute("width");
	auto mapH  = mapNode->first_attribute("height");
	auto tileW = mapNode->first_attribute("tilewidth");
	auto tileH = mapNode->first_attribute("tileheight");

	const int32_t mapWidth   = mapW  ? std::atoi(mapW->value())  : 0;
	const int32_t mapHeight  = mapH  ? std::atoi(mapH->value())  : 0;
	const int32_t tileWidth  = tileW ? std::atoi(tileW->value()) : 0;
	const int32_t tileHeight = tileH ? std::atoi(tileH->value()) : 0;

	m_mapSize  = { mapWidth,  mapHeight  };
	m_tileSize = { tileWidth, tileHeight };

	auto layerNode = mapNode->first_node("layer");
    auto tileset = tilesets.begin();

	while (layerNode)
	{
        if (!tileset->tiles.empty())
            m_tileMask.swap(tileset->tiles);

		auto dataNode = layerNode->first_node("data");
		auto parsedLayer = parse_csv(dataNode);
        auto& tilePlane = m_planes.emplace_back();

        if (auto name = layerNode->first_attribute("name"); name)
            tilePlane.name = std::string(name->value(), name->value_size());

        auto& image = tilePlane.image;

        if (tileset->image.pixels)
        {
            image.pixels.swap(tileset->image.pixels);
            image.width = tileset->image.width;
            image.height = tileset->image.height;
            image.bytePerPixel = tileset->image.bytePerPixel;
        }

		const vec2s ratio = { 1.f / (float)image.width, 1.f / (float)image.height };
        size_t index = 0;

        auto& vertices = tilePlane.vertices;
        vertices.reserve(tileset->tileCount * 4);

        auto& indices = tilePlane.indices;
        indices.reserve(tileset->tileCount * 6);

        for (int32_t y = 0u; y < mapHeight; ++y)
        {
            for (int32_t x = 0u; x < mapWidth; ++x)
            {
                const int32_t tileId = parsedLayer[index++];

                if (tileId)
                {
//  Positions
                    const vec2s leftBottom  = { static_cast<float>(x * tileWidth),             static_cast<float>(y * tileHeight + tileHeight) };
                    const vec2s rightBottom = { static_cast<float>(x * tileWidth + tileWidth), static_cast<float>(y * tileHeight + tileHeight) };
                    const vec2s rightTop    = { static_cast<float>(x * tileWidth + tileWidth), static_cast<float>(y * tileHeight) };
                    const vec2s leftTop     = { static_cast<float>(x * tileWidth),             static_cast<float>(y * tileHeight) };

//  UV
                    const int32_t tileNum = tileId - tileset->firstGID;

                    const int32_t vY = (tileNum >= tileset->columns) ? tileNum / tileset->columns : 0;
                    const int32_t vX = tileNum % tileset->columns;

                    const int32_t offsetX = vX * tileWidth;
                    const int32_t offsetY = vY * tileHeight;

                    const float left   = offsetX * ratio.x;
                    const float top    = offsetY * ratio.y;
                    const float right  = (offsetX + tileWidth) * ratio.x;
                    const float bottom = (offsetY + tileHeight) * ratio.y;

                    const uint32_t cell = static_cast<uint32_t>(vertices.size() >> 2);

                    vertices.push_back(leftBottom.x);
                    vertices.push_back(leftBottom.y);
                    vertices.push_back(left);
                    vertices.push_back(bottom);

                    vertices.push_back(rightBottom.x);
                    vertices.push_back(rightBottom.y);
                    vertices.push_back(right);
                    vertices.push_back(bottom);

                    vertices.push_back(rightTop.x);
                    vertices.push_back(rightTop.y);
                    vertices.push_back(right);
                    vertices.push_back(top);

                    vertices.push_back(leftTop.x);
                    vertices.push_back(leftTop.y);
                    vertices.push_back(left);
                    vertices.push_back(top);

                    indices.push_back(cell);
                    indices.push_back(cell + 1u);
                    indices.push_back(cell + 2u);

                    indices.push_back(cell);
                    indices.push_back(cell + 2u);
                    indices.push_back(cell + 3u);
                }
            }
        }

        layerNode = layerNode->next_sibling("layer");

        if (std::next(tileset) != tilesets.end())
            tileset++;
	}

	return (!m_planes.empty());
}


bool TileMap::loadObjects(const void* rootNode) noexcept
{
	const auto mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);

	for (auto objectGroupNode = mapNode->first_node("objectgroup");
		      objectGroupNode;
		      objectGroupNode = objectGroupNode->next_sibling("objectgroup"))
	{
		for (auto objectNode = objectGroupNode->first_node("object");
			      objectNode;
			      objectNode = objectNode->next_sibling("object"))
		{
			auto& object = m_objects.emplace_back();

			for (auto attribute = objectNode->first_attribute(); attribute; attribute = attribute->next_attribute())
			{
				if (strcmp(attribute->name(), "x") == 0)      { object.position.x = atoi(attribute->value()); continue; }
				if (strcmp(attribute->name(), "y") == 0)      { object.position.y = atoi(attribute->value()); continue; }
				if (strcmp(attribute->name(), "width") == 0)  { object.size.x     = atoi(attribute->value()); continue; }
				if (strcmp(attribute->name(), "height") == 0) { object.size.y     = atoi(attribute->value()); continue; }
				if (strcmp(attribute->name(), "name")   == 0)   object.name       = { attribute->value(), attribute->value_size() };
				if (strcmp(attribute->name(), "class")  == 0)   object.type       = { attribute->value(), attribute->value_size() };
			}

			if (const auto propertiesNode = objectNode->first_node("properties"))
			{
				for (auto propertyNode = propertiesNode->first_node("property");
					      propertyNode;
					      propertyNode = propertyNode->next_sibling("property"))
				{
					auto& property = object.properties.emplace_back();

					for (auto attribute = propertyNode->first_attribute(); attribute; attribute = attribute->next_attribute())
					{
						if (strcmp(attribute->name(), "name") == 0) { property.name  = { attribute->value(), attribute->value_size() }; continue; }
						if (strcmp(attribute->name(), "type") == 0) { property.type  = { attribute->value(), attribute->value_size() }; continue; }
						if (strcmp(attribute->name(), "value") == 0)  property.value = { attribute->value(), attribute->value_size() };
					}
				}
			}
		}
	}

	return ( ! m_objects.empty() );
}