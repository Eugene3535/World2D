#include "game/objects/mobs/Goomba.hpp"


Goomba::Goomba(Game* game) noexcept:
    GameObject(game),
    m_velocity{5.f, 5.f}
{
    setType<Goomba>();
}


Goomba::~Goomba() = default;


void Goomba::create(const GameObjectData& data) noexcept
{

}


void Goomba::spawn(ivec2s position) noexcept
{
    m_position = position;
}


void Goomba::update(float dt) noexcept
{
    m_position.x += m_velocity.x * dt;
}


void Goomba::draw(uint32_t shaderProgram) const noexcept
{

}