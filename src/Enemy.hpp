#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Entity.hpp"


class Enemy: public Entity
{
public:
	Enemy(sf::Sprite* sprite, const AnimationManager& a, int x, int y) noexcept:
		Entity(sprite, a, x, y)
	{
		setType<Enemy>(); 
		setOptions(20.0f, 15, "move");
	}

	void update(float dt) noexcept
	{
		hitbox.position.x += dx * dt;
		timer += dt;

		if (timer > 3.0f)
		{
			dx *= -1;
			timer = 0;
		}

		if (Health < 1)
		{
			anim.set("dead");
			dx = 0;
			timer_end += dt;

			if (timer_end > 2)
				isAlive = false;
		}

		anim.tick(dt);
	}
};

#endif // !ENEMY_HPP
