#pragma once

class Window
{
public:
    Window() noexcept;
    virtual ~Window();

    bool openWindow(const char* title, int width, int height) noexcept;

protected:
    struct GLFWwindow* m_window;
};