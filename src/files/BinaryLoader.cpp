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


struct FileHeader
{
    uint32_t magic;
    uint32_t version;
};


struct FileCloser
{
    void operator()(FILE* f) const noexcept
    {
        if (f)
            fclose(f);
    }
};

using FileOpener = std::unique_ptr<FILE, FileCloser>;


struct TileMapData
{
    uint32_t mapWidth;
    uint32_t mapHeight;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t layerCount;

    char textureBG[texture_name_buffer_size + 1]; // +1 for '\0'; BG == background
    char textureFG[texture_name_buffer_size + 1]; // FG == foreground

    uint32_t chunkCountBG;
    uint32_t chunkCountFG;

    uint32_t vertexCountBG;
    uint32_t indexCountBG;
    uint32_t vertexCountFG;
    uint32_t indexCountFG;
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
    const FileHeader header = {.magic = get_binary_magic(), .version = get_binary_version()};

    if (!try_write_data<FileHeader>(&header, 1, fp))
        return false;

    // Tilemap data
    TileMapData tmData;
    memset(static_cast<void*>(&tmData), 0, sizeof(TileMapData));

    tmData.mapWidth   = static_cast<uint32_t>(data.mapSize.x);
    tmData.mapHeight  = static_cast<uint32_t>(data.mapSize.y);
    tmData.tileWidth  = static_cast<uint32_t>(data.tileSize.x);
    tmData.tileHeight = static_cast<uint32_t>(data.tileSize.y);
    tmData.layerCount = data.foreground.chunks.empty() ? 1 : 2;

    strncpy(tmData.textureBG,
            data.background.texture.data(),
            std::min(data.background.texture.size(), texture_name_buffer_size - 1));

    strncpy(tmData.textureFG,
            data.foreground.texture.data(),
            std::min(data.foreground.texture.size(), texture_name_buffer_size - 1));

    // Chunk data
    tmData.chunkCountBG = data.background.chunks.size();
    tmData.chunkCountFG = data.foreground.chunks.size();

    // Vertices + indices
    tmData.vertexCountBG = static_cast<uint32_t>(data.background.vertices.size());
    tmData.indexCountBG  = static_cast<uint32_t>(data.background.indices.size());
    tmData.vertexCountFG = static_cast<uint32_t>(data.foreground.vertices.size());
    tmData.indexCountFG  = static_cast<uint32_t>(data.foreground.indices.size());

    if (!try_write_data<TileMapData>(&tmData, 1, fp))
        return false;

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

    if (tmData.layerCount > 1)
    {
        if (!write_large_binary_data(data.foreground, fp))
            return false;
    }

    return true;
}


template <class T>
static bool try_read_data(T& buffer, size_t count, FILE* fp) noexcept
{
    const size_t read = fread(static_cast<void*>(&buffer), sizeof(T), count, fp);

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

    if (!try_read_data<FileHeader>(header, 1, fp))
        return {};

    if (header.magic != get_binary_magic())
        return {}; // Unknown format

    TileMapData tilemapData;

    if (!try_read_data<TileMapData>(tilemapData, 1, fp))
        return {};


    auto data = std::make_unique<BinaryLoader::Data>();


    return data;
}