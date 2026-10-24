#pragma once


class HeadUpDisplay final
{
public:
    HeadUpDisplay(const struct WindowData& data) noexcept;
    ~HeadUpDisplay();

    void update(class TileMap& tilemap, float dt) noexcept;

private:
    const struct WindowData* m_windowData = nullptr;
};