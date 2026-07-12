#pragma once

#include "graphics/sprites/SpriteManager.hpp"


class Animator final
{
public:
	Animator() noexcept;
    ~Animator();

	void add(const std::string& name, std::span<const Sprite2D> frames, float rate, bool isLooped) noexcept;
	void set(const std::string& name) noexcept;
	void update(float dt) noexcept;
    void stop() noexcept;
	void pause() noexcept;
	void play() noexcept;
    void reverse(bool b) noexcept;

	bool isPlaying() const noexcept;
    bool isReversed() const noexcept;

    const Sprite2D* getCurrentSprite() const noexcept;

private:
    struct Animation
    {
        std::span<const Sprite2D> frames;
        float rate;
        bool isLooped;
    };

	Animation* m_current;
	std::unordered_map<std::string, Animation> m_animations;

    float m_timer;
    bool m_isPlaying;
    bool m_isReversed;
};