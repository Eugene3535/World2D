#ifndef MOVING_PLATFORM_HPP
#define MOVING_PLATFORM_HPP

#include "Entity.hpp"

class MovingPlatform: public Entity
{
public:
	MovingPlatform(sf::Sprite* sprite, const AnimationManager& a, int x, int y) noexcept:
		Entity(sprite, a, x, y)
	{
		setType<Enemy>();
		setOptions(60.0f, 0, "move");
	}

	void update(float dt) noexcept
	{
		hitbox.position.x += dx * dt;
		timer += dt;

		if (timer > 4.0f)
		{
			dx *= -1;
			timer = 0;
		}

		anim.tick(dt);
	}
};

#endif // !MOVING_PLATFORM_HPP
