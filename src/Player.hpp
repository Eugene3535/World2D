#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Entity.hpp"


class Player : public Entity
{
public:
	enum { stay, walk, duck, jump, climb, swim } state;
	bool onLadder, shoot, hit;

	struct
	{
		bool L, R, Up, Down, Space;
	} key;


	Player(sf::Sprite* sprite, const AnimationManager& a, std::span<Object> lev, int x, int y) noexcept:
		Entity(sprite, a, x, y)
	{
		setType<Player>(); 
		setOptions(0, 100, "stay");
		state = stay; hit = false;
		objects = lev;
		key.R = key.L = key.Up = key.Down = key.Space = false;
	}

	void Keyboard() noexcept
	{
		if (key.L)
		{
			dir = 1;
			if (state != duck) dx = -150;
			if (state == stay) state = walk;
		}

		if (key.R)
		{
			dir = 0;
			if (state != duck) dx = 150;
			if (state == stay) state = walk;
		}

		if (key.Up)
		{
			if (onLadder) state = climb;
			if (state == stay || state == walk) { dy = -360; state = jump; anim.play("jump"); }
			if (state == climb) dy = -100;
			if (state == climb) if (key.L || key.R) state = stay;
		}

		if (key.Down)
		{
			if (state == stay || state == walk) { state = duck; dx = 0; }
			if (state == climb) dy = 100;
		}

		if (key.Space)
		{
			shoot = true;
		}

		/////////////////////если клавиша отпущена///////////////////////////
		if (!(key.R || key.L))
		{
			dx = 0;
			if (state == walk) state = stay;
		}

		if (!(key.Up || key.Down))
		{
			if (state == climb) dy = 0;
		}

		if (!key.Down)
		{
			if (state == duck) { state = stay; }
		}

		if (!key.Space)
		{
			shoot = false;
		}

		key.R = key.L = key.Up = key.Down = key.Space = false;
	}

	void Animation(float dt) noexcept
	{
		if (state == stay) anim.set("stay");
		if (state == walk) anim.set("walk");
		if (state == jump) anim.set("jump");
		if (state == duck) anim.set("duck");
		if (state == climb) { anim.set("climb"); anim.pause(); if (dy != 0) anim.play(); }

		if (shoot) 
		{
			anim.set("shoot");
			if (state == walk) 
				anim.set("shootAndWalk");
		}

		if (hit) 
		{
			timer += dt;
			if (timer > 1) { hit = false; timer = 0; }
			anim.set("hit");
		}

		if (dir) anim.flip();

		anim.tick(dt);
	}

	void update(float dt) noexcept
	{
		Keyboard();

		Animation(dt);

		if (state == climb) if (!onLadder) state = stay;
		if (state != climb) dy += 1000.0f * dt;
		onLadder = false;

		hitbox.position.x += dx * dt;
		Collision(0);

		hitbox.position.y += dy * dt;
		Collision(1);
	}

	void Collision(int num) noexcept
	{
		const auto bounds = hitbox;

		for (int i = 0; i < objects.size(); i++)
			if (bounds.findIntersection(objects[i].bounds))
			{
				if (objects[i].name == "solid")
				{
					if (dy > 0 && num == 1) { hitbox.position.y  = objects[i].bounds.position.y - hitbox.size.y;  dy = 0;   state = stay; }
					if (dy < 0 && num == 1) { hitbox.position.y  = objects[i].bounds.position.y + objects[i].bounds.size.y;   dy = 0; }
					if (dx > 0 && num == 0) { hitbox.position.x = objects[i].bounds.position.x - hitbox.size.x; }
					if (dx < 0 && num == 0) { hitbox.position.x = objects[i].bounds.position.x + objects[i].bounds.size.x; }
				}

				if (objects[i].name == "ladder") { onLadder = true; if (state == climb) hitbox.position.x = objects[i].bounds.position.x - 10; }

				if (objects[i].name == "SlopeLeft")
				{
					const sf::FloatRect& r = objects[i].bounds;
					int y0 = (hitbox.position.x + hitbox.size.x * 0.5f - r.position.x) * r.size.y / r.size.x + r.position.y - hitbox.size.y;

					if (hitbox.position.y > y0)
					{
						if (hitbox.position.x + hitbox.size.x * 0.5f > r.position.x)
						{
							hitbox.position.y = y0; dy = 0; state = stay;
						}
					}
				}

				if (objects[i].name == "SlopeRight")
				{
					const sf::FloatRect& r = objects[i].bounds;
					int y0 = -(hitbox.position.x + hitbox.size.x * 0.5f - r.position.x) * r.size.y / r.size.x + r.position.y + r.size.y - hitbox.size.y;

					if (hitbox.position.y > y0)
						if (hitbox.position.x + hitbox.size.x * 0.5f < r.position.x + r.size.x)
						{
							hitbox.position.y = y0; 
							dy = 0; 
							state = stay;
						}
				}

			}
	}
};


#endif // !PLAYER_HPP
