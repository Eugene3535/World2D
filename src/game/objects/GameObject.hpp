#pragma once

#include "game/objects/GameObjectData.hpp"


class GameObject
{
public:
	GameObject(class Game* game) noexcept;
	virtual ~GameObject();

	virtual void create(const GameObjectData& data) noexcept = 0;
	virtual void spawn(ivec2s position) noexcept = 0;
	virtual void update(float dt) noexcept = 0;

	template<class T>
	bool is() const noexcept
	{
		return (m_typeId == getStaticId<T>());
	}

protected:
	template<class T>
	static uint32_t getStaticId() noexcept
	{
		static uint32_t id = nextId();

		return id;
	}

	template<class T>
	void setType() noexcept
	{
		m_typeId = getStaticId<T>();
	}

	class Game* m_game;

	ivec2s m_position;
	ivec2s m_size;

private:
	static uint32_t nextId() noexcept;

	uint32_t m_typeId = 0;
};
