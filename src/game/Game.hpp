#pragma once

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

    const MainWindow& getWindow() const noexcept;
    const View& getView() const noexcept;

    class Level* getLevel() noexcept;

private:
    MainWindow m_window;
    View m_view;

    class Level* m_level;
};