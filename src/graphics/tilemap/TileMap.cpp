#include <array>
#include <algorithm>
#include <cstring>
#include <span>

#include <glad/glad.h>

#include "files/Shader.hpp"
#include "files/TmxLoader.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/vao/VertexBufferLayout.hpp"
#include "graphics/resources/GlResourceManager.hpp"
#include "graphics/tilemap/TileMap.hpp"



TileMap::TileMap(const TmxLoader& loader, GlResourceManager& resources) noexcept:
    m_loader(&loader),
    m_glResources(&resources),
    Transform2D(),
    m_background{},
    m_foreground{},
    m_isLoaded(false)
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

    return true;
}


void TileMap::draw(const ivec4s& viewport) const noexcept
{
    const bool needSecondSampler = ((m_foreground.texture != 0) && (m_foreground.texture != m_background.texture));

	glBindTextureUnit(0, m_background.texture);

    if (needSecondSampler)
        glBindTextureUnit(1, m_foreground.texture);

	glBindVertexArray(m_background.VAO);
	// glDrawElements(GL_TRIANGLES, m_background.indexCount, GL_UNSIGNED_INT, nullptr);

    const auto mapSize = m_loader->getMapSize();
    const auto tileSize = m_loader->getTileSize();
    const auto mapSizeInPixels = glms_ivec2_mul(mapSize, tileSize);

    uint32_t left   = (viewport.x < 1) ? 0 : viewport.x;
    uint32_t top    = (viewport.y < 1) ? 0 : viewport.y;
    uint32_t width  = left + viewport.z + tileSize.x;
    uint32_t height = top + viewport.w + tileSize.y + (tileSize.y >> 1);

    if (width >= mapSizeInPixels.x)  width = mapSizeInPixels.x;
    if (height >= mapSizeInPixels.y) height = mapSizeInPixels.y;

    const uint32_t x = left / tileSize.x;
    const uint32_t y = top / tileSize.y;

    const uint32_t tile = x + y * mapSize.x;
    const GLsizei lines = (height - top) / tileSize.y;
    const GLsizei length = (width - left) / tileSize.x;

    std::array<GLsizei, 64> counts; // TODO: dynamic count (using "lines" variable)
    counts.fill(length * 6);

    std::array<const void*, 64> offsets; // TODO: dynamic count (using "lines" variable)

    for (GLsizei i = 0; i < lines; ++i)
        offsets[i] = (const void*)((tile + i * mapSize.x) * 6 * sizeof(GLuint));
    
    glMultiDrawElements(GL_TRIANGLES, counts.data(), GL_UNSIGNED_INT, offsets.data(), lines);

    if (m_foreground.VAO)
    {
        glBindVertexArray(m_foreground.VAO);
	    glDrawElements(GL_TRIANGLES, m_foreground.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
	glBindTextureUnit(0, 0);

    if (needSecondSampler)
        glBindTextureUnit(1, 0);
}


bool TileMap::loadTextures() noexcept
{
    const auto& tilePlanes = m_loader->getLayers();

    auto planeB = std::find_if(tilePlanes.begin(), tilePlanes.end(), [](const TmxLoader::Layer& plane) -> bool
    {
        return plane.name == "Background";
    });

    auto planeA = std::find_if(tilePlanes.begin(), tilePlanes.end(), [](const TmxLoader::Layer& plane) -> bool
    {
        return plane.name == "Foreground";
    });

    if (planeB == tilePlanes.end()) // At least one tile layer must exist.
        return false;

    const auto textureCount = std::count_if(tilePlanes.begin(), tilePlanes.end(), [](const TmxLoader::Layer& plane) -> size_t
    {
        return (plane.tileset->image.pixels != nullptr);
    });

    if (textureCount == 0)
        return false;

    auto textures = m_glResources->getHandles(GlResourceManager::GLTexture2D, textureCount);

    if (textures.empty())
        return false;

    Texture2D background(textures[0]);

    if (!background.loadFromImage(planeB->tileset->image))
        return false;

    if (textures.size() > 1)
    {
        Texture2D foreground(textures[0]);

        if (!foreground.loadFromImage(planeA->tileset->image))
            return false;

        m_foreground.texture = foreground.getHandle();
    }
    else
    {
        m_foreground.texture = background.getHandle();
    }

    m_background.texture = background.getHandle();

    return true;
}


void TileMap::createBufferData(BufferData& data) noexcept
{
    const auto* layer = static_cast<const TmxLoader::Layer*>(data.tileLayer);
    
    std::span<const uint32_t> tiles = layer->tiles;
    const auto* tileset = layer->tileset;

    auto& vertices = data.vertices;
    auto& indices = data.indices;

    vertices.reserve(tiles.size() * 4);
    indices.reserve(tiles.size() * 6);

    const vec2s ratio = 
    { 
        1.f / static_cast<float>(tileset->image.width), 
        1.f / static_cast<float>(tileset->image.height)
    };

    const auto mapSize  = m_loader->getMapSize();
    const auto tileSize = m_loader->getTileSize();
    size_t index = 0;

    for (int32_t y = 0; y < mapSize.y; ++y)
    {
        for (int32_t x = 0; x < mapSize.x; ++x)
        {
            const int32_t tileId = tiles[index++];

            if (tileId)
            {
//  Positions
                const vec2s leftBottom  = { static_cast<float>(x * tileSize.x),              static_cast<float>(y * tileSize.y + tileSize.y) };
                const vec2s rightBottom = { static_cast<float>(x * tileSize.x + tileSize.x), static_cast<float>(y * tileSize.y + tileSize.y) };
                const vec2s rightTop    = { static_cast<float>(x * tileSize.x + tileSize.x), static_cast<float>(y * tileSize.y) };
                const vec2s leftTop     = { static_cast<float>(x * tileSize.x),              static_cast<float>(y * tileSize.y) };
//  UV
                const int32_t tileNum = tileId - tileset->firstGID;

                const int32_t vY = (tileNum >= tileset->columns) ? tileNum / tileset->columns : 0;
                const int32_t vX = tileNum % tileset->columns;

                const int32_t offsetX = vX * tileSize.x;
                const int32_t offsetY = vY * tileSize.y;

                const float left   = offsetX * ratio.x;
                const float top    = offsetY * ratio.y;
                const float right  = (offsetX + tileSize.x) * ratio.x;
                const float bottom = (offsetY + tileSize.y) * ratio.y;

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
}


void TileMap::loadGraphics() noexcept
{
    const auto& layers = m_loader->getLayers();
    auto buffers = m_glResources->getHandles(GlResourceManager::GLBuffer, 4);

    m_background.VBO = buffers[0];
    m_background.EBO = buffers[1];
    m_foreground.VBO = buffers[2];
    m_foreground.EBO = buffers[3];

    {// Background tile plane
        BufferData data;
        data.tileLayer = static_cast<const void*>(&layers.front());
        createBufferData(data);

        std::span<const float> vertices = data.vertices;
        std::span<const uint32_t> indices = data.indices;
        glNamedBufferStorage(m_background.VBO, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), 0);
        glNamedBufferStorage(m_background.EBO, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), 0);
        m_background.indexCount = indices.size(); 
    }

    if (layers.size() > 1) // Foreground tile plane
    {
        BufferData data;
        data.tileLayer = static_cast<const void*>(&layers.back());
        createBufferData(data);

        std::span<const float> vertices = data.vertices;
        std::span<const uint32_t> indices = data.indices;
        glNamedBufferStorage(m_foreground.VBO, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(), 0);
        glNamedBufferStorage(m_foreground.EBO, static_cast<GLsizeiptr>(indices.size_bytes()), indices.data(), 0);
        m_foreground.indexCount = indices.size(); 
    }

    auto vertexArrays = m_glResources->getHandles(GlResourceManager::GLVertexArray, 2);

    const std::array<VertexBufferLayout::Attribute, 1> attributes = { VertexBufferLayout::Attribute::Float4 };
	VertexBufferLayout layout(attributes);

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

    const std::array<GLuint, 2> vertexArrays = { m_background.VAO, m_foreground.VAO };
    m_glResources->destroyHandles(GlResourceManager::GLVertexArray, vertexArrays);

    const std::array<GLuint, 4> buffers = 
    { 
        m_background.VBO, m_background.EBO,
        m_foreground.VBO, m_foreground.EBO 
    };
    m_glResources->destroyHandles(GlResourceManager::GLBuffer, buffers);

    const std::array<GLuint, 2> textures = { m_background.texture, m_foreground.texture };
    m_glResources->destroyHandles(GlResourceManager::GLTexture2D, textures);

    memset(&m_background, 0, sizeof(m_background));
    memset(&m_foreground, 0, sizeof(m_foreground));

    m_isLoaded = false;
}


const TmxLoader& TileMap::getLoader() const noexcept
{
    return *m_loader;
}