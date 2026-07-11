#pragma once

#include <cglm/struct/ivec2.h>


class MainWindow final
{
public:
    MainWindow() noexcept;
    ~MainWindow();

    bool open(const char* title, int width, int height, class View& view) noexcept;
    void close() const noexcept;

    void pollEvents() const noexcept;
    void display() const noexcept;

    float getElapsedTime() const noexcept;
    ivec2s getSize() const noexcept;

    bool isKeyPressed(int key) const noexcept;
    bool isOpen() const noexcept;

private:
    void setupCallbacks(class View& view) noexcept;

    struct GLFWwindow* m_glfwWindow;
};