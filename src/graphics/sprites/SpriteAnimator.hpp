#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include <cglm/struct/ivec4.h>


struct Sprite2D
{
    uint32_t texture;
    uint32_t frame;
    uint32_t width;
    uint32_t height;
};


class SpriteAnimator final
{
public:
    SpriteAnimator() noexcept;
    SpriteAnimator(const SpriteAnimator&) noexcept            = delete;
    SpriteAnimator(SpriteAnimator&&) noexcept                 = delete;
    SpriteAnimator& operator=(const SpriteAnimator&) noexcept = delete;
    SpriteAnimator& operator=(SpriteAnimator&&) noexcept      = delete;
    ~SpriteAnimator();

    void createSprite(const std::string& name, uint32_t texture) noexcept;
    void createSprite(const std::string& name, uint32_t texture, const ivec4s& frame) noexcept;

    void createLinearAnimaton(const std::string& name, uint32_t texture, int duration) noexcept;
    void createGridAnimaton(const std::string& name, uint32_t texture, int columns, int rows) noexcept;
    void createCustomAnimaton(const std::string& name, uint32_t texture, std::span<const ivec4s> frames) noexcept;

    void loadSpriteSheet(const std::filesystem::path& filePath, uint32_t texture) noexcept;

    std::optional<Sprite2D>   getSprite(const std::string& name) const noexcept;
    std::span<const Sprite2D> getAnimation(const std::string& name) const noexcept;

    void pushVerticesOnGPU(const uint32_t vertexBuffer) noexcept;

private:
    void addSprite(const uint32_t texture, const ivec4s frame, const vec2s ratio) noexcept;

    using sprite_range = std::pair<uint32_t, uint32_t>; // start sprite number, count ( single = 1, animation = n )
    std::unordered_map<std::string, sprite_range> m_animations;

    std::vector<Sprite2D> m_sprites;
    std::vector<float>    m_vertices;
    bool                  m_isLocked{false};
};