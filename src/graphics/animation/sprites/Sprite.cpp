#include <cassert>

#include <glad/glad.h>

#include "graphics/animation/sprites/Sprite.hpp"


Sprite::Sprite() noexcept:
    Transform2D()
{

}


Sprite::Sprite(const SpriteData& data) noexcept:
    m_data(data)
{

}


Sprite::~Sprite() = default;


void Sprite::setTexture(uint32_t texture) noexcept
{
    m_data.texture = texture;
}


void Sprite::draw(uint32_t vao) const noexcept
{
    assert(m_data.texture != 0);
    
    glBindTextureUnit(0, m_data.texture);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, m_data.frame, 4);
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
}