#include "game/Game.hpp"
#include "game/objects/GameObject.hpp"



GameObject::GameObject(Game* game) noexcept:
	m_game(game),
	m_position{ 0, 0 },
	m_size{ 0, 0 }
{
	setType<GameObject>();
}


GameObject::~GameObject() = default;


uint32_t GameObject::nextId() noexcept
{
	static uint32_t id = 0;

	return id++;
}
