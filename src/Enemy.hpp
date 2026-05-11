#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Entity.hpp"


class Enemy: public Entity
{
public:
	Enemy(const Animator& a, int x, int y) noexcept:
		Entity(a, x, y)
	{
		setType<Enemy>(); 
		setOptions(20.0f, 15, Animation::Type::EnemyMove);
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
			anim.set(Animation::Type::EnemyDead);
			dx = 0;
			timer_end += dt;

			if (timer_end > 2)
				isAlive = false;
		}

		anim.tick(dt);
	}
};

#endif // !ENEMY_HPP
