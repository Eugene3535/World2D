#pragma once

namespace sf
{
    class RenderWindow;
}

class MainLoop
{
public:
    MainLoop(class sf::RenderWindow& rw) noexcept;
    ~MainLoop();

    bool operator()(struct Game& game) noexcept;

private:
    class sf::RenderWindow& m_window;
};