#pragma once

#include <cstdint>


class GameObject
{
public:
	enum Type
	{
		None,
		Player,
		Enemy,
		Bullet,
		Spawner
	};

	GameObject(class Game* game, Type type) noexcept;
	virtual ~GameObject();

	virtual void update(float dt) noexcept;
	virtual void draw() noexcept;

	Type getType() const noexcept;

protected:
	class Game* m_game;

private:
	Type m_type;
};
