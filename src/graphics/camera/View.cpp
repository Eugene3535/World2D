#include <glad/glad.h>
#include <cglm/struct/cam.h>
#include "cglm/struct/affine-mat.h"

#include "graphics/camera/View.hpp"


View::View() noexcept:
    m_projection(glms_mat4_identity()),
    m_uniformBuffer(0),
    m_flipVertically(true)
{

}


View::~View()
{
    if(m_uniformBuffer)
        glDeleteBuffers(1, &m_uniformBuffer);
}


void View::create() noexcept
{
    if (!m_uniformBuffer)
    {
        glCreateBuffers(1, &m_uniformBuffer);
        glNamedBufferData(m_uniformBuffer, sizeof(mat4s), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBuffer, 0, sizeof(mat4s));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}


void View::resize(int32_t width, int32_t height) noexcept
{
    m_projection = m_flipVertically ? 
        glms_ortho(0.f, static_cast<float>(width), static_cast<float>(height), 0.f, -1.f, 1.f) :
        glms_ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -1.f, 1.f);
}


void View::updateUniformBuffer(mat4 modelViewProjection) const noexcept
{
    glNamedBufferSubData(m_uniformBuffer, 0, sizeof(mat4s), static_cast<const void*>(modelViewProjection));
}


void View::flipVertically(bool flip) noexcept
{
    m_flipVertically = flip;
}


const mat4s& View::getProjectionMatrix() const noexcept
{
    return m_projection;
}


bool View::isUpsideDown() const noexcept
{
    return m_flipVertically;
}