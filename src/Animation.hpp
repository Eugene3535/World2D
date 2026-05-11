#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <list>
#include <unordered_map>
#include <span>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"

class Animation
{
public:
	std::span<const sf::IntRect> frames;
	std::span<const sf::IntRect> frames_flip;
	const sf::Texture* texture = nullptr;

	float currentFrame = 0;
	float speed = 0;
	bool loop = true;
	bool flip = false;
	bool isPlaying = true;
};

class AnimationManager
{
public:
	AnimationManager(sf::Sprite* s) noexcept:
		sprite(s)
	{

	}


	void create(const std::string& name, const sf::Texture& texture, int x, int y, int w, int h, int count, float speed, int step = 0, bool Loop = true)
	{
		Animation a;
		a.texture = &texture;
		a.speed = speed;
		a.loop = Loop;

		auto& frames = AnimationManager::frameList.emplace_back();
		auto& frames_flip = AnimationManager::frameList.emplace_back();

		frames.reserve(count);
		frames_flip.reserve(count);

		for (int i = 0; i < count; ++i)
		{
			frames.push_back(sf::IntRect({x + i * step, y}, {w, h}));
			frames_flip.push_back(sf::IntRect({x + i * step + w, y}, {-w, h}));
		}

		a.frames = frames;
		a.frames_flip = frames_flip;

		animList[name] = a;
		currentAnim = &animList[name];
	}

	void loadFromXML(const std::string& fileName, const sf::Texture& texture)
	{
		auto document = std::make_unique<rapidxml::xml_document<char>>();
		rapidxml::file<char> xmlFile(fileName.c_str());
		document->parse<0>(xmlFile.data());

		const auto sprites = document->first_node("sprites");
		auto animations = sprites->first_node("animation");

		while (animations)
		{
			Animation anim;
			anim.texture = &texture;
			std::string title = animations->first_attribute("title")->value();
			int delay = atoi(animations->first_attribute("delay")->value());
			anim.speed = delay;

			auto& frames = AnimationManager::frameList.emplace_back();
			auto& frames_flip = AnimationManager::frameList.emplace_back();

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

			animList[title] = anim;
			currentAnim = &animList[title];

			animations = animations->next_sibling("animation");
		}
	}

	void set(const std::string& name)
	{
		if(auto it = animList.find(name); it != animList.end())
		{
			currentAnim = &it->second;
			currentAnim->flip = 0;
			sprite->setTexture(*currentAnim->texture);
			sprite->setTextureRect(currentAnim->flip ? currentAnim->frames_flip[0] : currentAnim->frames[0]);
		}
	}

	void draw(sf::RenderWindow& window, int x = 0, int y = 0)
	{
		sprite->setPosition(sf::Vector2f(x, y));
		window.draw(*sprite);
	}

	void flip(bool b = 1) 
	{ 
		if(currentAnim)
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

		sprite->setTextureRect(*nextFrame);
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

	void play(const std::string& name) 
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
	std::unordered_map<std::string, Animation> animList;
	sf::Sprite* sprite;

private:
	static std::list<std::vector<sf::IntRect>> frameList;
};

std::list<std::vector<sf::IntRect>> AnimationManager::frameList;

#endif // !ANIMATION_HPP
