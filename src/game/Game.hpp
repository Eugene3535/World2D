#pragma once

#include <memory>

#include "game/window/Window.hpp"
#include "game/context/Context.hpp"
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

    bool setup() noexcept;
    int play() noexcept;

private:
    std::unique_ptr<class WindowData> m_windowData;
    Window m_window;
    Context m_context;
    View m_view;
};