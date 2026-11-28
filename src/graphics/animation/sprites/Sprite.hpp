#pragma once

#include "graphics/animation/sprites/SpriteData.hpp"
#include "graphics/transform/Transform2D.hpp"


class Sprite:
    public Transform2D
{
public:
    Sprite() noexcept;
    Sprite(const SpriteData& data) noexcept;
    ~Sprite();

    void setTexture(uint32_t texture) noexcept;

    void draw(uint32_t vao) const noexcept;

private:
    SpriteData m_data{0, 0, 0, 0};
};
