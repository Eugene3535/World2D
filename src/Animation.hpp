#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <list>
#include <unordered_map>
#include <span>
#include <cassert>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"
#include <magic_enum/magic_enum.hpp>

class Animation
{
public:
	enum class Type
	{
		BulletMove,
		BulletExplode,
		EnemyMove,
		EnemyDead,
		MovingPlatformMove,
		PlayerJump,
		PlayerHit,
		PlayerDuck,
		PlayerShoot,
		PlayerShootAndWalk,
		PlayerClimb,
		PlayerIdle,
		PlayerWalk,
		Undefined
	};


	std::span<const sf::IntRect> frames;
	std::span<const sf::IntRect> frames_flip;

	float currentFrame = 0;
	float speed = 0;
	bool loop = true;
	bool flip = false;
	bool isPlaying = true;
};

class Animator
{
public:
	Animator(const sf::Texture& texture) noexcept:
		sprite(texture)
	{

	}


	void create(const std::string& name, int x, int y, int w, int h, int count, float speed, int step = 0, bool Loop = true)
	{
		Animation a;
		a.speed = speed;
		a.loop = Loop;

		auto& frames = Animator::frameList.emplace_back();
		auto& frames_flip = Animator::frameList.emplace_back();

		frames.reserve(count);
		frames_flip.reserve(count);

		for (int i = 0; i < count; ++i)
		{
			frames.push_back(sf::IntRect({x + i * step, y}, {w, h}));
			frames_flip.push_back(sf::IntRect({x + i * step + w, y}, {-w, h}));
		}

		a.frames = frames;
		a.frames_flip = frames_flip;

		const auto animType = magic_enum::enum_cast<Animation::Type>(name).value_or(Animation::Type::Undefined);
		assert(animType != Animation::Type::Undefined);

		animList[animType] = a;
		currentAnim = &animList[animType];
	}

	void loadFromXML(const std::string& fileName)
	{
		auto document = std::make_unique<rapidxml::xml_document<char>>();
		rapidxml::file<char> xmlFile(fileName.c_str());
		document->parse<0>(xmlFile.data());

		const auto sprites = document->first_node("sprites");
		auto animations = sprites->first_node("animation");

		while (animations)
		{
			Animation anim;
			std::string title = animations->first_attribute("title")->value();
			int delay = atoi(animations->first_attribute("delay")->value());
			anim.speed = delay;

			auto& frames = Animator::frameList.emplace_back();
			auto& frames_flip = Animator::frameList.emplace_back();

			auto cut = animations->first_node("cut");

			while (cut)
			{
				auto pX = cut->first_attribute("x");
				auto pY = cut->first_attribute("y");
				auto pW = cut->first_attribute("w");
				auto pH = cut->first_attribute("h");

				int x = pX ? atoi(pX->value()) : 0;
				int y = pY ? atoi(pY->value()) : 0;
				int w = pW ? atoi(pW->value()) : 0;
				int h = pH ? atoi(pH->value()) : 0;

				frames.emplace_back(sf::IntRect({x, y}, {w, h}));
				frames_flip.emplace_back(sf::IntRect({x + w, y}, {-w, h}));

				cut = cut->next_sibling();
			}

			anim.frames = frames;
			anim.frames_flip = frames_flip;

			const auto animType = magic_enum::enum_cast<Animation::Type>(title).value_or(Animation::Type::Undefined);
			assert(animType != Animation::Type::Undefined);

			animList[animType] = anim;
			currentAnim = &animList[animType];

			animations = animations->next_sibling("animation");
		}
	}

	void set(Animation::Type name)
	{
		if(auto it = animList.find(name); it != animList.end())
		{
			currentAnim = &it->second;
			currentAnim->flip = 0;
			sprite.setTextureRect(currentAnim->flip ? currentAnim->frames_flip[0] : currentAnim->frames[0]);
		}
	}

	void draw(sf::RenderWindow& window, int x = 0, int y = 0)
	{
		sprite.setPosition(sf::Vector2f(x, y));
		window.draw(sprite);
	}

	void flip(bool b = 1) 
	{ 
		if (currentAnim)
			currentAnim->flip = b; 
	}

	void tick(float dt) 
	{
		if (!currentAnim->isPlaying)
			return;

		currentAnim->currentFrame += currentAnim->speed * dt;

		if (currentAnim->currentFrame >= currentAnim->frames.size())
		{
			currentAnim->currentFrame -= currentAnim->frames.size();

			if (!currentAnim->loop)
			{
				currentAnim->isPlaying = false;
				return;
			}
		}

		int i = currentAnim->currentFrame;
		const auto* nextFrame = currentAnim->flip ? (currentAnim->frames_flip.data() + i) : (currentAnim->frames.data() + i);

		sprite.setTextureRect(*nextFrame);
	}

	void pause() 
	{
		if(currentAnim)
			currentAnim->isPlaying = false; 
	}

	void play() 
	{
		if(currentAnim)
			currentAnim->isPlaying = true; 
	}

	void play(Animation::Type name) 
	{ 
		animList[name].isPlaying = true; 
	}

	bool isPlaying() 
	{
		if(currentAnim)
			return currentAnim->isPlaying;

		return false;
	}

	float getH() 
	{
		if(currentAnim)
			return currentAnim->frames[0].size.y;

		return 0.0f;
	}

	float getW() 
	{
		if(currentAnim)
			return currentAnim->frames[0].size.x;

		return 0.0f;
	}

	Animation* currentAnim = nullptr;
	std::unordered_map<Animation::Type, Animation> animList;
	sf::Sprite sprite;

private:
	static std::list<std::vector<sf::IntRect>> frameList;
};

std::list<std::vector<sf::IntRect>> Animator::frameList;

#endif // !ANIMATION_HPP
