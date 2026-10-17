#include <algorithm>
#include <charconv>
#include <cstring>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"

#include "files/FileProvider.hpp"
#include "files/TmxLoader.hpp"


TmxLoader::TmxLoader() noexcept = default;
TmxLoader::~TmxLoader()         = default;


bool TmxLoader::loadFromFile(const std::string& filename) noexcept
{
    const auto filepath = FileProvider::findPathToFile(filename);

    if (!std::filesystem::exists(filepath))
        return false;

    auto                 document = std::make_unique<rapidxml::xml_document<char>>();
    rapidxml::file<char> xmlFile(filepath.string().c_str());
    document->parse<0>(xmlFile.data());
    const auto* mapNode = document->first_node("map");

    m_layers.clear();
    m_objects.clear();

    if (!loadTilesets(static_cast<const void*>(mapNode)))
        return false;

    if (!loadTileLayers(static_cast<const void*>(mapNode)))
        return false;

    if (!loadObjects(static_cast<const void*>(mapNode)))
        return false;

    return true;
}


const std::list<TmxLoader::Layer>& TmxLoader::getLayers() const noexcept
{
    return m_layers;
}


std::span<const TmxLoader::Object> TmxLoader::getObjects() const noexcept
{
    return m_objects;
}


ivec2s TmxLoader::getMapSize() const noexcept
{
    return m_mapSize;
}


ivec2s TmxLoader::getTileSize() const noexcept
{
    return m_tileSize;
}


bool TmxLoader::loadTilesets(const void* rootNode) noexcept
{
    const auto* mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);

    for (auto* tilesetNode = mapNode->first_node("tileset"); tilesetNode;
         tilesetNode       = tilesetNode->next_sibling("tile"
                                                       "se"
                                                       "t"))
    {
        auto* imageNode = tilesetNode->first_node("image");

        if (!imageNode)
            continue;

        auto* source = imageNode->first_attribute("source");

        if (!source)
            continue;

        const auto filename = std::filesystem::path(source->value()).filename().string();
        const auto filepath = FileProvider::findPathToFile(filename);

        auto alreadyExists = std::find_if(m_tilesets.begin(),
                                          m_tilesets.end(),
                                          [&filename](const Tileset& ts) { return ts.name == filename; });

        if (alreadyExists != m_tilesets.end())
            continue; // Already exists

        auto& tileset = m_tilesets.emplace_back();

        if (!tileset.image.loadFromFile(filepath))
            return false;

        tileset.name    = filename;
        auto* tileCount = tilesetNode->first_attribute("tilecount");
        auto* columns   = tilesetNode->first_attribute("columns");
        auto* firstGID  = tilesetNode->first_attribute("firstgid");

        tileset.tileCount = tileCount ? atoi(tileCount->value()) : 0;
        tileset.columns   = columns ? atoi(columns->value()) : 0;
        tileset.rows      = (!tileCount || !columns) ? 0 : tileset.tileCount / tileset.columns;
        tileset.firstGID  = firstGID ? atoi(firstGID->value()) : 0;
    }

    return (!m_tilesets.empty());
}


bool TmxLoader::loadTileLayers(const void* rootNode) noexcept
{
    const auto* mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);

    const auto* mapW  = mapNode->first_attribute("width");
    const auto* mapH  = mapNode->first_attribute("height");
    const auto* tileW = mapNode->first_attribute("tilewidth");
    const auto* tileH = mapNode->first_attribute("tileheight");

    const int32_t mapWidth   = mapW ? std::atoi(mapW->value()) : 0;
    const int32_t mapHeight  = mapH ? std::atoi(mapH->value()) : 0;
    const int32_t tileWidth  = tileW ? std::atoi(tileW->value()) : 0;
    const int32_t tileHeight = tileH ? std::atoi(tileH->value()) : 0;

    m_mapSize  = {mapWidth, mapHeight};
    m_tileSize = {tileWidth, tileHeight};

    auto* layerNode = mapNode->first_node("layer");

    while (layerNode)
    {
        auto* dataNode = layerNode->first_node("data");
        auto  tileNums = parseCSV(dataNode);

        if (tileNums.empty())
            return false;

        auto& layer = m_layers.emplace_back();

        if (auto* name = layerNode->first_attribute("name"); name)
            layer.name = std::string(name->value(), name->value_size());

        const auto    tileRange = std::minmax_element(tileNums.begin(), tileNums.end());
        const int32_t minTile   = *tileRange.first;
        const int32_t maxTile   = *tileRange.second;

        auto currentTileset = std::find_if(m_tilesets.begin(),
                                           m_tilesets.end(),
                                           [minTile, maxTile](const TmxLoader::Tileset& ts)
                                           {
                                               return minTile >= (ts.firstGID - 1) &&
                                                      maxTile <= (ts.firstGID + ts.tileCount);
                                           });

        if (currentTileset == m_tilesets.end())
            return false;

        layer.tileset = &*currentTileset;
        layer.tiles.swap(tileNums);

        layerNode = layerNode->next_sibling("layer");
    }

    return (!m_layers.empty());
}


bool TmxLoader::loadObjects(const void* rootNode) noexcept
{
    const auto* mapNode = static_cast<const rapidxml::xml_node<char>*>(rootNode);

    for (auto* objectGroupNode = mapNode->first_node("objectgroup"); objectGroupNode != nullptr;
         objectGroupNode       = objectGroupNode->next_sibling("objectgroup"))
    {
        for (auto* objectNode = objectGroupNode->first_node("object"); objectNode != nullptr;
             objectNode       = objectNode->next_sibling("object"))
        {
            auto& object = m_objects.emplace_back();

            for (auto attribute = objectNode->first_attribute(); attribute != nullptr;
                 attribute      = attribute->next_attribute())
            {
                if (strcmp(attribute->name(), "x") == 0)
                {
                    object.position.x = atoi(attribute->value());
                    continue;
                }
                if (strcmp(attribute->name(), "y") == 0)
                {
                    object.position.y = atoi(attribute->value());
                    continue;
                }
                if (strcmp(attribute->name(), "width") == 0)
                {
                    object.size.x = atoi(attribute->value());
                    continue;
                }
                if (strcmp(attribute->name(), "height") == 0)
                {
                    object.size.y = atoi(attribute->value());
                    continue;
                }
                if (strcmp(attribute->name(), "name") == 0)
                    object.name = {attribute->value(), attribute->value_size()};
                if (strcmp(attribute->name(), "class") == 0)
                    object.type = {attribute->value(), attribute->value_size()};
            }

            if (const auto* propertiesNode = objectNode->first_node("properties"); propertiesNode != nullptr)
            {
                for (auto* propertyNode = propertiesNode->first_node("property"); propertyNode != nullptr;
                     propertyNode       = propertyNode->next_sibling("property"))
                {
                    auto& property = object.properties.emplace_back();

                    std::string_view type;
                    std::string_view value;

                    for (auto* attribute = propertyNode->first_attribute(); attribute;
                         attribute       = attribute->next_attribute())
                    {
                        if (strcmp(attribute->name(), "name") == 0)
                            property.name = {attribute->value(), attribute->value_size()};

                        else if (strcmp(attribute->name(), "type") == 0)
                            type = {attribute->value(), attribute->value_size()};

                        else if (strcmp(attribute->name(), "value") == 0)
                            value = {attribute->value(), attribute->value_size()};
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
                        float result   = 0.f;
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

    return (!m_objects.empty());
}


std::vector<uint32_t> TmxLoader::parseCSV(const void* node) noexcept
{
    const auto*      dataNode = static_cast<const rapidxml::xml_node<char>*>(node);
    std::string_view csv(dataNode->value(), dataNode->value_size());

    const auto commaCount = std::count_if(csv.begin(), csv.end(), [](const char c) { return c == ','; });

    std::vector<uint32_t> result;
    result.reserve(commaCount + 1);

    const char* begin = csv.data();
    const char* end   = begin + csv.size();

    while (begin < end)
    {
        while (begin < end && *begin == ' ')
            ++begin;

        uint32_t value = 0;
        auto [p, ec]   = std::from_chars(begin, end, value);

        if (ec == std::errc())
        {
            result.push_back(value);
            begin = p;
        }
        else
            ++begin;

        while (begin < end && (*begin == ',' || *begin == ' '))
            ++begin;
    }

    return result;
}