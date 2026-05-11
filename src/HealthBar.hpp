#ifndef HEALTH_BAR_HPP
#define HEALTH_BAR_HPP

#include <memory>
#include <cassert>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class HealthBar
{
public:
	sf::Texture t;
	std::unique_ptr<sf::Sprite> s;
	const int max;
	sf::RectangleShape bar;

	HealthBar() noexcept:
		max(100)
	{
		assert(t.loadFromFile("res/images/HealthBar.png"));
		s = std::make_unique<sf::Sprite>(t);

		bar.setFillColor(sf::Color(0,0,0));
	}

	void update(int k) noexcept
	{
		if (k > 0 && k < max) 
			bar.setSize(sf::Vector2f(10, (max - k) * 70 / max));
	}

   	void draw(sf::RenderWindow &window) noexcept
	{
		sf::Vector2f center = window.getView().getCenter();
		sf::Vector2f size = window.getView().getSize();

		s->setPosition({center.x - size.x/2 + 10, center.y - size.y/2 + 10});
	    bar.setPosition({center.x - size.x/2 + 11, center.y - size.y/2 + 13});

		window.draw(*s);
		window.draw(bar);
	}
};

#endif // !HEALTH_BAR_HPP
