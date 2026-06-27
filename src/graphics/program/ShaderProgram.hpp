#pragma once

#include <cstdint>
#include <span>


class ShaderProgram final
{
public:
    ShaderProgram() noexcept;
    ShaderProgram(const ShaderProgram&) noexcept = delete;
    ShaderProgram(ShaderProgram&&) noexcept;
    ShaderProgram& operator = (const ShaderProgram&) noexcept = delete;
    ShaderProgram& operator = (ShaderProgram&&) noexcept;
    ~ShaderProgram() noexcept;

    void operator()(bool bind) const noexcept;

    uint32_t link(std::span<const class Shader> shaders) noexcept;
    uint32_t getHandle() const noexcept;
    int32_t  getUniformLocation(const char* name) const noexcept;

private:
    uint32_t m_handle;
};