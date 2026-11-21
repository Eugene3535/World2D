#include <cassert>
#include <cstdio>
#include <cstring>
#include <span>

#include "files/BinaryLoader.hpp"
#include "files/FileProvider.hpp"


static constexpr size_t texture_name_buffer_size = 32;


static constexpr uint32_t get_binary_magic() noexcept
{
    return 0x56424631; // 'V' 'B' 'F' '1' (Voxel Binary Format 1), 'VBF1';
}

static constexpr uint32_t get_binary_version() noexcept
{
    return 1; // Версия формата
}


struct FileCloser
{
    void operator()(FILE* f) const noexcept
    {
        if (f)
            fclose(f);
    }
};

using FileOpener = std::unique_ptr<FILE, FileCloser>;


struct FileHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t mapWidth;
    uint32_t mapHeight;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t tilePlaneCount;
    uint32_t objects;
};


struct TilePlane
{
    uint32_t chunks;
    uint32_t vertices;
    uint32_t indices;
    uint32_t texture;
};


struct ObjectEntry
{
    uint32_t name;
    uint32_t type;
    ivec2s   position;
    ivec2s   size;
    uint32_t properties;
};


struct PropertyEntry
{
    enum Type : uint32_t
    {
        Undefined = 0,
        Boolean,
        Character,
        Integer,
        Floating
    } type;

    uint32_t name;
    uint32_t size;
};


BinaryLoader::BinaryLoader(const std::string& filename) noexcept :
    m_filename(filename)
{
    assert(m_filename.find_first_of("\\/") == std::string::npos);

    if (!m_filename.ends_with(".bin"))
    {
        if (const size_t dot = m_filename.rfind('.'); dot != std::string::npos)
            m_filename.replace(dot + 1, m_filename.size() - dot - 1, "bin");
    }
}


BinaryLoader::~BinaryLoader() = default;


template <class T>
static bool try_write_data(const T* data, size_t count, FILE* fp) noexcept
{
    const size_t written = fwrite(static_cast<const void*>(data), sizeof(T), count, fp);

    return (written == count);
}


bool BinaryLoader::save(const BinaryLoader::Data& data) const noexcept
{
    namespace fs = std::filesystem;
    std::error_code ec;

    auto filepath = FileProvider::findPathToFile(m_filename);

    if (fs::exists(filepath, ec))
    {
        fs::remove(filepath, ec);
        ec.clear();
    }
    else
    {
        const fs::path saveFolder = FileProvider::getAppPath() / "res/save";

        if (fs::create_directories(saveFolder, ec); ec)
            return false;

        filepath = saveFolder / m_filename;
        ec.clear();
    }

    FileOpener fileOpener(fopen(filepath.string().c_str(), "wb"));

    if (!fileOpener)
        return false;

    auto* fp = fileOpener.get();

    // Header
    FileHeader header;
    memset(static_cast<void*>(&header), 0, sizeof(FileHeader));
    header.magic   = get_binary_magic();
    header.version = get_binary_version();

    header.mapWidth       = static_cast<uint32_t>(data.mapSize.x);
    header.mapHeight      = static_cast<uint32_t>(data.mapSize.y);
    header.tileWidth      = static_cast<uint32_t>(data.tileSize.x);
    header.tileHeight     = static_cast<uint32_t>(data.tileSize.y);
    header.tilePlaneCount = data.foreground.chunks.empty() ? 1u : 2u;
    header.objects        = static_cast<uint32_t>(data.objects.size());

    if (!try_write_data<FileHeader>(&header, 1, fp))
        return false;

    TilePlane plane;
    { // Background
        plane.chunks   = static_cast<uint32_t>(data.background.chunks.size());
        plane.vertices = static_cast<uint32_t>(data.background.vertices.size());
        plane.indices  = static_cast<uint32_t>(data.background.indices.size());
        plane.texture  = static_cast<uint32_t>(data.background.texture.size());

        if (!try_write_data<TilePlane>(&plane, 1, fp))
            return false;

        if (!try_write_data<char>(data.background.texture.data(), data.background.texture.size(), fp))
            return false;
    }

    auto write_large_binary_data = [](const Data::TilePlane& plane, FILE* fp) -> bool
    {
        const auto& chunks   = plane.chunks;
        const auto& vertices = plane.vertices;
        const auto& indices  = plane.indices;

        if (!try_write_data<Data::TilePlane::Chunk>(chunks.data(), chunks.size(), fp))
            return false;

        if (!try_write_data<float>(vertices.data(), vertices.size(), fp))
            return false;

        if (!try_write_data<uint32_t>(indices.data(), indices.size(), fp))
            return false;

        return true;
    };

    // Large binary data (chunks, vertices, indices)
    if (!write_large_binary_data(data.background, fp))
        return false;

    if (!data.foreground.chunks.empty())
    { // Foreground
        plane.chunks   = static_cast<uint32_t>(data.foreground.chunks.size());
        plane.vertices = static_cast<uint32_t>(data.foreground.vertices.size());
        plane.indices  = static_cast<uint32_t>(data.foreground.indices.size());
        plane.texture  = static_cast<uint32_t>(data.foreground.texture.size());

        if (!try_write_data<TilePlane>(&plane, 1, fp))
            return false;

        if (!try_write_data<char>(data.foreground.texture.data(), data.foreground.texture.size(), fp))
            return false;

        if (!write_large_binary_data(data.foreground, fp))
            return false;
    }

    std::span<const TmxObject> objects        = data.objects;
    size_t                     writtenObjects = 0;

    for (const auto& object : objects)
    {
        ObjectEntry objEntry;
        objEntry.name       = static_cast<uint32_t>(object.name.size());
        objEntry.type       = static_cast<uint32_t>(object.type.size());
        objEntry.position   = object.position;
        objEntry.size       = object.size;
        objEntry.properties = static_cast<uint32_t>(object.properties.size());

        if (!try_write_data<ObjectEntry>(&objEntry, 1, fp))
            continue;

        if (!try_write_data<char>(object.name.data(), object.name.size(), fp))
            continue;

        if (objEntry.type > 0) // object may not have a type
            if (!try_write_data<char>(object.type.data(), object.type.size(), fp))
                continue;

        // Properties
        size_t writtenProperties = 0;

        struct
        {
            const bool*        b;
            const int32_t*     i;
            const float*       f;
            const std::string* s;
        } cache;

        for (const auto& property : object.properties)
        {
            PropertyEntry propEntry{.type = PropertyEntry::Undefined, .name = 0, .size = 0};
            propEntry.name = static_cast<uint32_t>(property.name.size());

            if (const auto* b = std::get_if<bool>(&property.value))
            {
                propEntry.type = PropertyEntry::Boolean;
                propEntry.size = 1;
                cache.b        = b;
            }
            else if (const auto* i = std::get_if<int>(&property.value))
            {
                propEntry.type = PropertyEntry::Integer;
                propEntry.size = 1;
                cache.i        = i;
            }
            else if (const auto* f = std::get_if<float>(&property.value))
            {
                propEntry.type = PropertyEntry::Floating;
                propEntry.size = 1;
                cache.f        = f;
            }
            else if (const auto* s = std::get_if<std::string>(&property.value))
            {
                propEntry.type = PropertyEntry::Character;
                propEntry.size = static_cast<uint32_t>(s->size());
                cache.s        = s;
            }

            if (propEntry.type == PropertyEntry::Undefined)
                continue;

            if (!try_write_data<PropertyEntry>(&propEntry, 1, fp))
                continue;

            if (!try_write_data<char>(property.name.data(), property.name.size(), fp))
                continue;

            bool result = false;

            switch (propEntry.type)
            {
                case PropertyEntry::Boolean:
                    result = try_write_data<bool>(cache.b, 1, fp);
                    break;

                case PropertyEntry::Integer:
                    result = try_write_data<int>(cache.i, 1, fp);
                    break;

                case PropertyEntry::Floating:
                    result = try_write_data<float>(cache.f, 1, fp);
                    break;

                case PropertyEntry::Character:
                    result = try_write_data<char>(cache.s->data(), cache.s->size(), fp);
                    break;

                default:
                    break;
            }

            if (result)
                ++writtenProperties;
        }

        if (writtenProperties == object.properties.size())
            ++writtenObjects;
    }

    return (writtenObjects == objects.size());
}


template <class T>
static bool try_read_data(T* buffer, size_t count, FILE* fp) noexcept
{
    const size_t read = fread(static_cast<void*>(buffer), sizeof(T), count, fp);

    if (feof(fp))
        printf("Error: EOF.\n");
    else if (ferror(fp))
        printf("File reading error.\n");

    return (read == count);
}


std::unique_ptr<BinaryLoader::Data> BinaryLoader::load() const noexcept
{
    namespace fs = std::filesystem;
    std::error_code ec;

    const auto filepath = FileProvider::findPathToFile(m_filename);

    if (!fs::exists(filepath, ec))
        return {};

    FileOpener fileOpener(fopen(filepath.string().c_str(), "rb"));

    if (!fileOpener)
        return {};

    auto* fp = fileOpener.get();

    FileHeader header;

    if (!try_read_data<FileHeader>(&header, 1, fp))
        return {};

    if (header.magic != get_binary_magic())
        return {}; // Unknown format

    auto binaryData = std::make_unique<BinaryLoader::Data>();

    binaryData->mapSize.x  = header.mapWidth;
    binaryData->mapSize.y  = header.mapHeight;
    binaryData->tileSize.x = header.tileWidth;
    binaryData->tileSize.y = header.tileHeight;

    TilePlane plane;

    if (!try_read_data<TilePlane>(&plane, 1, fp))
        return {};

    auto& background = binaryData->background;
    background.texture.resize(plane.texture);
    background.chunks.resize(plane.chunks);
    background.vertices.resize(plane.vertices);
    background.indices.resize(plane.indices);

    if (!try_read_data<char>(background.texture.data(), background.texture.size(), fp))
        return {};

    auto read_large_binary_data = [](Data::TilePlane& plane, FILE* fp) -> bool
    {
        auto& chunks   = plane.chunks;
        auto& vertices = plane.vertices;
        auto& indices  = plane.indices;

        if (!try_read_data<Data::TilePlane::Chunk>(chunks.data(), chunks.size(), fp))
            return false;

        if (!try_read_data<float>(vertices.data(), vertices.size(), fp))
            return false;

        if (!try_read_data<uint32_t>(indices.data(), indices.size(), fp))
            return false;

        return true;
    };

    // Large binary data (chunks, vertices, indices)
    if (!read_large_binary_data(binaryData->background, fp))
        return {};

    if (header.tilePlaneCount > 0)
    { // Foreground
        if (!try_read_data<TilePlane>(&plane, 1, fp))
            return {};

        auto& foreground = binaryData->foreground;
        foreground.texture.resize(plane.texture);
        foreground.chunks.resize(plane.chunks);
        foreground.vertices.resize(plane.vertices);
        foreground.indices.resize(plane.indices);

        if (!try_read_data<char>(foreground.texture.data(), foreground.texture.size(), fp))
            return {};

        if (!read_large_binary_data(foreground, fp))
            return {};
    }

    // Game objects
    if (header.objects == 0)
        return binaryData;

    binaryData->objects.resize(static_cast<uint32_t>(header.objects));
    std::span<TmxObject> objects     = binaryData->objects;
    size_t               readObjects = 0;

    for (auto& object : objects)
    {
        ObjectEntry objEntry;

        if (!try_read_data<ObjectEntry>(&objEntry, 1, fp))
            continue;

        object.name.resize(static_cast<size_t>(objEntry.name));
        object.type.resize(static_cast<size_t>(objEntry.type));
        object.position = objEntry.position;
        object.size     = objEntry.size;

        if (!try_read_data<char>(object.name.data(), object.name.size(), fp))
            continue;

        if (objEntry.type > 0)
            if (!try_read_data<char>(object.type.data(), object.type.size(), fp))
                continue;

        size_t readProperties = 0;

        for (uint32_t i = 0; i < objEntry.properties; ++i)
        {
            PropertyEntry propEntry;

            if (!try_read_data<PropertyEntry>(&propEntry, 1, fp))
                continue;

            auto& property = object.properties.emplace_back();
            property.name.resize(static_cast<size_t>(propEntry.name));

            if (!try_read_data<char>(property.name.data(), property.name.size(), fp))
                continue;

            bool result = false;

            switch (propEntry.type)
            {
                case PropertyEntry::Boolean:
                {
                    bool value;
                    result         = try_read_data<bool>(&value, 1, fp);
                    property.value = value;
                }
                break;

                case PropertyEntry::Integer:
                {
                    int32_t value;
                    result         = try_read_data<int32_t>(&value, 1, fp);
                    property.value = value;
                }
                break;

                case PropertyEntry::Floating:
                {
                    float value;
                    result         = try_read_data<float>(&value, 1, fp);
                    property.value = value;
                }
                break;

                case PropertyEntry::Character:
                {
                    std::string value;
                    value.resize(static_cast<size_t>(propEntry.size));
                    result         = try_read_data<char>(value.data(), value.size(), fp);
                    property.value = std::move(value);
                }
                break;

                default:
                    break;
            }

            if (result)
                ++readProperties;
        }

        if (readProperties == object.properties.size())
            ++readObjects;
    }

    assert(readObjects == objects.size());

    return binaryData;
}