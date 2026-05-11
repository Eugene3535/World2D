#ifndef BULLET_HPP
#define BULLET_HPP

#include "Entity.hpp"

class Bullet: public Entity
{
public:
	Bullet(sf::Sprite* sprite, const AnimationManager& a, std::span<Object> obj, int x, int y, bool dir) noexcept:
		Entity(sprite, a, x, y)
	{
		setType<Bullet>(); 
		setOptions(400.0f, 10, "move");

		if (dir) dx = -dx;
		objects = obj;
	}

	void update(float dt) noexcept
	{
		hitbox.position.x += dx * dt;

		for (const auto& obj : objects)
			if (hitbox.findIntersection(obj.bounds))	
				Health = 0;	
		
		if (Health < 1) 
		{
			anim.set("explode"); 
			dx = 0;

			if (!anim.isPlaying()) 
				isAlive = false;
		}

		anim.tick(dt);
	}
};

#endif // !BULLET_HPP
