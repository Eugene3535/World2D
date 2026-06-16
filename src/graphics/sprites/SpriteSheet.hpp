#ifndef SPRITE_MANAGER_HPP
#define SPRITE_MANAGER_HPP

#include <string>
#include <vector>
#include <span>
#include <unordered_map>
#include <filesystem>
#include <optional>

#include <cglm/struct/ivec4.h>


struct Sprite2D
{
	uint32_t texture;
	uint32_t frame;
	uint32_t width;
	uint32_t height;
};


class SpriteSheet final
{
	using sprite_range = std::pair<uint32_t, uint32_t>; // start sprite number, count ( single = 1, animation = n )

public:
	SpriteSheet() noexcept;
	SpriteSheet(const SpriteSheet&)              noexcept = delete;
	SpriteSheet(SpriteSheet&&)                   noexcept = delete;
	SpriteSheet& operator = (const SpriteSheet&) noexcept = delete;
	SpriteSheet& operator = (SpriteSheet&&)      noexcept = delete;
	~SpriteSheet();

	void createSprite(const std::string& name, uint32_t texture) noexcept;
	void createSprite(const std::string& name, uint32_t texture, const ivec4s& frame) noexcept;

	void createLinearAnimaton(const std::string& name, uint32_t texture, int duration) noexcept;
	void createGridAnimaton(const std::string& name, uint32_t texture, int columns, int rows) noexcept;
	void createCustomAnimaton(const std::string& name, uint32_t texture, std::span<const ivec4s> frames) noexcept;
	
	void loadSpriteSheet(const std::filesystem::path& filePath, uint32_t texture) noexcept;

	std::optional<Sprite2D>   getSprite(const std::string& name) const noexcept;
	std::span<const Sprite2D> getAnimation(const std::string& name) const noexcept;

	void bind(bool toBind) const noexcept;

	void pushVerticesOnGPU() noexcept;

private:
	void addSprite(const uint32_t texture, const ivec4s frame, const vec2s ratio) noexcept;

	uint32_t m_vao;
	uint32_t m_vbo;

	std::unordered_map<std::string, sprite_range> m_animations;
	std::vector<Sprite2D> m_sprites;
	std::vector<float> m_vertices;
};

#endif // !SPRITE_MANAGER_HPP