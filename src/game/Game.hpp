#pragma once

#include <memory>

class Game final
{
public:
    Game() noexcept;
    ~Game();

    bool setupContext() noexcept;
    int play() noexcept;


private:
    struct GLFWwindow* openWindow(const char* title, int width, int height) noexcept;
    bool initOpenGL() noexcept;
    void initDebug() noexcept;

    struct GLFWwindow* m_window;
    std::unique_ptr<class View> m_view;
};