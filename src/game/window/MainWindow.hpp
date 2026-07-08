#pragma once

#include <cglm/struct/ivec2.h>


class MainWindow final
{
public:
    MainWindow() noexcept;
    ~MainWindow();

    bool open(struct WindowData& data) noexcept;
    void close() const noexcept;

    void pollEvents() const noexcept;
    void display() const noexcept;

    float getElapsedTime() const noexcept;
    ivec2s getSize() const noexcept;

    bool isKeyPressed(char symbol) const noexcept;
    bool isOpen() const noexcept;

private:
    void setupCallbacks(struct WindowData& data) noexcept;

    struct GLFWwindow* m_glfwWindow;
};