#pragma once

#include <memory>

#include "game/window/Window.hpp"
#include "game/context/Context.hpp"


class Game final:
    public Window,
    public Context
{
public:
    Game() noexcept;
    ~Game();

    bool setup() noexcept;
    int play() noexcept;
};