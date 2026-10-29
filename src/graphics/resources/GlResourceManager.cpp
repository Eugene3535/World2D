#include <glad/glad.h>

#ifdef DEBUG
#include <cstdio>
#endif
#include <array>
#include <type_traits>

#include "files/FileProvider.hpp"
#include "files/Shader.hpp"
#include "graphics/resources/GlResourceManager.hpp"


static GLuint create_program(std::span<const GLuint> shaders) noexcept
{
    GLint program = glCreateProgram();

    for (const auto& shader : shaders)
        glAttachShader(program, shader);

    glLinkProgram(program);

    int32_t success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        GLchar infoLog[1024]{};
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
#ifdef DEBUG
        printf("SHADER PROGRAM: Link-time error:\n%s\n", infoLog);
#endif
        glDeleteProgram(program);

        return 0;
    }
    else
    {
        GLuint uniformBlockIndex = glGetUniformBlockIndex(program, "Matrices");

        if (uniformBlockIndex != GL_INVALID_INDEX)
            glUniformBlockBinding(program, uniformBlockIndex, 0);
    }

    for (const auto& shader : shaders)
        glDetachShader(program, shader);

    return program;
}


GlResourceManager::GlResourceManager() noexcept = default;


GlResourceManager::~GlResourceManager()
{
    reset();
}


bool GlResourceManager::compilePrograms() noexcept
{
    static_assert(std::is_same_v<GLbyte, int8_t>, "GLbyte mismatch");
    static_assert(std::is_same_v<GLubyte, uint8_t>, "GLubyte mismatch");
    static_assert(std::is_same_v<GLshort, int16_t>, "GLshort mismatch");
    static_assert(std::is_same_v<GLushort, uint16_t>, "GLushort mismatch");
    static_assert(std::is_same_v<GLint, int32_t>, "GLint mismatch");
    static_assert(std::is_same_v<GLuint, uint32_t>, "GLuint mismatch");
    static_assert(std::is_same_v<GLfloat, float>, "GLfloat mismatch");
    static_assert(std::is_same_v<GLdouble, double>, "GLdouble mismatch");

    std::unordered_map<std::string, Shader> shaderMap;

    for (const auto filename : {"sprite.frag", "sprite.vert"})
        shaderMap[filename] = Shader();

    for (auto& [filename, shader] : shaderMap)
    {
        GLenum target;

        if (filename.ends_with(".vert"))
            target = GL_VERTEX_SHADER;
        else if (filename.ends_with(".frag"))
            target = GL_FRAGMENT_SHADER;
        else if (filename.ends_with(".geom"))
            target = GL_GEOMETRY_SHADER;

        if (!shader.loadFromFile(FileProvider::findPathToFile(filename), target))
            return false;
    }


    { // Sprite
        std::array<GLuint, 2> shaders;
        shaders[0] = shaderMap["sprite.vert"].getHandle();
        shaders[1] = shaderMap["sprite.frag"].getHandle();

        if (const GLuint program = create_program(shaders))
            m_shaderPrograms.emplace("sprite", program);
    }

    return true;
}


std::span<const uint32_t> GlResourceManager::getHandles(Resource type, uint32_t count) noexcept
{
    std::vector<uint32_t>  handles(count);
    std::vector<uint32_t>* container{nullptr};

    switch (type)
    {
        case Resource::GLTexture2D:
            glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(count), handles.data());
            container = &m_textures;
            break;

        case Resource::GLBuffer:
            glCreateBuffers(static_cast<GLsizei>(count), handles.data());
            container = &m_buffers;
            break;

        case Resource::GLVertexArray:
            glCreateVertexArrays(static_cast<GLsizei>(count), handles.data());
            container = &m_vertexArrays;
            break;

        default:
            break;
    }

    if (container)
    {
        container->insert(container->end(), handles.begin(), handles.end());
        const size_t              index = container->size() - count;
        std::span<const uint32_t> sp(container->begin() + index, count);

        return sp;
    }

    return {};
}


void GlResourceManager::destroyHandles(Resource type, std::span<const uint32_t> handles) noexcept
{
    std::vector<uint32_t>* container{nullptr};

    if (!handles.empty())
    {
        switch (type)
        {
            case Resource::GLTexture2D:
                glDeleteTextures(static_cast<GLsizei>(handles.size()), handles.data());
                container = &m_textures;
                break;

            case Resource::GLBuffer:
                glDeleteBuffers(static_cast<GLsizei>(handles.size()), handles.data());
                container = &m_buffers;
                break;

            case Resource::GLVertexArray:
                glDeleteVertexArrays(static_cast<GLsizei>(handles.size()), handles.data());
                container = &m_vertexArrays;
                break;

            default:
                break;
        }

        if (container)
        {
            for (const auto handle : handles)
            {
                for (size_t i = 0; i < container->size(); ++i)
                {
                    if (handle == (*container)[i])
                    {
                        std::swap((*container)[i], container->back());
                        container->pop_back();
                    }
                }
            }
        }
    }
}


void GlResourceManager::appendPrograms(std::unordered_map<std::string, uint32_t>& programs) noexcept
{
    m_shaderPrograms.swap(programs);
}


uint32_t GlResourceManager::getProgram(const std::string& name) const noexcept
{
    if (auto it = m_shaderPrograms.find(name); it != m_shaderPrograms.end())
        return it->second;

    return 0;
}


void GlResourceManager::reset() noexcept
{
    if (!m_textures.empty())
    {
        glDeleteTextures(static_cast<GLsizei>(m_textures.size()), m_textures.data());
        m_textures.clear();
    }

    if (!m_buffers.empty())
    {
        glDeleteBuffers(static_cast<GLsizei>(m_buffers.size()), m_buffers.data());
        m_buffers.clear();
    }

    if (!m_vertexArrays.empty())
    {
        glDeleteVertexArrays(static_cast<GLsizei>(m_vertexArrays.size()), m_vertexArrays.data());
        m_vertexArrays.clear();
    }

    if (!m_shaderPrograms.empty())
    {
        for (const auto& [_, program] : m_shaderPrograms)
            glDeleteProgram(program);

        m_shaderPrograms.clear();
    }
}