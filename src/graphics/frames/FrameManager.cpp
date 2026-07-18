#include "RapidXML/rapidxml_utils.hpp"

#include "graphics/frames/FrameManager.hpp"


FrameManager::FrameManager() noexcept = default;
FrameManager::~FrameManager() = default;



void FrameManager::addFrame(const std::string& name, const sf::IntRect& frame) noexcept
{
	if(auto it = m_animations.find(name); it == m_animations.end())
	{
		auto& rect = m_frameList.emplace_back(frame);
		m_animations.emplace(name, FrameManager::FrameView(&rect, 1));
	}
}


void FrameManager::addLinearAnimaton(const std::string& name, const sf::Vector2u& textureSize, uint32_t duration) noexcept
{
	if(auto it = m_animations.find(name); it == m_animations.end())
	{
		auto& frames = m_animationList.emplace_back();
		frames.reserve(duration);

		const auto stride = textureSize.x / duration;
	
		for (uint32_t i = 0; i < duration; ++i)
		{
			frames.emplace_back(sf::Vector2i(i * stride, 0), sf::Vector2i(stride, textureSize.y));
		}

		m_animations.emplace(name, frames);
	}
}


void FrameManager::addGridAnimaton(const std::string& name, const sf::Vector2u& textureSize, uint32_t columns, uint32_t rows) noexcept
{
	if(auto it = m_animations.find(name); it == m_animations.end())
	{
		auto& frames = m_animationList.emplace_back();
		frames.reserve(columns * rows);

		const sf::Vector2u size { textureSize.x / columns, textureSize.y / rows };
	
		for (uint32_t y = 0; y < rows; ++y)
		{
			for (uint32_t x = 0; x < columns; ++x)
			{
				frames.emplace_back(sf::Vector2i(x * size.x, y * size.y), sf::Vector2i(size));
			}
		}

		m_animations.emplace(name, frames);
	}
}


void FrameManager::addCustomAnimaton(const std::string& name, FrameManager::FrameView frames) noexcept
{
	if(auto it = m_animations.find(name); it == m_animations.end())
	{
		auto& rects = m_animationList.emplace_back();
		rects.insert(rects.begin(), frames.begin(), frames.end());

		m_animations.emplace(name, rects);
	}
}


void FrameManager::loadSpriteSheet(const std::filesystem::path& filePath) noexcept
{
	auto document = std::make_unique<rapidxml::xml_document<char>>();
	rapidxml::file<char> xmlFile(filePath.string().c_str());
	document->parse<0>(xmlFile.data());
	const auto spriteNode = document->first_node("sprites");


	for(auto animNode = spriteNode->first_node("animation");
		     animNode != nullptr;
		     animNode = animNode->next_sibling("animation"))
	{
		const auto title = animNode->first_attribute("title");

		if (!title)
			continue;

		const std::string name(title->value(), title->value_size());

		if (auto it = m_animations.find(name); it != m_animations.end())
			continue;

		FrameManager::Frames tmpFrames;

		auto cutNode = animNode->first_node("cut");

		while (cutNode)
		{
			auto pX = cutNode->first_attribute("x");
			auto pY = cutNode->first_attribute("y");
			auto pW = cutNode->first_attribute("w");
			auto pH = cutNode->first_attribute("h");

			int x = pX ? atoi(pX->value()) : 0;
			int y = pY ? atoi(pY->value()) : 0;
			int w = pW ? atoi(pW->value()) : 0;
			int h = pH ? atoi(pH->value()) : 0;

			tmpFrames.emplace_back(sf::IntRect({ x, y }, { w, h }));

			cutNode = cutNode->next_sibling();
		}
		
		if(!tmpFrames.empty())
		{
			auto& frames = m_animationList.emplace_back();
			tmpFrames.swap(frames);
			m_animations.emplace(name, frames);
		}
	}
}


sf::IntRect FrameManager::getFrame(const std::string& name) const noexcept
{
	auto it = m_animations.find(name);

	return (it != m_animations.end()) ? it->second.front() : sf::IntRect();
}


FrameManager::FrameView FrameManager::getAnimation(const std::string& name) const noexcept
{
	auto it = m_animations.find(name);

	return (it != m_animations.end()) ? it->second : std::span<const sf::IntRect>();
}