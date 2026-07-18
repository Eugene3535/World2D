#include <list>
#include <algorithm>
#include <charconv>
#include <cstring>
#include <string_view>
#include <unordered_map>
#include <ranges>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"

#include "files/TileMap.hpp"


struct Tileset
{
    std::unique_ptr<sf::Image> image;
    int32_t                    columns;
    int32_t                    rows;
    int32_t                    tileCount;
    int32_t                    firstGID;
};


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
        tileset.image = std::make_unique<sf::Image>();

        if (!tileset.image->loadFromFile(filepath))
            return false;

        auto tileCount = tilesetNode->first_attribute("tilecount");
        auto columns   = tilesetNode->first_attribute("columns");
        auto firstGID  = tilesetNode->first_attribute("firstgid");

        tileset.tileCount = tileCount ? atoi(tileCount->value()) : 0;
        tileset.columns   = columns ? atoi(columns->value()) : 0;
        tileset.rows      = ((!tileCount) || (!columns)) ? 0 : tileset.tileCount / tileset.columns;
        tileset.firstGID  = firstGID ? atoi(firstGID->value()) : 0;
    }

    return (!tilesets.empty());
}


static std::vector<int32_t> parse_csv(const void* node, std::vector<int>& result) noexcept
{
    const auto dataNode = static_cast<const rapidxml::xml_node<char>*>(node);
    std::string csv(dataNode->value(), dataNode->value_size());

    const auto commaCount = std::count_if(csv.begin(), csv.end(), [](char c){ return c == ','; });
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
        if (loadObjects(static_cast<const void*>(mapNode)))
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


const sf::Vector2i& TileMap::getMapSize() const noexcept
{
    return m_mapSize;
}


const sf::Vector2i& TileMap::getTileSize() const noexcept
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

    if (!(mapWidth && mapHeight && tileWidth && tileHeight))
        return false;

	m_mapSize  = { mapWidth,  mapHeight  };
	m_tileSize = { tileWidth, tileHeight };

	auto layerNode = mapNode->first_node("layer");
    auto tileset = tilesets.begin();

	while (layerNode)
	{
		auto dataNode = layerNode->first_node("data");
        std::vector<int> parsedLayer;
		parse_csv(dataNode, parsedLayer);
        auto& tilePlane  = m_planes.emplace_back();

        if (auto name = layerNode->first_attribute("name"); name)
            tilePlane.name = std::string(name->value(), name->value_size());

        auto& image = tilePlane.image;

        if (tileset->image)
            image.swap(tileset->image);

        const size_t tileCount = std::ranges::count_if(parsedLayer, [](int n) { return n != 0; });

        auto& vertices = tilePlane.vertices;
        vertices.reserve(tileCount * 6);

        const uint32_t rows = tileset->rows;
        const uint32_t columns = tileset->columns;
        size_t index = 0;

        for (uint32_t y = 0u; y < mapHeight; ++y)
        {
            for (uint32_t x = 0u; x < mapWidth; ++x)
            {
                const int32_t tile_id = parsedLayer[index++];

                if(tile_id)
                {
//  Vertex XY coords				
                    const float vx = static_cast<float>(x * tileWidth);
                    const float vy = static_cast<float>(y * tileHeight);

//  Left-top coords of the tile in texture grid
                    const int32_t tileNum = tile_id - 1;
                    const int32_t top = (tileNum >= columns) ? tileNum / columns : 0;
                    const int32_t left = tileNum % columns;
                    const auto uv = sf::Vector2f(left * tileWidth, top * tileHeight);

//  First triangle
                    vertices.emplace_back(sf::Vector2f(vx, vy), sf::Color::White, uv);
                    vertices.emplace_back(sf::Vector2f(vx + tileWidth, vy), sf::Color::White, sf::Vector2f(uv.x + tileWidth, uv.y));
                    vertices.emplace_back(sf::Vector2f(vx + tileWidth, vy + tileHeight), sf::Color::White, sf::Vector2f(uv.x + tileWidth, uv.y + tileHeight));

//  Second triangle
                    vertices.emplace_back(sf::Vector2f(vx, vy), sf::Color::White, uv);
                    vertices.emplace_back(sf::Vector2f(vx + tileWidth, vy + tileHeight), sf::Color::White, sf::Vector2f(uv.x + tileWidth, uv.y + tileHeight));
                    vertices.emplace_back(sf::Vector2f(vx, vy + tileHeight), sf::Color::White, sf::Vector2f(uv.x, uv.y + tileHeight));
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
                    std::string_view type;
                    std::string_view value;

					for (auto attribute = propertyNode->first_attribute(); attribute; attribute = attribute->next_attribute())
					{
						if (strcmp(attribute->name(), "name") == 0) 
                            property.name = { attribute->value(), attribute->value_size() }; 
                        
						else if (strcmp(attribute->name(), "type") == 0)  
                            type  = { attribute->value(), attribute->value_size() };

						else if (strcmp(attribute->name(), "value") == 0) 
                            value = { attribute->value(), attribute->value_size() };
					}

                    if (type == "bool")
                    {
                        property.value = (value == "true") ? true : false; 
                    }
                    else if (type == "int")
                    {
                        property.value = atoi(value.data()); 
                    }
                    else if (type == "float")
                    {
                        float result = 0.f;
                        auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
                        
                        if (ec == std::errc())
                            property.value = result; 
                    }
                    else if (type == "string")
                    {
                        property.value = std::string(value); 
                    }
				}
			}
		}
	}

	return ( ! m_objects.empty() );
}