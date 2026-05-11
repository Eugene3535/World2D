#ifndef TILE_MAP_HPP
#define TILE_MAP_HPP

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <cstring>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"

struct Object
{
    std::string name;
    std::string type;
    sf::FloatRect bounds;
};

class Level
{
public:
    bool LoadFromFile(const std::filesystem::path& filepath, const sf::Texture* t);
    Object GetObject(std::string name);
    std::vector<Object> GetObjects(std::string name);
	std::vector<Object> GetAllObjects();
    void Draw(sf::RenderWindow &window);
	sf::Vector2i GetTileSize();

private:
    int width, height, tileWidth, tileHeight;
    int firstTileID;
    sf::Rect<float> drawingBounds;
    const sf::Texture* tileset = nullptr;
    std::vector<Object> objects;
    std::vector<sf::VertexBuffer> layers;
};

bool Level::LoadFromFile(const std::filesystem::path& filepath, const sf::Texture* t)
{
    auto document = std::make_unique<rapidxml::xml_document<char>>();
	rapidxml::file<char> xmlFile(filepath.string().c_str());
	document->parse<0>(xmlFile.data());

    const auto mapNode = document->first_node("map");

    if (!mapNode)
		return false;

	auto pMapW  = mapNode->first_attribute("width");
	auto pMapH  = mapNode->first_attribute("height");
	auto pTileW = mapNode->first_attribute("tilewidth");
	auto pTileH = mapNode->first_attribute("tileheight");

	width   = pMapW  ? std::atoi(pMapW->value())  : 0;
	height  = pMapH  ? std::atoi(pMapH->value())  : 0;
	tileWidth  = pTileW ? std::atoi(pTileW->value()) : 0;
	tileHeight = pTileH ? std::atoi(pTileH->value()) : 0;

	tileset = t;

    auto layerNode = mapNode->first_node("layer");

    while(layerNode)
    {
        auto dataNode = layerNode->first_node("data"); 

        if(!dataNode)
            return false;

        std::vector<int> parsedLayer;
        std::string data(dataNode->value());

        std::replace(data.begin(), data.end(), ',', ' ');

        std::stringstream sstream(data);
        {
            int tileNum;

            while(sstream >> tileNum)
                parsedLayer.push_back(tileNum);
        }

        if(parsedLayer.empty())
            return false;

        std::vector<sf::Vertex> vertices;
        vertices.reserve(parsedLayer.size() * 6);

        size_t index = 0;
        int columns = tileset->getSize().x / tileWidth;
        int rows = tileset->getSize().y / tileHeight;

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                const int tile_id = parsedLayer[index];
                ++index;

                if(tile_id)
                {
//  Vertex XY coords				
                    const float vX = static_cast<float>(x * tileWidth);
                    const float vY = static_cast<float>(y * tileHeight);
//  Left-top coords of the tile in texture grid
                    const int tile_num = tile_id - 1;
                    const int top = (tile_num >= columns) ? tile_num / columns : 0;
                    const int left = tile_num % columns;
                    const sf::Vector2f tex_coords(left * tileWidth, top * tileHeight);
//  First triangle
                    vertices.emplace_back(sf::Vector2f(vX, vY), sf::Color::White, tex_coords);
                    vertices.emplace_back(sf::Vector2f(vX + tileWidth, vY), sf::Color::White, sf::Vector2f(tex_coords.x + tileWidth, tex_coords.y));
                    vertices.emplace_back(sf::Vector2f(vX + tileWidth, vY + tileHeight), sf::Color::White, sf::Vector2f(tex_coords.x + tileWidth, tex_coords.y + tileHeight));
//  Second triangle
                    vertices.emplace_back(sf::Vector2f(vX, vY), sf::Color::White, tex_coords);
                    vertices.emplace_back(sf::Vector2f(vX + tileWidth, vY + tileHeight), sf::Color::White, sf::Vector2f(tex_coords.x + tileWidth, tex_coords.y + tileHeight));
                    vertices.emplace_back(sf::Vector2f(vX, vY + tileHeight), sf::Color::White, sf::Vector2f(tex_coords.x, tex_coords.y + tileHeight));
                }
            }

//  Unload to VBO
        auto& vbo = layers.emplace_back();

        if (!vertices.empty())
        {
            vbo.setUsage(sf::VertexBuffer::Usage::Static);
            vbo.setPrimitiveType(sf::PrimitiveType::Triangles);

            if (!vbo.create(vertices.size())) return false;
            if (!vbo.update(vertices.data())) return false;
        }

        layerNode = layerNode->next_sibling("layer");
    }

    if (auto objectGroupNode = mapNode->first_node("objectgroup"); objectGroupNode)
    {
        auto objectNode = objectGroupNode->first_node("object");

        while (objectNode)
        {
            Object object;
			auto attribute = objectNode->first_attribute();
            
            while (attribute)
            {
				if (strcmp(attribute->name(), "x")      == 0) { object.bounds.position.x = std::atof(attribute->value()); }
				if (strcmp(attribute->name(), "y")      == 0) { object.bounds.position.y = std::atof(attribute->value()); }
				if (strcmp(attribute->name(), "width")  == 0) { object.bounds.size.x     = std::atof(attribute->value()); }
				if (strcmp(attribute->name(), "height") == 0) { object.bounds.size.y     = std::atof(attribute->value()); }
				if (strcmp(attribute->name(), "name")   == 0)   object.name = attribute->value();

                attribute = attribute->next_attribute();
            }
            
            objects.push_back(object);
            objectNode = objectNode->next_sibling("object");
        }
    }

    return true;
}

Object Level::GetObject(std::string name)
{
    for (int i = 0; i < objects.size(); i++)
        if (objects[i].name == name)
            return objects[i];

    return {};
}

std::vector<Object> Level::GetObjects(std::string name)
{
	std::vector<Object> vec;
    for(int i = 0; i < objects.size(); i++)
        if(objects[i].name == name)
			vec.push_back(objects[i]);

	return vec;
}


std::vector<Object> Level::GetAllObjects()
{
    return objects;
};


sf::Vector2i Level::GetTileSize()
{
	return sf::Vector2i(tileWidth, tileHeight);
}

void Level::Draw(sf::RenderWindow &window)
{
	for(auto& layer : layers)
		window.draw(layer, tileset);
}

#endif // TILE_MAP_HPP
