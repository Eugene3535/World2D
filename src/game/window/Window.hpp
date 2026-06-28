#pragma once

#include "graphics/camera/View.hpp"


class Window:
    public View
{
public:
    Window() noexcept;
    ~Window();

    bool openWindow(const char* title, int width, int height) noexcept;

    // добавить клавиши мб 

protected:
    struct GLFWwindow* m_glfwWindow;
};