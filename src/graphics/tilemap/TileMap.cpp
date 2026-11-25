#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <ranges>

#include <glad/glad.h>

#include "files/BinaryLoader.hpp"
#include "files/Shader.hpp"
#include "files/TmxLoader.hpp"
#include "game/assets/AssetManager.hpp"
#include "graphics/resources/GlResourceManager.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/tilemap/TileMap.hpp"
#include "graphics/vao/VertexBufferLayout.hpp"

#define MAX_CHUNK_SIZE 64u


struct BufferData
{
    std::vector<float>    vertices;
    std::vector<uint32_t> indices;
    const void*           tileLayer{nullptr};
};


TileMap::TileMap(const std::string& locationName) noexcept :
    Transform2D(),
    m_locationName(locationName)
{
    assert(!locationName.empty());
    assert(m_locationName.find_first_of("\\/") == std::string::npos);

    if (const size_t dot = m_locationName.rfind('.'); dot != std::string::npos)
        m_locationName = m_locationName.substr(0, dot); // erase extension
}


TileMap::~TileMap()
{
    cleanup();
}


bool TileMap::open() noexcept
{
    if (m_isLoaded)
        return true;

    namespace fs = std::filesystem;
    std::error_code ec;

    std::string fileName = m_locationName + ".bin";
    auto binFile = FileProvider::findPathToFile(fileName);

    if (fs::exists(binFile, ec))
    {
        auto binaryLoader = std::make_unique<BinaryLoader>(fileName);

        if (openFromFile(binaryLoader.get()))
            return true;
    }

    if (const size_t dot = fileName.rfind('.'); dot != std::string::npos)
        fileName.replace(dot + 1, fileName.size() - dot - 1, "tmx");

    TmxLoader tmxLoader;

    if (!tmxLoader.loadFromFile(fileName))
        return false;

    m_mapSize  = tmxLoader.getMapSize();
    m_tileSize = tmxLoader.getTileSize();

    if (!loadTextures(tmxLoader))
        return false;

    loadGraphics(tmxLoader);

    if (auto objects = tmxLoader.getObjects(); !objects.empty())
    {
        m_objects.reserve(objects.size());
        m_objects.assign(objects.begin(), objects.end());
    }

    return m_isLoaded;
}


bool TileMap::save() noexcept
{
    if (!m_isLoaded)
        return false;

    auto binaryLoader = std::make_unique<BinaryLoader>(m_locationName + ".bin");

    return saveToFile(binaryLoader.get());
}


void TileMap::drawBackground(const ivec4s& viewport) noexcept
{
    prepareChunks(m_background, viewport);
    draw(m_background, viewport);
}


void TileMap::drawForeground(const ivec4s& viewport) noexcept
{
    prepareChunks(m_foreground, viewport);
    draw(m_foreground, viewport);
}


ivec2s TileMap::getMapSize() const noexcept
{
    return m_mapSize;
}


ivec2s TileMap::getTileSize() const noexcept
{
    return m_tileSize;
}


std::span<struct TmxObject> TileMap::getObjects() noexcept
{
    return m_objects;
}


bool TileMap::openFromFile(const BinaryLoader* loader) noexcept
{
    const auto binaryData = loader->load();

    if (!binaryData)
        return false;

    m_mapSize  = binaryData->mapSize;
    m_tileSize = binaryData->tileSize;

    { // Textures
        const Texture2D* background = Assets->get<Texture2D>(binaryData->background.texture);

        if (!background)
            return false;

        m_background.texture     = background->getHandle();
        m_background.textureName = binaryData->background.texture;

        const Texture2D* foreground = Assets->get<Texture2D>(binaryData->foreground.texture);

        if (!foreground)
        {
            m_foreground.texture     = background->getHandle();
            m_foreground.textureName = binaryData->foreground.texture;
        }
    }

    auto glResources = Assets->getResourceManager();
    auto buffers     = glResources->getHandles(GlResourceManager::GLBuffer, 4);

    m_background.VBO = buffers[0];
    m_background.EBO = buffers[1];
    m_foreground.VBO = buffers[2];
    m_foreground.EBO = buffers[3];

    m_background.chunks.resize(binaryData->background.chunks.size());

    for (size_t i = 0; i < m_background.chunks.size(); ++i)
    {
        auto& binChunk = binaryData->background.chunks[i];
        auto& chunk    = m_background.chunks[i];

        chunk.indices = reinterpret_cast<const void*>(binChunk.offset);
        chunk.count   = binChunk.count;
        chunk.width   = binChunk.width;
        chunk.height  = binChunk.height;
    }

    { // Background tile plane
        std::span<const float>    vertices = binaryData->background.vertices;
        std::span<const uint32_t> indices  = binaryData->background.indices;

        glNamedBufferStorage(m_background.VBO,
                             static_cast<GLsizeiptr>(vertices.size_bytes()),
                             vertices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

        glNamedBufferStorage(m_background.EBO,
                             static_cast<GLsizeiptr>(indices.size_bytes()),
                             indices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
    }

    if (!binaryData->foreground.chunks.empty()) // Foreground tile plane
    {
        std::span<const float>    vertices = binaryData->foreground.vertices;
        std::span<const uint32_t> indices  = binaryData->foreground.indices;

        glNamedBufferStorage(m_foreground.VBO,
                             static_cast<GLsizeiptr>(vertices.size_bytes()),
                             vertices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

        glNamedBufferStorage(m_foreground.EBO,
                             static_cast<GLsizeiptr>(indices.size_bytes()),
                             indices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

        m_foreground.chunks.resize(binaryData->foreground.chunks.size());

        for (size_t i = 0; i < m_foreground.chunks.size(); ++i)
        {
            auto& binChunk = binaryData->foreground.chunks[i];
            auto& chunk    = m_foreground.chunks[i];

            chunk.indices = reinterpret_cast<const void*>(binChunk.offset);
            chunk.count   = binChunk.count;
            chunk.width   = binChunk.width;
            chunk.height  = binChunk.height;
        }
    }

    auto vertexArrays = glResources->getHandles(GlResourceManager::GLVertexArray, 2);

    const std::array<VertexBufferLayout::Attribute, 1> attributes = {VertexBufferLayout::Attribute::Float4};
    VertexBufferLayout                                 layout(attributes);

    layout.createVertexInputState(vertexArrays[0], m_background.VBO);
    glVertexArrayElementBuffer(vertexArrays[0], m_background.EBO);

    if (!binaryData->foreground.chunks.empty())
    {
        layout.createVertexInputState(vertexArrays[1], m_foreground.VBO);
        glVertexArrayElementBuffer(vertexArrays[1], m_foreground.EBO);
    }

    m_background.VAO = vertexArrays[0];
    m_foreground.VAO = vertexArrays[1];

    std::span<TmxObject> objects = binaryData->objects;

    if (!objects.empty())
    {
        m_objects.reserve(objects.size());
        m_objects.assign(objects.begin(), objects.end());
    }

    m_isLoaded = true;

    return m_isLoaded;
}


template <class T>
static bool try_get_gpu_data(std::vector<T>& buffer, uint32_t handle)
{
    GLint bufferSize;
    glGetNamedBufferParameteriv(handle, GL_BUFFER_SIZE, &bufferSize);

    void* mapped = glMapNamedBufferRange(handle, 0, bufferSize, GL_MAP_READ_BIT);

    if (!mapped)
        return false;

    buffer.resize(bufferSize / (sizeof(T)));
    memcpy(buffer.data(), mapped, bufferSize);

    glUnmapNamedBuffer(handle);

    return true;
}


bool TileMap::saveToFile(const BinaryLoader* saver) noexcept
{
    if (!m_isLoaded)
        return false;

    BinaryLoader::Data data;

    data.mapSize            = m_mapSize;
    data.tileSize           = m_tileSize;
    data.background.texture = m_background.textureName;
    data.foreground.texture = m_foreground.textureName;

    std::vector<BinaryLoader::Data::TilePlane::Chunk> fileChunks;
    std::vector<float>                                vertices;
    std::vector<uint32_t>                             indices;

    fileChunks.resize(m_background.chunks.size());
    const auto& backgroundChunks = m_background.chunks;

    for (size_t i = 0; i < fileChunks.size(); ++i)
    {
        auto& chunk  = fileChunks[i];
        chunk.offset = reinterpret_cast<uintptr_t>(backgroundChunks[i].indices);
        chunk.count  = backgroundChunks[i].count;
        chunk.width  = backgroundChunks[i].width;
        chunk.height = backgroundChunks[i].height;
    }

    if (!try_get_gpu_data<float>(vertices, m_background.VBO))
        return false;

    if (!try_get_gpu_data<uint32_t>(indices, m_background.EBO))
        return false;

    data.background.chunks.swap(fileChunks);
    data.background.vertices.swap(vertices);
    data.background.indices.swap(indices);

    if (!m_foreground.chunks.empty())
    {
        fileChunks.resize(m_foreground.chunks.size());
        const auto& foregroundChunks = m_foreground.chunks;

        for (size_t i = 0; i < fileChunks.size(); ++i)
        {
            fileChunks[i].offset = reinterpret_cast<uint64_t>(foregroundChunks[i].indices);
            fileChunks[i].count  = foregroundChunks[i].count;
        }

        if (!try_get_gpu_data<float>(vertices, m_foreground.VBO))
            return false;

        if (!try_get_gpu_data<uint32_t>(indices, m_foreground.EBO))
            return false;

        data.foreground.chunks.swap(fileChunks);
        data.foreground.vertices.swap(vertices);
        data.foreground.indices.swap(indices);
    }

    if (!m_objects.empty())
    {
        data.objects.reserve(m_objects.size());
        data.objects.assign(m_objects.begin(), m_objects.end());
    }

    return saver->save(data);
}


bool TileMap::loadTextures(const TmxLoader& loader) noexcept
{
    const auto& tilePlanes = loader.getLayers();

    auto planeB = std::find_if(tilePlanes.begin(),
                               tilePlanes.end(),
                               [](const TmxLoader::Layer& plane) -> bool { return plane.name == "Background"; });

    auto planeA = std::find_if(tilePlanes.begin(),
                               tilePlanes.end(),
                               [](const TmxLoader::Layer& plane) -> bool { return plane.name == "Foreground"; });

    if (planeB == tilePlanes.end()) // At least one tile layer must exist.
        return false;

    Texture2D* background = Assets->get<Texture2D>(planeB->tileset->filename);

    if (!background)
        return false;

    m_background.texture     = background->getHandle();
    m_background.textureName = planeB->tileset->filename;

    Texture2D* foreground = Assets->get<Texture2D>(planeA->tileset->filename);

    if (!foreground)
        return false;

    m_foreground.texture     = foreground->getHandle();
    m_background.textureName = planeA->tileset->filename;

    return true;
}


void TileMap::createBufferData(TilePlane& plane, void* bufferData) noexcept
{
    auto*       data  = static_cast<BufferData*>(bufferData);
    const auto* layer = static_cast<const TmxLoader::Layer*>(data->tileLayer);

    std::span<const uint32_t> tiles   = layer->tiles;
    const auto*               tileset = layer->tileset;

    auto& vertices = data->vertices;
    auto& indices  = data->indices;

    const uint32_t tileCount = std::ranges::count_if(tiles, [](uint32_t n) { return n != 0; });

    vertices.reserve(static_cast<size_t>(tileCount * 4));
    indices.reserve(static_cast<size_t>(tileCount * 6));

    const vec2s ratio = {1.f / static_cast<float>(tileset->width), 1.f / static_cast<float>(tileset->height)};

    const uint32_t chunksX = (m_mapSize.x + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;
    const uint32_t chunksY = (m_mapSize.y + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;

    std::vector<TilePlane::Chunk> chunks(chunksX * chunksY, {nullptr, 0});

    for (uint32_t i = 0; i < chunksY; ++i)
    {
        for (uint32_t j = 0; j < chunksX; ++j)
        {
            const uint32_t cy = i * MAX_CHUNK_SIZE; // global chunk Y
            const uint32_t cx = j * MAX_CHUNK_SIZE; // global chunk X

            const uint32_t chunkHeight = std::min(MAX_CHUNK_SIZE, m_mapSize.y - cy);
            const uint32_t chunkWidth  = std::min(MAX_CHUNK_SIZE, m_mapSize.x - cx);

            auto* chunk = chunks.data() + i * chunksX + j;

            assert(chunk->count == 0);
            chunk->indices = (const void*)(indices.size() * sizeof(uint32_t));
            chunk->width   = chunkWidth;
            chunk->height  = chunkHeight;

            size_t tile = static_cast<size_t>(cy * m_mapSize.x + cx);

            for (int32_t y = 0; y < chunkHeight; ++y)
            {
                for (int32_t x = 0; x < chunkWidth; ++x)
                {
                    const uint32_t tileId = tiles[tile + x];

                    if (!tileId)
                        continue;

                    //  Positions
                    const float globalX = static_cast<float>((cx + x) * m_tileSize.x);
                    const float globalY = static_cast<float>((cy + y) * m_tileSize.y);

                    const vec2s leftBottom  = {globalX, globalY + m_tileSize.y};
                    const vec2s rightBottom = {globalX + m_tileSize.x, globalY + m_tileSize.y};
                    const vec2s rightTop    = {globalX + m_tileSize.x, globalY};
                    const vec2s leftTop     = {globalX, globalY};

                    //  UV
                    const int32_t tileNum = tileId - tileset->firstGID;

                    const int32_t vY = (tileNum >= tileset->columns) ? (tileNum / tileset->columns) : 0;
                    const int32_t vX = tileNum % tileset->columns;

                    const int32_t offsetX = vX * m_tileSize.x;
                    const int32_t offsetY = vY * m_tileSize.y;

                    const float left   = offsetX * ratio.x;
                    const float top    = offsetY * ratio.y;
                    const float right  = (offsetX + m_tileSize.x) * ratio.x;
                    const float bottom = (offsetY + m_tileSize.y) * ratio.y;

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

                    const uint32_t vertexIndex = (vertices.size() >> 2) - 4;

                    indices.push_back(vertexIndex);
                    indices.push_back(vertexIndex + 1u);
                    indices.push_back(vertexIndex + 2u);

                    indices.push_back(vertexIndex);
                    indices.push_back(vertexIndex + 2u);
                    indices.push_back(vertexIndex + 3u);

                    chunk->count += 6;
                }

                tile += m_mapSize.x;
            }
        }
    }

    plane.chunks.swap(chunks);
    plane.width  = chunksX;
    plane.height = chunksY;
}


void TileMap::loadGraphics(const TmxLoader& loader) noexcept
{
    auto glResources = Assets->getResourceManager();

    const auto& layers  = loader.getLayers();
    auto        buffers = glResources->getHandles(GlResourceManager::GLBuffer, 4);

    m_background.VBO = buffers[0];
    m_background.EBO = buffers[1];
    m_foreground.VBO = buffers[2];
    m_foreground.EBO = buffers[3];

    { // Background tile plane
        BufferData data;
        data.tileLayer = static_cast<const void*>(&layers.front());
        createBufferData(m_background, static_cast<void*>(&data));

        std::span<const float>    vertices = data.vertices;
        std::span<const uint32_t> indices  = data.indices;

        glNamedBufferStorage(m_background.VBO,
                             static_cast<GLsizeiptr>(vertices.size_bytes()),
                             vertices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

        glNamedBufferStorage(m_background.EBO,
                             static_cast<GLsizeiptr>(indices.size_bytes()),
                             indices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
    }

    if (layers.size() > 1) // Foreground tile plane
    {
        BufferData data;
        data.tileLayer = static_cast<const void*>(&layers.back());
        createBufferData(m_foreground, static_cast<void*>(&data));

        std::span<const float>    vertices = data.vertices;
        std::span<const uint32_t> indices  = data.indices;

        glNamedBufferStorage(m_foreground.VBO,
                             static_cast<GLsizeiptr>(vertices.size_bytes()),
                             vertices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

        glNamedBufferStorage(m_foreground.EBO,
                             static_cast<GLsizeiptr>(indices.size_bytes()),
                             indices.data(),
                             GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
    }

    auto vertexArrays = glResources->getHandles(GlResourceManager::GLVertexArray, 2);

    const std::array<VertexBufferLayout::Attribute, 1> attributes = {VertexBufferLayout::Attribute::Float4};
    VertexBufferLayout                                 layout(attributes);

    layout.createVertexInputState(vertexArrays[0], m_background.VBO);
    glVertexArrayElementBuffer(vertexArrays[0], m_background.EBO);

    if (layers.size() > 1)
    {
        layout.createVertexInputState(vertexArrays[1], m_foreground.VBO);
        glVertexArrayElementBuffer(vertexArrays[1], m_foreground.EBO);
    }

    m_background.VAO = vertexArrays[0];
    m_foreground.VAO = vertexArrays[1];

    m_isLoaded = true;
}


void TileMap::cleanup() noexcept
{
    if (!m_isLoaded)
        return;

    auto glResources = Assets->getResourceManager();

    const std::array<GLuint, 2> vertexArrays = {m_background.VAO, m_foreground.VAO};
    glResources->destroyHandles(GlResourceManager::GLVertexArray, vertexArrays);

    const std::array<GLuint, 4> buffers = {m_background.VBO, m_background.EBO, m_foreground.VBO, m_foreground.EBO};
    glResources->destroyHandles(GlResourceManager::GLBuffer, buffers);

    const std::array<GLuint, 2> textures = {m_background.texture, m_foreground.texture};
    glResources->destroyHandles(GlResourceManager::GLTexture2D, textures);

    memset(&m_background, 0, sizeof(m_background));
    memset(&m_foreground, 0, sizeof(m_foreground));

    m_isLoaded = false;
}


void TileMap::prepareChunks(TileMap::TilePlane& plane, const ivec4s& extents) noexcept
{
    const auto& chunks = plane.chunks;

    if (chunks.empty())
        return;

    auto& visibleChunks = plane.visibleChunks;
    visibleChunks.clear();

    if (chunks.size() == 1)
    {
        const auto& chunk = chunks[0];

        if (chunk.count)
            visibleChunks.push_back(chunk);

        return;
    }

    const int32_t chunkPixelWidth  = MAX_CHUNK_SIZE * m_tileSize.x;
    const int32_t chunkPixelHeight = MAX_CHUNK_SIZE * m_tileSize.y;

    int32_t left   = extents.x / chunkPixelWidth;
    int32_t top    = extents.y / chunkPixelHeight;
    int32_t right  = extents.z / chunkPixelWidth;
    int32_t bottom = extents.w / chunkPixelHeight;

    if ((left == top) && (left == right) && (left == bottom))
    {
        const auto& chunk = chunks[static_cast<size_t>(left)];

        if (chunk.count)
            visibleChunks.push_back(chunk);

        return;
    }

    for (int32_t y = top; y <= bottom; ++y)
    {
        for (int32_t x = left; x <= right; ++x)
        {
            const size_t index = y * plane.width + x;

            if (index < chunks.size())
                visibleChunks.push_back(chunks[index]);
        }
    }

    size_t index = 0;

    while (index < visibleChunks.size())
    {
        bool shouldRemove = false;

        if (visibleChunks[index].count == 0)
        {
            shouldRemove = true;
        }
        else
        {
            for (size_t j = 0; j < index; ++j)
            {
                if (visibleChunks[j].indices == visibleChunks[index].indices)
                {
                    shouldRemove = true;
                    break;
                }
            }
        }

        if (shouldRemove)
        {
            visibleChunks[index] = visibleChunks.back();
            visibleChunks.pop_back();
        }
        else
        {
            ++index;
        }
    }
}


void TileMap::draw(const TileMap::TilePlane& plane, const ivec4s& viewport) const noexcept
{
    if (plane.visibleChunks.empty())
        return;

    glBindTextureUnit(0, plane.texture);
    glBindVertexArray(plane.VAO);

    for (const auto& chunk : plane.visibleChunks)
        glDrawElements(GL_TRIANGLES, chunk.count, GL_UNSIGNED_INT, chunk.indices);

    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
}