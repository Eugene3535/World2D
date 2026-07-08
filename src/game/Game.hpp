#pragma once

#include "game/window/WindowData.hpp"
#include "game/window/MainWindow.hpp"
#include "graphics/camera/View.hpp"


class Game final
{
public:
    enum State
    {
        TitleScreen,
        Stage
    };

    Game() noexcept;
    ~Game();

    bool initialize() noexcept;
    int play() noexcept;

private:
    WindowData m_windowData;
    MainWindow m_window;
    View m_view;
};