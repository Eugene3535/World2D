#include <vector>
#include <memory>
#include <cstdio>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include "common/MemoryAllocator.hpp"
#include "Animation.hpp"
#include "Entity.hpp"
#include "Bullet.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "MovingPlatform.hpp"
#include "HealthBar.hpp"


void SetSpriteSize(sf::Sprite& sprite, float width, float height)
{
    const auto& rect = sprite.getTextureRect();

	if (rect.size.x && rect.size.y)
	{
		float dx = width / std::abs(rect.size.x);
		float dy = height / std::abs(rect.size.y);
		sprite.setScale({dx, dy});
	}
}


void SetSpriteSize(sf::Sprite& sprite, const sf::Vector2f& size)
{
	SetSpriteSize(sprite, size.x, size.y);
}


sf::Vector2f GetSpriteSize(const sf::Sprite& sprite)
{
	const auto& rect  = sprite.getTextureRect();
	const auto& scale = sprite.getScale();

	return { rect.size.x * scale.x, rect.size.y * scale.y };
}




int main()
{
	sf::RenderWindow window(sf::VideoMode({800, 600}), "The Game!");
	window.setVerticalSyncEnabled(true);

	sf::View view(sf::FloatRect({ 0.f, 0.f }, { 800.f, 600.f }));

	sf::Image image;
	if (!image.loadFromFile("res/images/tileset1.png")) return -1;
	image.createMaskFromColor(sf::Color::White);

	sf::Texture tileset;
	if (!tileset.loadFromImage(image)) return -1;

	Level lvl;
	lvl.LoadFromFile("res/Level-1.tmx", &tileset);

	sf::Texture enemy_t, moveplatform_t, megaman_t, bullet_t, bg;
	if (!bg.loadFromFile("res/images/bg.png")) return -1;
	if (!enemy_t.loadFromFile("res/images/enemy.png")) return -1;
	if (!moveplatform_t.loadFromFile("res/images/movingPlatform.png")) return -1;
	if (!megaman_t.loadFromFile("res/images/megaman.png")) return -1;
	if (!bullet_t.loadFromFile("res/images/bullet.png")) return -1;


	auto allocator = std::make_unique<MemoryAllocator<1024 * 48>>();

	AnimationManager anim(allocator->allocate<sf::Sprite>(megaman_t));
	anim.loadFromXML("res/anim_megaman.xml", megaman_t);
	anim.animList[Animation::Type::PlayerJump].loop = 0;

	AnimationManager anim2(allocator->allocate<sf::Sprite>(bullet_t));
	anim2.create("BulletMove", bullet_t, 7, 10, 8, 8, 1, 0);
	anim2.create("BulletExplode", bullet_t, 27, 7, 18, 18, 4, 16, 29, false);
//	anim2.set("move"); // start animation

	AnimationManager anim3(allocator->allocate<sf::Sprite>(enemy_t));
	anim3.create("EnemyMove", enemy_t, 0, 0, 16, 16, 2, 4, 18);
	anim3.create("EnemyDead", enemy_t, 58, 0, 16, 16, 1, 0);

	AnimationManager anim4(allocator->allocate<sf::Sprite>(moveplatform_t));
	anim4.create("MovingPlatformMove", moveplatform_t, 0, 0, 95, 22, 1, 0);

	sf::Sprite* background = allocator->allocate<sf::Sprite>(bg);
	background->setOrigin(sf::Vector2f(bg.getSize()) * 0.5f);
	SetSpriteSize(*background, 800, 600);

	std::vector<std::unique_ptr<Entity>> entities;

	std::vector<Object> enemyObjects = lvl.GetObjects("enemy");
	std::vector<Object> mpObjects    = lvl.GetObjects("MovingPlatform");
	std::vector<Object> solidObjects = lvl.GetObjects("solid");
	std::vector<Object> allObjects   = lvl.GetAllObjects();

	for (int i = 0; i < enemyObjects.size(); i++)
		entities.emplace_back(std::make_unique<Enemy>(allocator->allocate<sf::Sprite>(enemy_t), anim3, static_cast<int>(enemyObjects[i].bounds.position.x), static_cast<int>(enemyObjects[i].bounds.position.y)));

	for (int i = 0; i < mpObjects.size(); i++)
		entities.emplace_back(std::make_unique<MovingPlatform>(allocator->allocate<sf::Sprite>(moveplatform_t), anim4, static_cast<int>(mpObjects[i].bounds.position.x), static_cast<int>(mpObjects[i].bounds.position.y)));

	Object pl = lvl.GetObject("player");
	Player Mario(allocator->allocate<sf::Sprite>(megaman_t), anim, allObjects, pl.bounds.position.x, pl.bounds.position.y);

	HealthBar healthBar;

	sf::Clock clock;

	int cooldown = 0;

	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds(); // 33.000 было

		++cooldown;

		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
					window.close();

				if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
				{
					if (cooldown > 10)
					{
						entities.emplace_back(new Bullet(allocator->allocate<sf::Sprite>(bullet_t), anim2, solidObjects, static_cast<int>(Mario.hitbox.position.x + 18), static_cast<int>(Mario.hitbox.position.y + 18), Mario.dir));
						cooldown = 0;
					}
				}
			}

			if (const auto* resized = event->getIf<sf::Event::Resized>())
			{
				view.setSize(sf::Vector2f(resized->size.x, resized->size.y));
				background->setOrigin(sf::Vector2f(bg.getSize()) * 0.5f);
				SetSpriteSize(*background, resized->size.x, resized->size.y);
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			Mario.key.L = true;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			Mario.key.R = true;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			Mario.key.Up = true;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			Mario.key.Down = true;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
			Mario.key.Space = true;

		auto quick_remove = [&allocator](std::vector<std::unique_ptr<Entity>>& v, size_t i)
		{
			auto sprite = v[i]->anim.sprite;
			v[i].swap(v.back());
			allocator->release<sf::Sprite>(sprite);
			v.pop_back();
		};

		for (auto i = 0; i < entities.size(); ++i)
		{
			entities[i]->update(dt);

			if (!entities[i]->isAlive)
			{
				quick_remove(entities, i);
			}
		}

		Mario.update(dt);
		healthBar.update(Mario.Health);

		for (auto& it : entities)
		{
			// 1. �����
			//if ((*it)->Name == "Enemy")
			if(it->is<Enemy>())
			{
				Entity* enemy = it.get();

				if (enemy->Health < 1)
					continue;

				if (Mario.hitbox.findIntersection(enemy->hitbox))
				{
					if (Mario.dy > 0)
					{
						enemy->dx = 0;
						Mario.dy = -0.2;
						enemy->Health = 0;
					}
					else if (!Mario.hit)
					{
						Mario.Health -= 5;
						Mario.hit = true;
						if (Mario.dir)
							Mario.hitbox.position.x += 10;
						else
							Mario.hitbox.position.x -= 10;
					}
				}

				for (auto& it2 : entities)
				{
					//if (bullet->Name == "Bullet")
					if(it2->is<Bullet>())
						if (it2->Health > 0)
							if (it2->hitbox.findIntersection(enemy->hitbox))
							{
								it2->Health = 0;
								enemy->Health -= 5;
							}
				}
			}

			// 2. ���������� ���������
			//if ((*it)->Name == "MovingPlatform")
			if(it->is<MovingPlatform>())
			{
				if (Mario.hitbox.findIntersection(it->hitbox))
					if (Mario.dy > 0)
					{
						if (Mario.hitbox.position.y + Mario.hitbox.size.y < it->hitbox.position.y + it->hitbox.size.y)
						{
							Mario.hitbox.position.y = it->hitbox.position.y - Mario.hitbox.size.y + 3;
							Mario.hitbox.position.x += it->dx * dt;
							Mario.dy = 0;
							Mario.state = Player::stay;
						}
					}
			}

			// 3.. � �.�.
		}


		view.setCenter({Mario.hitbox.position.x + Mario.hitbox.size.x / 2, Mario.hitbox.position.y + Mario.hitbox.size.y / 2});
		window.setView(view);

		sf::Vector2f center  { view.getCenter() };
		sf::Vector2f winSize { window.getSize() };

		int half_w = winSize.x / 2;
		int half_h = winSize.y / 2;

		sf::FloatRect viewport;
		viewport.position.x   = center.x - half_w - 32;
		viewport.position.y    = center.y - half_h - 32;
		viewport.size.x  = winSize.x + 32;
		viewport.size.y = winSize.y + 32;

		background->setPosition(view.getCenter());
		window.clear();
		window.draw(*background);

		lvl.Draw(window);

		for (const auto& e : entities)
		{
			if(viewport.findIntersection(e->hitbox))
				e->draw(window);
		}

		Mario.draw(window);
		healthBar.draw(window);

		window.display();
	}

	return 0;
}

