#include <cassert>
#include <cstdio>
#include <cstring>

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

struct TilePlane
{
    uint32_t chunks;
    uint32_t vertices;
    uint32_t indices;
    uint32_t texture;
};


struct FileHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t mapWidth;
    uint32_t mapHeight;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t tilePlaneCount;
};



BinaryLoader::BinaryLoader(const std::string& filename) noexcept :
    m_filename(filename)
{
    assert(m_filename.find("\\/") == std::string::npos);

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
    header.magic = get_binary_magic();
    header.version = get_binary_version();

    header.mapWidth   = static_cast<uint32_t>(data.mapSize.x);
    header.mapHeight  = static_cast<uint32_t>(data.mapSize.y);
    header.tileWidth  = static_cast<uint32_t>(data.tileSize.x);
    header.tileHeight = static_cast<uint32_t>(data.tileSize.y);
    header.tilePlaneCount = data.foreground.chunks.empty() ? 1u : 2u;

    if (!try_write_data<FileHeader>(&header, 1, fp))
        return false;

    TilePlane plane;
    {// Background
        plane.chunks = static_cast<uint32_t>(data.background.chunks.size());
        plane.vertices = static_cast<uint32_t>(data.background.vertices.size());
        plane.indices = static_cast<uint32_t>(data.background.indices.size());
        plane.texture = static_cast<uint32_t>(data.background.texture.size());

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
    {// Foreground
        plane.chunks = static_cast<uint32_t>(data.foreground.chunks.size());
        plane.vertices = static_cast<uint32_t>(data.foreground.vertices.size());
        plane.indices = static_cast<uint32_t>(data.foreground.indices.size());
        plane.texture = static_cast<uint32_t>(data.foreground.texture.size());

        if (!try_write_data<TilePlane>(&plane, 1, fp))
            return false;

        if (!try_write_data<char>(data.foreground.texture.data(), data.foreground.texture.size(), fp))
            return false;

        if (!write_large_binary_data(data.foreground, fp))
            return false;
    }

    return true;
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

    binaryData->mapSize.x = header.mapWidth;
    binaryData->mapSize.y = header.mapHeight;
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
    {// Foreground
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

    return binaryData;
}