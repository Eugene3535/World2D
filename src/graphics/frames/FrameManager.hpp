#pragma once

#include <string>
#include <vector>
#include <list>
#include <span>
#include <unordered_map>
#include <filesystem>

#include <SFML/Graphics/Rect.hpp>



class FrameManager final
{
public:
	using Frames = std::vector<sf::IntRect>;
	using FrameView = std::span<const sf::IntRect>;

	FrameManager() noexcept;
	FrameManager(const FrameManager&)              noexcept = delete;
	FrameManager(FrameManager&&)                   noexcept = delete;
	FrameManager& operator = (const FrameManager&) noexcept = delete;
	FrameManager& operator = (FrameManager&&)      noexcept = delete;
	~FrameManager();


	void addFrame(const std::string& name, const sf::IntRect& frame)                                                noexcept;
	void addLinearAnimaton(const std::string& name, const sf::Vector2u& textureSize, uint32_t duration)             noexcept;
	void addGridAnimaton(const std::string& name, const sf::Vector2u& textureSize, uint32_t columns, uint32_t rows) noexcept;
	void addCustomAnimaton(const std::string& name, FrameView frames)                                               noexcept;
	void loadSpriteSheet(const std::filesystem::path& filePath)                                                     noexcept;

	sf::IntRect getFrame(const std::string& name)     const noexcept;
	FrameView   getAnimation(const std::string& name) const noexcept;

private:
	std::list<sf::IntRect> m_frameList;
	std::list<Frames> m_animationList;
	std::unordered_map<std::string, FrameView> m_animations;
};