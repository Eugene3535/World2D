#ifdef DEBUG
#include <cstdio>
#endif

#include <glad/glad.h>

#include "files/Shader.hpp"
#include "graphics/program/ShaderProgram.hpp"


ShaderProgram::ShaderProgram() noexcept:
    m_handle(0)
{
 
}


ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept:
    m_handle(other.m_handle)
{
    other.m_handle = 0;
}


ShaderProgram& ShaderProgram::operator = (ShaderProgram&& other) noexcept
{
    m_handle = other.m_handle;
    other.m_handle = 0;

    return *this;
}


ShaderProgram::~ShaderProgram() noexcept
{
    glDeleteProgram(m_handle);
}


void ShaderProgram::operator()(bool bind) const noexcept
{
    glUseProgram( (bind && m_handle) ? m_handle : 0 );
}


uint32_t ShaderProgram::link(std::span<const Shader> shaders) noexcept
{
    glDeleteProgram(m_handle);
    m_handle = 0;

    GLint program = glCreateProgram();

    for(const auto& shader : shaders)
        glAttachShader(program, shader.getHandle());
    
    glLinkProgram(program);

    int32_t success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        GLchar infoLog[1024]{};
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        printf("SHADER PROGRAM: Link-time error:\n%s\n", infoLog);
        glDeleteProgram(program);
    }
    else
    {
        uint32_t uniformBlockIndex = glGetUniformBlockIndex(program, "Matrices");
        glUniformBlockBinding(program, uniformBlockIndex, 0);
        m_handle = program;
    }

    for(const auto& shader : shaders)
        glDetachShader(program, shader.getHandle());

    return m_handle;
}


uint32_t ShaderProgram::getHandle() const noexcept
{
    return m_handle;
}


int32_t ShaderProgram::getUniformLocation(const char* name) const noexcept
{
    if(m_handle)
        return glGetUniformLocation(m_handle, name);

    return -1;
}