#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <ranges>
#include <span>

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


TileMap::TileMap(const TmxLoader& loader) noexcept :
    Transform2D(),
    m_loader(&loader)
{
}


TileMap::~TileMap()
{
    cleanup();
}


bool TileMap::create() noexcept
{
    cleanup();

    if (!loadTextures())
        return false;

    loadGraphics();

    return m_isLoaded;
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


const TmxLoader& TileMap::getLoader() const noexcept
{
    return *m_loader;
}


template <class T>
auto try_get_gpu_data(std::vector<T>& buffer, uint32_t handle) -> bool
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


bool TileMap::saveToFile(const BinaryLoader& saver) noexcept
{
    if (!m_isLoaded)
        return false;

    BinaryLoader::Data data;

    data.mapSize  = m_loader->getMapSize();
    data.tileSize = m_loader->getTileSize();

    const auto& tilePlanes = m_loader->getLayers();

    const auto planeB = std::find_if(tilePlanes.begin(),
                                     tilePlanes.end(),
                                     [](const TmxLoader::Layer& plane) -> bool { return plane.name == "Background"; });

    const auto planeA = std::find_if(tilePlanes.begin(),
                                     tilePlanes.end(),
                                     [](const TmxLoader::Layer& plane) -> bool { return plane.name == "Foreground"; });

    data.background.texture = planeB->tileset->filename;
    data.foreground.texture = planeA->tileset->filename;

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
            fileChunks[i].offset = reinterpret_cast<uintptr_t>(foregroundChunks[i].indices);
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

    return saver.save(data);
}


bool TileMap::loadTextures() noexcept
{
    const auto& tilePlanes = m_loader->getLayers();

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

    m_background.texture = background->getHandle();

    Texture2D* foreground = Assets->get<Texture2D>(planeA->tileset->filename);

    if (!foreground)
        return false;

    m_foreground.texture = foreground->getHandle();

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

    const auto     mapSize  = m_loader->getMapSize();
    const auto     tileSize = m_loader->getTileSize();
    const uint32_t chunksX  = (mapSize.x + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;
    const uint32_t chunksY  = (mapSize.y + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;

    std::vector<TilePlane::Chunk> chunks(chunksX * chunksY, {nullptr, 0});

    for (uint32_t i = 0; i < chunksY; ++i)
    {
        for (uint32_t j = 0; j < chunksX; ++j)
        {
            const uint32_t cy = i * MAX_CHUNK_SIZE; // global chunk Y
            const uint32_t cx = j * MAX_CHUNK_SIZE; // global chunk X

            const uint32_t chunkHeight = std::min(MAX_CHUNK_SIZE, mapSize.y - cy);
            const uint32_t chunkWidth  = std::min(MAX_CHUNK_SIZE, mapSize.x - cx);

            auto* chunk = chunks.data() + i * chunksX + j;

            assert(chunk->count == 0);
            chunk->indices = (const void*)(indices.size() * sizeof(uint32_t));
            chunk->width   = chunkWidth;
            chunk->height  = chunkHeight;

            size_t tile = static_cast<size_t>(cy * mapSize.x + cx);

            for (int32_t y = 0; y < chunkHeight; ++y)
            {
                for (int32_t x = 0; x < chunkWidth; ++x)
                {
                    const uint32_t tileId = tiles[tile + x];

                    if (!tileId)
                        continue;

                    //  Positions
                    const float globalX = static_cast<float>((cx + x) * tileSize.x);
                    const float globalY = static_cast<float>((cy + y) * tileSize.y);

                    const vec2s leftBottom  = {globalX, globalY + tileSize.y};
                    const vec2s rightBottom = {globalX + tileSize.x, globalY + tileSize.y};
                    const vec2s rightTop    = {globalX + tileSize.x, globalY};
                    const vec2s leftTop     = {globalX, globalY};

                    //  UV
                    const int32_t tileNum = tileId - tileset->firstGID;

                    const int32_t vY = (tileNum >= tileset->columns) ? (tileNum / tileset->columns) : 0;
                    const int32_t vX = tileNum % tileset->columns;

                    const int32_t offsetX = vX * tileSize.x;
                    const int32_t offsetY = vY * tileSize.y;

                    const float left   = offsetX * ratio.x;
                    const float top    = offsetY * ratio.y;
                    const float right  = (offsetX + tileSize.x) * ratio.x;
                    const float bottom = (offsetY + tileSize.y) * ratio.y;

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

                tile += mapSize.x;
            }
        }
    }

    plane.chunks.swap(chunks);
    plane.width  = chunksX;
    plane.height = chunksY;
}


void TileMap::loadGraphics() noexcept
{
    auto glResources = Assets->getResourceManager();

    const auto& layers  = m_loader->getLayers();
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

    const auto tileSize = m_loader->getTileSize();

    const int32_t chunkPixelWidth  = MAX_CHUNK_SIZE * tileSize.x;
    const int32_t chunkPixelHeight = MAX_CHUNK_SIZE * tileSize.y;

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