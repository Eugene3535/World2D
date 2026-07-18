#pragma once

#include <unordered_map>
#include <string>
#include <span>

#include <SFML/Graphics/Sprite.hpp>


class Animator final
{
public:
    using Frames = std::span<const sf::IntRect>;

	Animator(sf::Sprite& sprite) noexcept;
    ~Animator();

    void add(const std::string& name, Frames frames, float rate, bool isLooped) noexcept;
	bool set(const std::string& name) noexcept;
    void setRate(float fps) noexcept;
	void update(float dt) noexcept;
    void stop() noexcept;
	void pause() noexcept;
	void play() noexcept;

	bool isPlaying() const noexcept;

    const sf::IntRect& getCurrentFrame() const noexcept;

private:
    sf::Sprite& m_sprite;

    struct Animation
    {
        Frames frames;
        float rate;
        bool isLooped;
    };

    std::unordered_map<std::string, Animation> m_animations;
    Animation* m_current;

    float m_rate;
    float m_timer;
    bool m_isPlaying;
};