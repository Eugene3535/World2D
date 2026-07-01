#include "game/Game.hpp"
#include "game/objects/GameObject.hpp"



GameObject::GameObject(Game* game, Type type) noexcept:
	m_game(game),
	m_type(type)
{

}


GameObject::~GameObject() = default;


void GameObject::update(float dt) noexcept
{

}


void GameObject::draw() noexcept
{

}


GameObject::Type GameObject::getType() const noexcept
{
	return m_type;
}
