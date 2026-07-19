#pragma once

class MainLoop
{
public:
    MainLoop(class Game& game) noexcept;
    ~MainLoop();

    bool operator()() noexcept;

private:
    class Game& m_game;
};