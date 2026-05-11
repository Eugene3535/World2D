#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <span>

#include "Animation.hpp"
#include "Level.hpp"

class Entity
{
public:
	Animator anim;
	std::span<Object> objects;
	float timer, timer_end;
	sf::FloatRect hitbox;
	float dx, dy;
	int Health;
	bool isAlive, dir;

	Entity(const Animator& A, int X, int Y) noexcept:
		anim(A)
	{
		hitbox.position.x = X;
		hitbox.position.y = Y;
		dir = 0;
		Health = 0;

		isAlive = true;
		timer = 0;
		timer_end = 0;
		dx = 0;
		dy = 0;
	}

	virtual ~Entity() noexcept = default;

	virtual void update(float dt) noexcept {}

	void draw(sf::RenderWindow& window) noexcept
	{
		anim.draw(window, hitbox.position.x, hitbox.position.y);
	}

	void setOptions(float speed, int hp, Animation::Type startAnim) noexcept
	{
		anim.set(startAnim);

		hitbox.size.x = anim.getW();
		hitbox.size.y = anim.getH();
		dx = speed;
		Health = hp;
	}

	template<class T>
	bool is() const noexcept
	{
		return m_typeId == getStaticId<T>();
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

private:
	static uint32_t nextId() noexcept
	{
		static uint32_t id = 0;

		return id++;
	}

	uint32_t m_typeId = 0;
};

#endif // ENTITY_HPP
