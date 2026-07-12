#pragma once

#include "game/objects/GameObject.hpp"
#include "graphics/render/Drawable.hpp"


class Goomba:
    public GameObject,
    public Drawable
{
public:
    Goomba(class Game* game) noexcept;
    ~Goomba();

    void create(const GameObjectData& data) noexcept override;
    void spawn(ivec2s position) noexcept override;
	void update(float dt) noexcept override;
	void draw(uint32_t shaderProgram) const noexcept override;

private:
    vec2s m_velocity;
};